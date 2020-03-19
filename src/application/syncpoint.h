#pragma once

#include <mutex>

#include "game/gamecontext.h"
#include "game/tickercontext.h"

namespace application {

class SyncPoint : public game::TickerContext::TickableBase<SyncPoint> {
public:
    SyncPoint(game::GameContext &context)
        : TickableBase(context)
    {
        syncMutex.lock();
    }

    ~SyncPoint() {
        syncMutex.unlock();
    }

    std::mutex &getMutex() {
        return syncMutex;
    }

    void tick(game::TickerContext &tickerContext) {
        (void) tickerContext;

        syncMutex.unlock();
        syncMutex.lock();
    }

private:
    std::mutex syncMutex;
};

}
