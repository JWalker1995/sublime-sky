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

        graphics::GlBuffer<float> pbo;
        GLsync sync;
        const float *mappedData = 0;
        std::atomic_uint jobsRemainingCount = 0;
    };

    DownloadBuffer *buffers;
    unsigned int numBuffers;

    std::mutex roiMutex;
    std::vector<RegionOfInterest> rois;

    unsigned int numWorkParts;
    jw_util::WorkQueue<DownloadBuffer *, unsigned int, unsigned int> processQueue;
    static void processBufferChunk(DownloadBuffer *buffer, unsigned int rowOffset, unsigned int rowLimit);

    void takeDepthSnapshot(DownloadBuffer &buffer);
    void checkForSynced(DownloadBuffer &buffer);
    void checkForProcessed(DownloadBuffer &buffer);
};

}
