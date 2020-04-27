#include "externalgenerator.h"

#include <random>

#include "spdlog/logger.h"

#include "game/gamecontext.h"
#include "network/messagebuilder.h"
#include "schemas/message_generated.h"
#include "network/connectionpoolspecialized.h"
#include "util/pool.h"
#include "world/world.h"

namespace worldgen {

ExternalGenerator::ExternalGenerator(game::GameContext &context, const SsProtocol::Config::ExternalWorldGenerator *config)
    : TickableBase(context)
{
    (void) config;

    context.provideInstance<ExternalGenerator>(this);
}

ExternalGenerator::~ExternalGenerator() {
    context.removeInstance<ExternalGenerator>();
}

void ExternalGenerator::tick(game::TickerContext &tickerContext) {
    (void) tickerContext;

    network::MessageBuilder::Lock lock(context);

    static thread_local std::vector<flatbuffers::Offset<SsProtocol::TerrainChunk>> terrainChunks;
    assert(terrainChunks.empty());

    for (spatial::CellKey cube : generateQueue) {
        SsProtocol::Vec3_u32 cellCoord(cube.cellCoord.x, cube.cellCoord.y, cube.cellCoord.z);
        terrainChunks.push_back(SsProtocol::CreateTerrainChunk(lock.getBuilder(), cube.sizeLog2, &cellCoord));
    }
    generateQueue.clear();

    auto terrainChunksVec = lock.getBuilder().CreateVector(terrainChunks);
    terrainChunks.clear();

    auto terrainMessage = SsProtocol::CreateTerrainMessage(lock.getBuilder(), terrainChunksVec);
    auto message = SsProtocol::CreateMessage(lock.getBuilder(), SsProtocol::MessageUnion_TerrainMessage, terrainMessage.Union());
    lock.getBuilder().Finish(message);

    context.get<network::ConnectionPoolSpecialized<SsProtocol::Capabilities_GenerateTerrainChunk>>().send(lock.getBuilder().GetBufferPointer(), lock.getBuilder().GetSize());
}

void ExternalGenerator::generate(spatial::CellKey cube) {
    // TODO: Batch requests up and send in a ticker

    spatial::UintCoord c0 = cube.getCoord<0, 0, 0>();
    spatial::UintCoord c1 = cube.getCoord<1, 1, 1>();
    context.get<spdlog::logger>().trace("Requesting size:{} cube generation: ({}, {}, {}) : ({}, {}, {})", cube.sizeLog2, c0.x, c0.y, c0.z, c1.x, c1.y, c1.z);

    generateQueue.push_back(cube);
}

void ExternalGenerator::handleResponse(const SsProtocol::TerrainChunk *chunk, const std::vector<unsigned int> &materialMap) {
    spatial::CellKey cube;
    cube.sizeLog2 = chunk->cell_size_log2();
    cube.cellCoord.x = chunk->cell_coord()->x();
    cube.cellCoord.y = chunk->cell_coord()->y();
    cube.cellCoord.z = chunk->cell_coord()->z();

    spatial::UintCoord c0 = cube.getCoord<0, 0, 0>();
    spatial::UintCoord c1 = cube.getCoord<1, 1, 1>();
    context.get<spdlog::logger>().trace("Received size:{} cube generation: ({}, {}, {}) : ({}, {}, {})", cube.sizeLog2, c0.x, c0.y, c0.z, c1.x, c1.y, c1.z);

    unsigned int chunkSizeLog2 = (sizeof(unsigned long long) * CHAR_BIT - 1 - __builtin_clzll(chunk->cell_materials()->size())) / 3;
    static constexpr std::size_t chunkSize0 = 1;
    std::size_t chunkSize1 = static_cast<std::size_t>(1) << (chunkSizeLog2 * 1);
    std::size_t chunkSize2 = static_cast<std::size_t>(1) << (chunkSizeLog2 * 2);
    std::size_t chunkSize3 = static_cast<std::size_t>(1) << (chunkSizeLog2 * 3);

    if (chunk->cell_materials()->size() != chunkSize3) {
        context.get<spdlog::logger>().error("Received TerrainChunk::cell_materials with size {}, which is not a valid octree size (must be a integral power of 8). Skipping chunk.", chunk->cell_materials()->size());
        return;
    }

    struct CreatingVisitor {
        CreatingVisitor(game::GameContext &context)
            : context(context)
        {}

        void beforeEnterChild(world::Node *node, unsigned int childIndex) {
            (void) childIndex;

            if (node->isLeaf) {
                node->setBranch(context);
            }
        }

        game::GameContext &context;
    };
    CreatingVisitor visitor(context);
    world::Node *node = context.get<world::World>().getRoot()->getChild<CreatingVisitor &>(cube, visitor);

    if (chunk->is_rigid_body()) {
        // TODO
//        node->isRigidBody = true;
    }

    const std::uint32_t *matPtr = chunk->cell_materials()->data();
    unsigned int countBadMaterials = 0;

    unsigned int stack[32];
    unsigned int stackPos = chunkSizeLog2;
    stack[stackPos] = 0;
    while (true) {
        while (stackPos > 0) {
            if (node->isLeaf) {
                node->setBranch(context);
            }
            node = node->children;

            stack[--stackPos] = 0;
        }

        unsigned int matIdx;
        if (*matPtr < materialMap.size()) {
            matIdx = *matPtr;
        } else {
            matIdx = 0;
            countBadMaterials++;
        }

        assert(node->isLeaf);
        node->materialIndex = static_cast<world::MaterialIndex>(materialMap[matIdx]);

        incStackEntry:
        switch (++stack[stackPos]) {
            case 1: matPtr += chunkSize0 << stackPos; break;
            case 2: matPtr += (chunkSize1 - chunkSize0) << stackPos; break;
            case 3: matPtr += chunkSize0 << stackPos; break;
            case 4: matPtr += (chunkSize2 - chunkSize1 - chunkSize0) << stackPos; break;
            case 5: matPtr += chunkSize0 << stackPos; break;
            case 6: matPtr += (chunkSize1 - chunkSize0) << stackPos; break;
            case 7: matPtr += chunkSize0 << stackPos; break;
            case 8:
                matPtr -= (chunkSize2 + chunkSize1 + chunkSize0) << stackPos;

                if (stackPos == chunkSizeLog2) {
                    goto finished;
                }

                stackPos++;
                node = node->parent;

                goto incStackEntry;
        }

        node++;
    }
    finished:

    assert(matPtr == chunk->cell_materials()->data());

    if (countBadMaterials > 0) {
        context.get<spdlog::logger>().warn("Received {} materials with invalid indices. They were replaced with material 0.", countBadMaterials);
    }
}

}
