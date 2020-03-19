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

        static constexpr unsigned int x = Chunk::size / 2;
        glm::vec3 midpoint = chunk->points[x][x][x];
        world::CellValue &foundCell = context.get<world::HashTreeWorld>().getCellValueContaining(midpoint);
        if (foundCell.points != chunk) {
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

        assert(foundCell.points == chunk);
        foundCell.points = 0;
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
