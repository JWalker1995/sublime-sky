#include "depthbufferprocessor.h"

#include <random>

#include "spdlog/logger.h"

#include "graphics/gpuprogram.h"
#include "render/program/pointclouddepthprogram.h"
#include "render/program/pointcloudcolorprogram.h"
#include "render/program/pixelmatchfetcherprogram.h"
#include "application/window.h"
#include "render/imguirenderer.h"
#include "schemas/config_client_generated.h"
#include "render/meshupdater.h"
#include "world/hashtreeworld.h"

namespace render {

DepthBufferProcessor::DepthBufferProcessor(game::GameContext &context, const SsProtocol::Config::DepthBufferProcessor *config)
    : TickableBase(context)
    , processQueue(config->num_threads())
{
    (void) config;

    numBuffers = config->num_pixel_buffer_objects();
    buffers = new DownloadBuffer[numBuffers];
    for (unsigned int i = 0; i < numBuffers; i++) {
        buffers[i].processor = this;
    }

    numWorkParts = config->num_work_parts();

    glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
}

DepthBufferProcessor::~DepthBufferProcessor() {
    processQueue.pause();
    delete[] buffers;
}

DepthBufferProcessor::DownloadBuffer::DownloadBuffer()
    : pointIdPbo(GL_PIXEL_PACK_BUFFER, GL_DYNAMIC_READ)
    , depthPbo(GL_PIXEL_PACK_BUFFER, GL_DYNAMIC_READ)
{}

void DepthBufferProcessor::tick(game::TickerContext &tickerContext) {
    world::HashTreeWorld &hashTreeWorld = context.get<world::HashTreeWorld>();
    render::SceneManager::MeshHandle meshHandle = context.get<render::MeshUpdater>().getMeshHandle();
    std::size_t pointsExtent = meshHandle.getSceneManager().getPointBuffer().getExtentSize();
    std::size_t pointsActive = meshHandle.getSceneManager().getPointBuffer().getActiveSize();
    assert(pointsExtent <= getNullPointId());

    if (pointsExtent > numUnseenPoints) {
        processQueue.pause();

        numUnseenPoints = pointsExtent * 3 / 2;
        delete[] unseenPoints;
        unseenPoints = new std::atomic_flag[numUnseenPoints];

        processQueue.start();
    }

    tryAdvance();

    static thread_local std::vector<RegionOfInterest> localRois;
    assert(localRois.empty());

    float priorFramesProcessedCount;
    {
        const std::lock_guard<std::mutex> lock(processingResultsMutex);
        rois.swap(localRois);
        priorFramesProcessedCount = framesProcessedCount;
        framesProcessedCount = 0.0f;
    }


    static thread_local std::default_random_engine gen;
    std::normal_distribution<float> dist(0.0f, 1.0f);

    context.get<spdlog::logger>().debug("Found {} regions of interest", localRois.size());
    for (const RegionOfInterest &roi : localRois) {
        for (unsigned int i = 0; i < 1; i++) {
            glm::vec3 position = roi.position + glm::vec3(dist(gen), dist(gen), dist(gen));

            world::MaterialIndex materialIndex = hashTreeWorld.queryMaterialAt(position);
            if (true
                    && materialIndex != world::MaterialIndex::Null
                    && materialIndex != world::MaterialIndex::Generating
                    && !hashTreeWorld.isGas(materialIndex)
            ) {
                render::SceneManager::PointMutator point = meshHandle.createPoint();

                point.shared.materialIndex = 0;

                point.shared.position[0] = position.x;
                point.shared.position[1] = position.y;
                point.shared.position[2] = position.z;

                glm::vec3 normal(1.0f);
                point.shared.normal[0] = normal.x;
                point.shared.normal[1] = normal.y;
                point.shared.normal[2] = normal.z;
            }
        }
    }
    localRois.clear();

    if (pointsActive > 1024 * 1024 * 2) {
        static thread_local std::size_t nextTestPoint = 0;
        std::size_t start = nextTestPoint;
        std::size_t end = nextTestPoint + std::min<std::size_t>(1024, pointsExtent * priorFramesProcessedCount / 32.0f);
        if (end > pointsExtent) {
            end = pointsExtent;
            nextTestPoint = 0;
        } else {
            nextTestPoint = end;
        }

        for (std::size_t i = start; i < end; i++) {
            bool unseen = unseenPoints[i].test_and_set(std::memory_order_relaxed);
            if (unseen) {
                render::SceneManager::PointMutator point = meshHandle.mutatePoint(i);
                if (point.shared.normal[0] != 0.0f || point.shared.normal[1] != 0.0f || point.shared.normal[2] != 0.0f) {
                    meshHandle.destroyPoint(i);
                    std::fill_n(point.shared.normal, 3, 0.0f);
                }
            }
        }
    }
}

void DepthBufferProcessor::tryAdvance() {
    for (unsigned int i = 0; i < numBuffers; i++) {
        DownloadBuffer &buf = buffers[i];

        switch (buf.status) {
            case DownloadBuffer::Status::Syncing: checkForSynced(buf); break;
            case DownloadBuffer::Status::Processing: checkForProcessed(buf); break;
            default: break;
        }
    }
}

void DepthBufferProcessor::takeDepthSnapshot() {
    for (unsigned int i = 0; i < numBuffers; i++) {
        DownloadBuffer &buf = buffers[i];

        if (buf.status == DownloadBuffer::Status::Initialized) {
            takeDepthSnapshot(buf);
            return;
        }
    }

    std::string statuses;
    for (unsigned int i = 0; i < numBuffers; i++) {
        switch (buffers[i].status) {
            case DownloadBuffer::Status::Initialized: statuses += 'i'; break;
            case DownloadBuffer::Status::Syncing: statuses += 's'; break;
            case DownloadBuffer::Status::Processing: statuses += 'p'; break;
        }
    }
    context.get<spdlog::logger>().warn("Cannot take depth snapshot because there's no available buffers! Statuses: {}, queue size: {}", statuses, processQueue.getQueueSize());
}

void DepthBufferProcessor::takeDepthSnapshot(DownloadBuffer &buffer) {
    assert(buffer.status == DownloadBuffer::Status::Initialized);

    buffer.transformationMatrixInv = glm::inverse(context.get<render::MeshUpdater>().getMeshHandle().readMesh().shared.transform);

    application::Window::Dimensions dims = context.get<application::Window>().dimensions;
    buffer.width = dims.width;
    buffer.height = dims.height;
    unsigned int size = dims.width * dims.height;
    buffer.pointIdPbo.update_size_nocopy(size);
    buffer.depthPbo.update_size_nocopy(size);

    buffer.pointIdPbo.bind();
    glReadPixels(0, 0, dims.width, dims.height, GL_RED, GL_FLOAT, 0);
    graphics::GL::catchErrors();

    buffer.depthPbo.bind();
    glReadPixels(0, 0, dims.width, dims.height, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    graphics::GL::catchErrors();

    buffer.sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    graphics::GL::catchErrors();

    glFlush();
    graphics::GL::catchErrors();

    buffer.status = DownloadBuffer::Status::Syncing;
}

void DepthBufferProcessor::checkForSynced(DownloadBuffer &buffer) {
    assert(buffer.status == DownloadBuffer::Status::Syncing);

    GLint value;
    glGetSynciv(buffer.sync, GL_SYNC_STATUS, 1, nullptr, &value);
    graphics::GL::catchErrors();
    assert(value == GL_UNSIGNALED || value == GL_SIGNALED);

    if (value == GL_SIGNALED) {
        buffer.pointIdPbo.bind();
        buffer.mappedPointIdData = static_cast<std::uint32_t *>(glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, buffer.width * buffer.height, GL_MAP_READ_BIT));
        graphics::GL::catchErrors();

        buffer.depthPbo.bind();
        buffer.mappedDepthData = static_cast<float *>(glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, buffer.width * buffer.height, GL_MAP_READ_BIT));
        graphics::GL::catchErrors();

        buffer.status = DownloadBuffer::Status::Processing;

        unsigned int prevProcessingCount = buffer.jobsRemainingCount.exchange(numWorkParts);
        assert(prevProcessingCount == 0);

        unsigned int start = 0;
        for (unsigned int i = 0; i < numWorkParts; i++) {
            unsigned int end = buffer.height * (i + 1) / numWorkParts;
            processQueue.push(&processBufferChunk, &buffer, start, end);
            start = end;
        }
    }
}

void DepthBufferProcessor::checkForProcessed(DownloadBuffer &buffer) {
    assert(buffer.status == DownloadBuffer::Status::Processing);

    if (buffer.jobsRemainingCount == 0) {
        buffer.mappedPointIdData = 0;
        buffer.mappedDepthData = 0;

        buffer.pointIdPbo.bind();
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        graphics::GL::catchErrors();

        buffer.depthPbo.bind();
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        graphics::GL::catchErrors();

        buffer.status = DownloadBuffer::Status::Initialized;
    }
}

void DepthBufferProcessor::processBufferChunk(DownloadBuffer *buffer, unsigned int rowOffset, unsigned int rowLimit) {
    // RUNS IN THREAD

    assert(buffer->mappedPointIdData);
    assert(buffer->mappedDepthData);

    assert(rowOffset <= rowLimit);
    assert(rowLimit <= buffer->height);

    static thread_local std::vector<RegionOfInterest> localRois;
    localRois.reserve(1024);
    assert(localRois.empty());

    unsigned int width = buffer->width;
    const std::uint32_t *cur = buffer->mappedPointIdData + width * rowOffset;
    for (unsigned int y = rowOffset; y < rowLimit; y++) {
        for (unsigned int x = 0; x < width; x++) {
            std::uint32_t val = *cur;
            if (val == getNullPointId()) {
                float sum = 0.0f;
                float count = 0.0f;
                for (signed int dy = -1; dy <= 1; dy++) {
                    unsigned int ny = y + dy;
                    for (signed int dx = -1; dx <= 1; dx++) {
                        unsigned int nx = x + dx;
                        if (ny < buffer->height && nx < width) {
                            float nv = buffer->mappedDepthData[ny * width + nx];
                            if (nv != 1.0f) {
                                sum += nv;
                                count += 1.0f;
                            }
                        }
                    }
                }

                if (count >= 2.0f) {
                    float clipX = static_cast<float>(x) / width * 2.0f - 1.0f;
                    float clipY = static_cast<float>(y) / buffer->height * 2.0f - 1.0f;
                    float clipZ = sum / count;
                    float clipW = 1.0f;
                    glm::vec4 unTransformed = buffer->transformationMatrixInv * glm::vec4(clipX, clipY, clipZ, clipW);

                    RegionOfInterest roi;
                    roi.position = glm::vec3(unTransformed.x, unTransformed.y, unTransformed.z) / unTransformed.w;
                    localRois.push_back(roi);

//                    if (localRois.size() >= 1024) {
//                        goto finishedProcessing;
//                    }
                }
            } else {
                assert(val < buffer->processor->numUnseenPoints);
                buffer->processor->unseenPoints[val].clear(std::memory_order_relaxed);
            }
        }
    }

    finishedProcessing:

    buffer->jobsRemainingCount--;

    {
        const std::lock_guard<std::mutex> lock(buffer->processor->processingResultsMutex);
        buffer->processor->framesProcessedCount += static_cast<float>(rowLimit - rowOffset) / buffer->height;
        buffer->processor->rois.insert(buffer->processor->rois.end(), localRois.cbegin(), localRois.cend());
    }

    localRois.clear();
}

}
