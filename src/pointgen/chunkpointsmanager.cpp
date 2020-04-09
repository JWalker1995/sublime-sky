#include "chunkpointsmanager.h"

#include "game/gamecontext.h"
#include "util/pool.h"
#include "world/hashtreeworld.h"
#include "pointgen/pointgenerator.h"

namespace pointgen {

ChunkPointsManager::ChunkPointsManager(game::GameContext &context)
    : context(context)
{}

Chunk *ChunkPointsManager::generate(const spatial::CellKey &cellKey) {
    static constexpr unsigned int targetCacheSize = 1024;

    Chunk *chunk;
    if (numChunks < targetCacheSize) {
        chunk = addAvailableChunk();
    } else {
        chunk = mostRecentlyUsed->moreRecentlyUsed;

        glm::vec3 min;
        for (unsigned int i = 0; i < Chunk::size; i++) {
            min = chunk->points[0][0][i];
            if (!std::isnan(min.x)) { break; }
        }

        glm::vec3 max;
        for (unsigned int i = 0; i < Chunk::size; i++) {
            max = chunk->points[Chunk::size - 1][Chunk::size - 1][Chunk::size - 1 - i];
            if (!std::isnan(max.x)) { break; }
        }
        max -= glm::vec3(1e-3f);

        spatial::CellKey chunkKey = spatial::CellKey::fromCoords(spatial::UintCoord::fromPoint(min), spatial::UintCoord::fromPoint(max));
        world::HashTreeWorld::Cell &foundCell = context.get<world::HashTreeWorld>().lookupChunk(chunkKey);
        if (foundCell.second.points != chunk) {
            assert(false);

            // This should never happen; but we can deal with it by orphaning the point chunk and trying again.
            Chunk *chunkLru = chunk->lessRecentlyUsed;
            Chunk *chunkMru = chunk->moreRecentlyUsed;
            chunkMru->lessRecentlyUsed = chunkLru;
            chunkLru->moreRecentlyUsed = chunkMru;

            chunk->moreRecentlyUsed = 0;
            chunk->lessRecentlyUsed = 0;

            numChunks--;

            return generate(cellKey);
        }

        assert(foundCell.second.points == chunk);
        foundCell.second.points = 0;
        // Now the chunk is available because nothing refers to it
    }

    context.get<PointGenerator>().generate(chunk, cellKey);

    return chunk;
}

void ChunkPointsManager::use(Chunk *chunk) {
    if (chunk == mostRecentlyUsed) {
        return;
    }

    Chunk *chunkLru = chunk->lessRecentlyUsed;
    Chunk *chunkMru = chunk->moreRecentlyUsed;
    chunkMru->lessRecentlyUsed = chunkLru;
    chunkLru->moreRecentlyUsed = chunkMru;

    chunk->moreRecentlyUsed = mostRecentlyUsed->moreRecentlyUsed;
    chunk->lessRecentlyUsed = mostRecentlyUsed;

    chunk->moreRecentlyUsed->lessRecentlyUsed = chunk;
    chunk->lessRecentlyUsed->moreRecentlyUsed = chunk;

    mostRecentlyUsed = chunk;
}

Chunk *ChunkPointsManager::addAvailableChunk() {
    Chunk *chunk = context.get<util::Pool<Chunk>>().alloc();

    if (mostRecentlyUsed) {
        chunk->moreRecentlyUsed = mostRecentlyUsed->moreRecentlyUsed;
        chunk->lessRecentlyUsed = mostRecentlyUsed;

        chunk->moreRecentlyUsed->lessRecentlyUsed = chunk;
        chunk->lessRecentlyUsed->moreRecentlyUsed = chunk;
    } else {
        chunk->moreRecentlyUsed = chunk;
        chunk->lessRecentlyUsed = chunk;

        mostRecentlyUsed = chunk;
    }

    numChunks++;

    return chunk;
}

}
