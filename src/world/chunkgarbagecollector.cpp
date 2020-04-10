#include "chunkgarbagecollector.h"

#include <random>

#include "schemas/config_client_generated.h"
#include "world/hashtreeworld.h"

namespace world {

ChunkGarbageCollector::ChunkGarbageCollector(game::GameContext &context, const SsProtocol::Config::ChunkGarbageCollector *config)
    : TickableBase(context)
{
    collectionsPerTick = config->collections_per_tick();
}

void ChunkGarbageCollector::tick(game::TickerContext &tickerContext) {
    (void) tickerContext;

    static thread_local std::default_random_engine generator(123);
    std::poisson_distribution<unsigned int> dist(collectionsPerTick);

    unsigned int count = dist(generator);
    for (unsigned int i = 0; i < count; i++) {
        collect();
    }
}

void ChunkGarbageCollector::collect() {
    HashTreeWorld &hashTreeWorld = context.get<HashTreeWorld>();

    std::size_t numBuckets = hashTreeWorld.getMap().bucket_count();
    static thread_local std::default_random_engine generator(456);
    std::uniform_int_distribution<std::size_t> dist(0, numBuckets - 1);
    std::size_t bucketIndex = dist(generator);

    HashTreeWorld::MapType::local_iterator it = hashTreeWorld.getMap().begin(bucketIndex);
    HashTreeWorld::MapType::local_iterator end = hashTreeWorld.getMap().end(bucketIndex);
    while (it != end) {
        HashTreeWorld::Cell *cell = &*it;

        // Gotta do this here so erases don't invalidate it
        it++;

        if (hashTreeWorld.shouldSubdiv(cell->first)) {
            hashTreeWorld.removeChunk(cell);
        } else if (!hashTreeWorld.shouldSubdiv(cell->first.parent())) {
            hashTreeWorld.removeChunk(cell);
        }
    }
}

}
