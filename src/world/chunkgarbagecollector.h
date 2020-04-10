#pragma once

#include "game/tickercontext.h"

namespace SsProtocol {
namespace Config { struct ChunkGarbageCollector; }
}

namespace world {

class ChunkGarbageCollector : public game::TickerContext::TickableBase<ChunkGarbageCollector> {
public:
    ChunkGarbageCollector(game::GameContext &context, const SsProtocol::Config::ChunkGarbageCollector *config);

    void tick(game::TickerContext &tickerContext);

private:
    float collectionsPerTick;

    void collect();
};

}
