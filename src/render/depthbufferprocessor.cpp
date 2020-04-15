#include "depthbufferprocessor.h"

#include "spdlog/logger.h"

#include "graphics/gpuprogram.h"
#include "render/program/pointclouddepthprogram.h"
#include "render/program/pointcloudcolorprogram.h"
#include "render/program/pixelmatchfetcherprogram.h"
#include "application/window.h"
#include "render/imguirenderer.h"
#include "render/camera.h"
#include "schemas/config_client_generated.h"
#include "render/meshupdater.h"

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
}

DepthBufferProcessor::~DepthBufferProcessor() {
    delete[] buffers;
}

DepthBufferProcessor::DownloadBuffer::DownloadBuffer()
    : pbo(GL_PIXEL_PACK_BUFFER, GL_DYNAMIC_READ)
{}

void DepthBufferProcessor::tick(game::TickerContext &tickerContext) {
    tryAdvance();

    const std::lock_guard<std::mutex> lock(roiMutex);
    context.get<spdlog::logger>().debug("Found {} regions of interest", rois.size());
    SceneManager::MeshHandle meshHandle = context.get<render::MeshUpdater>().getMeshHandle();
    for (const RegionOfInterest &roi : rois) {
        render::SceneManager::PointMutator point = meshHandle.createPoint();

        point.shared.materialIndex = static_cast<unsigned int>(4);

        point.shared.position[0] = roi.position.x;
        point.shared.position[1] = roi.position.y;
        point.shared.position[2] = roi.position.z;

        glm::vec3 normal(0.0f);
        point.shared.normal[0] = normal.x;
        point.shared.normal[1] = normal.y;
        point.shared.normal[2] = normal.z;
    }
    rois.clear();
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

    context.get<spdlog::logger>().warn("Cannot take depth snapshot because there's no available buffers!");
}

void DepthBufferProcessor::takeDepthSnapshot(DownloadBuffer &buffer) {
    assert(buffer.status == DownloadBuffer::Status::Initialized);

    buffer.transformationMatrixInv = glm::inverse(context.get<Camera>().getTransform());

    application::Window::Dimensions dims = context.get<application::Window>().dimensions;
    buffer.width = dims.width;
    buffer.height = dims.height;
    unsigned int size = dims.width * dims.height;
    buffer.pbo.update_size_nocopy(size);

    buffer.pbo.bind();
    glReadPixels(0, 0, dims.width, dims.height, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    graphics::GL::catchErrors();

    buffer.sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    glFlush();

    buffer.status = DownloadBuffer::Status::Syncing;
}

void DepthBufferProcessor::checkForSynced(DownloadBuffer &buffer) {
    assert(buffer.status == DownloadBuffer::Status::Syncing);

    GLint value;
    glGetSynciv(buffer.sync, GL_SYNC_STATUS, 1, nullptr, &value);
    assert(value == GL_UNSIGNALED || value == GL_SIGNALED);

    if (value == GL_SIGNALED) {
        buffer.pbo.bind();
        buffer.mappedData = static_cast<float *>(glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, buffer.width * buffer.height, GL_MAP_READ_BIT));
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
        buffer.mappedData = 0;

        buffer.pbo.bind();
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

        buffer.status = DownloadBuffer::Status::Initialized;
    }
}

void DepthBufferProcessor::processBufferChunk(DownloadBuffer *buffer, unsigned int rowOffset, unsigned int rowLimit) {
    // RUNS IN THREAD

    assert(buffer->mappedData);

    assert(rowOffset <= rowLimit);
    assert(rowLimit <= buffer->height);

    static thread_local std::vector<RegionOfInterest> localRois;
    assert(localRois.empty());

    unsigned int width = buffer->width;
    const float *cur = buffer->mappedData + width * rowOffset;
    for (unsigned int y = rowOffset; y < rowLimit; y++) {
        for (unsigned int x = 0; x < width; x++) {
            float val = *cur;
            if (val == 1.0f) {
                float sum = 0.0f;
                float count = 0.0f;
                for (signed int dy = -1; dy <= 1; dy++) {
                    unsigned int ny = y + dy;
                    for (signed int dx = -1; dx <= 1; dx++) {
                        unsigned int nx = x + dx;
                        if (ny < buffer->height && nx < width) {
                            float nv = buffer->mappedData[ny * width + nx];
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
                    roi.position = glm::vec3(unTransformed.x, unTransformed.y, unTransformed.z);
                    localRois.push_back(roi);

                    if (localRois.size() >= 1024) {
                        goto finishedProcessing;
                    }
                }
            }
        }
    }

    finishedProcessing:

    buffer->jobsRemainingCount--;

    {
        const std::lock_guard<std::mutex> lock(buffer->processor->roiMutex);
        buffer->processor->rois.insert(buffer->processor->rois.end(), localRois.cbegin(), localRois.cend());
    }

    localRois.clear();
}

}
