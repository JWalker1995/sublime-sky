#pragma once

#include "pointgen/chunk.h"
#include "spatial/cellkey.h"

namespace game { class GameContext; }

namespace pointgen {

class ChunkPointsManager {
public:
    ChunkPointsManager(game::GameContext &context);

    Chunk *generate(const spatial::CellKey &cellKey);
    void use(Chunk *chunk);
    void release(Chunk *chunk);

private:
    game::GameContext &context;

    Chunk *mostRecentlyUsed = 0;
    unsigned int numChunks = 0;

    Chunk *addAvailableChunk();
};

}
