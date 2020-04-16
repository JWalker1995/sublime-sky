#pragma once

#include <deque>

#include "graphics/glm.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "game/tickercontext.h"
#include "graphics/glbuffer.h"
#include "jw_util/workqueue.h"

namespace SsProtocol {
namespace Config { struct DepthBufferProcessor; }
}

namespace render {

class DepthBufferProcessor : public game::TickerContext::TickableBase<DepthBufferProcessor> {
public:
    DepthBufferProcessor(game::GameContext &context, const SsProtocol::Config::DepthBufferProcessor *config);
    ~DepthBufferProcessor();

    void tick(game::TickerContext &tickerContext);

    // The more often this is called, the better
    void tryAdvance();

    static float getFillColorRed() {
        std::uint32_t pointId = getNullPointId();
        float red = *reinterpret_cast<float *>(&pointId);
        // Make sure clamping won't change it
        assert(red >= 0.0f);
        assert(red <= 1.0f);
        return red;
    }

    void takeDepthSnapshot();

private:
    class RegionOfInterest {
    public:
        glm::vec3 position;
    };

    class DownloadBuffer {
    public:
        enum class Status {
            Initialized,
            Syncing, // Rendering to buffer; fence is active
            Processing, // Data is mapped
        };

        DownloadBuffer();

        DepthBufferProcessor *processor;

        Status status = Status::Initialized;

        glm::mat4x4 transformationMatrixInv;

        unsigned int width;
        unsigned int height;

        graphics::GlBuffer<std::uint32_t> pointIdPbo;
        graphics::GlBuffer<float> depthPbo;
        GLsync sync;
        const std::uint32_t *mappedPointIdData = 0;
        const float *mappedDepthData = 0;
        std::atomic_uint jobsRemainingCount = 0;
    };

    DownloadBuffer *buffers;
    unsigned int numBuffers;

    std::mutex processingResultsMutex;
    float framesProcessedCount = 0.0f;
    std::vector<RegionOfInterest> rois;

    std::atomic_flag *unseenPoints = 0;
    std::size_t numUnseenPoints = 0;

    unsigned int numWorkParts;
    jw_util::WorkQueue<DownloadBuffer *, unsigned int, unsigned int> processQueue;
    static void processBufferChunk(DownloadBuffer *buffer, unsigned int rowOffset, unsigned int rowLimit);

    void takeDepthSnapshot(DownloadBuffer &buffer);
    void checkForSynced(DownloadBuffer &buffer);
    void checkForProcessed(DownloadBuffer &buffer);

    static constexpr std::uint32_t getNullPointId() {
        // Largest uint whose float representation will not be clamped by glClearColor

        // Can be calculated with this loop:
        /*
        for (std::uint32_t i = static_cast<std::uint32_t>(-1); i > 0; i--) {
            float r = *reinterpret_cast<float *>(&i);
            if (r > 0.0f && r <= 1.0f) {
                return i;
            }
        }
        // Ends up being 1.0
        */

        return 0x3F800000;
    }
};

}
