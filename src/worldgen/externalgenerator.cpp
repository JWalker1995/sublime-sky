#include "externalgenerator.h"

#include "spdlog/logger.h"

#include "game/gamecontext.h"
#include "network/messagebuilder.h"
#include "schemas/message_generated.h"
#include "network/connectionpoolspecialized.h"
#include "world/hashtreeworld.h"
#include "particle/particlemanager.h"
#include "util/pool.h"

// TODO: Remove
#include "render/meshupdater.h"

namespace worldgen {

ExternalGenerator::ExternalGenerator(game::GameContext &context, const SsProtocol::Config::ExternalWorldGenerator *config)
    : context(context)
{
    (void) config;

    assert(pointgen::Chunk::size == world::Chunk::size);
}

void ExternalGenerator::generate(spatial::CellKey cube, const pointgen::Chunk *points) {
    spatial::UintCoord c0 = cube.getCoord<0, 0, 0>();
    spatial::UintCoord c1 = cube.getCoord<1, 1, 1>();
    context.get<spdlog::logger>().trace("Requesting size:{} cube generation: ({}, {}, {}) : ({}, {}, {})", cube.sizeLog2, c0.x, c0.y, c0.z, c1.x, c1.y, c1.z);

    network::MessageBuilder::Lock lock(context);

    SsProtocol::Vec3_u32 cellCoord(cube.cellCoord.x, cube.cellCoord.y, cube.cellCoord.z);
    SsProtocol::Vec3_f cellsPositions[pointgen::Chunk::size][pointgen::Chunk::size][pointgen::Chunk::size];
    for (unsigned int i = 0; i < pointgen::Chunk::size; i++) {
        for (unsigned int j = 0; j < pointgen::Chunk::size; j++) {
            for (unsigned int k = 0; k < pointgen::Chunk::size; k++) {
                glm::vec3 pt = points->points[i][j][k];
                cellsPositions[i][j][k] = SsProtocol::Vec3_f(pt.x, pt.y, pt.z);
            }
        }
    }
    auto cellsVec = lock.getBuilder().CreateVectorOfStructs(&cellsPositions[0][0][0], pointgen::Chunk::size * pointgen::Chunk::size * pointgen::Chunk::size);
    auto chunkCommand = SsProtocol::CreateTerrainChunk(lock.getBuilder(), cube.sizeLog2, &cellCoord, cellsVec);
    auto message = SsProtocol::CreateMessage(lock.getBuilder(), SsProtocol::MessageUnion_TerrainChunk, chunkCommand.Union());
    lock.getBuilder().Finish(message);

    context.get<network::ConnectionPoolSpecialized<SsProtocol::Capabilities_GenerateTerrainChunk>>().send(lock.getBuilder().GetBufferPointer(), lock.getBuilder().GetSize());
}

void ExternalGenerator::handleResponse(const SsProtocol::TerrainChunk *chunk, unsigned int materialOffset) {
    world::HashTreeWorld &hashTreeWorld = context.get<world::HashTreeWorld>();
    render::MeshUpdater &meshUpdater = context.get<render::MeshUpdater>();

    assert(materialOffset != static_cast<unsigned int>(-1));

    static unsigned int chunksLeft = 10;
    if (chunksLeft == 0) {
        return;
    }

    spatial::CellKey cube;
    cube.sizeLog2 = chunk->cell_size_log2();
    cube.cellCoord.x = chunk->cell_coord()->x();
    cube.cellCoord.y = chunk->cell_coord()->y();
    cube.cellCoord.z = chunk->cell_coord()->z();

    spatial::UintCoord c0 = cube.getCoord<0, 0, 0>();
    spatial::UintCoord c1 = cube.getCoord<1, 1, 1>();
    context.get<spdlog::logger>().trace("Received size:{} cube generation: ({}, {}, {}) : ({}, {}, {})", cube.sizeLog2, c0.x, c0.y, c0.z, c1.x, c1.y, c1.z);

    world::HashTreeWorld::Cell *cell = context.get<world::HashTreeWorld>().findNodeMatching(cube);
    if (!cell) {
        return;
    }

    if (!cell->second.chunk) {
        cell->second.chunk = context.get<util::Pool<world::Chunk>>().alloc();
    }

    const pointgen::Chunk *pointChunk = hashTreeWorld.getChunkPoints(cell);

    bool hasNonGas = false;

    const std::uint32_t *ptPtr = chunk->cell_materials()->data();
    for (unsigned int i = 0; i < pointgen::Chunk::size; i++) {
        for (unsigned int j = 0; j < pointgen::Chunk::size; j++) {
            for (unsigned int k = 0; k < pointgen::Chunk::size; k++) {
                world::MaterialIndex materialIndex = static_cast<world::MaterialIndex>(materialOffset + *ptPtr++);
                cell->second.chunk->cells[i][j][k].materialIndex = materialIndex;
                bool isNotGas = !hashTreeWorld.isGas(materialIndex);
                hasNonGas |= isNotGas;

                if (isNotGas) {
                    if (i >= 2 && i < pointgen::Chunk::size - 2) {
                        if (j >= 2 && j < pointgen::Chunk::size - 2) {
                            if (k >= 2 && k < pointgen::Chunk::size - 2) {
                                render::SceneManager::VoronoiCellMutator voronoiCell = meshUpdater.getMeshHandle().createVoronoiCell();

                                voronoiCell.shared.materialIndex = static_cast<unsigned int>(materialIndex);

                                spatial::CellKey cellKey = cube.grandChild<world::Chunk::sizeLog2>(i, j, k);
                                assert(cellKey.sizeLog2 == 0);
                                voronoiCell.shared.cellPosition[0] = cellKey.cellCoord.x;
                                voronoiCell.shared.cellPosition[1] = cellKey.cellCoord.y;
                                voronoiCell.shared.cellPosition[2] = cellKey.cellCoord.z;

                                unsigned int nci = 0;

                                glm::vec3 pt = pointChunk->points[i][j][k];
                                if (std::isnan(pt.x)) {
                                    continue;
                                }
                                pt -= (cellKey.cellCoord + spatial::UintCoord(-2)).toPoint();
                                pt *= 32.0f / 5.0f;
                                unsigned int nc = !hashTreeWorld.isGas(cell->second.chunk->cells[i][j][k].materialIndex);
                                nc = nc * 32 + std::max(0, std::min(static_cast<signed int>(pt.x), 31));
                                nc = nc * 32 + std::max(0, std::min(static_cast<signed int>(pt.y), 31));
                                nc = nc * 32 + std::max(0, std::min(static_cast<signed int>(pt.z), 31));
                                assert(nc < 0x10000);
                                voronoiCell.shared.neighborCells[nci++] = nc;

                                for (signed int di = -2; di <= 2; di++) {
                                    for (signed int dj = -2; dj <= 2; dj++) {
                                        for (signed int dk = -2; dk <= 2; dk++) {
                                            if (di == 0 && dj == 0 && dk == 0) {
                                                continue;
                                            }
                                            glm::vec3 pt = pointChunk->points[i + di][j + dj][k + dk];
                                            if (std::isnan(pt.x)) {
                                                continue;
                                            }
                                            pt -= (cellKey.cellCoord + spatial::UintCoord(-2)).toPoint();
                                            pt *= 32.0f / 5.0f;
                                            unsigned int nc = !hashTreeWorld.isGas(cell->second.chunk->cells[i + di][j + dj][k + dk].materialIndex);
                                            nc = nc * 32 + std::max(0, std::min(static_cast<signed int>(pt.x), 31));
                                            nc = nc * 32 + std::max(0, std::min(static_cast<signed int>(pt.y), 31));
                                            nc = nc * 32 + std::max(0, std::min(static_cast<signed int>(pt.z), 31));
                                            assert(nc < 0x10000);
                                            voronoiCell.shared.neighborCells[nci++] = nc;
                                        }
                                    }
                                }

                                assert(nci <= graphics::VoronoiCellShared::neighborCellCount);
                                while (nci < graphics::VoronoiCellShared::neighborCellCount) {
                                    voronoiCell.shared.neighborCells[nci++] = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (hasNonGas) {
        chunksLeft--;
    }

#ifndef NDEBUG
    cell->second.constantMaterialIndex = static_cast<world::MaterialIndex>(-1);
#endif

    context.get<world::HashTreeWorld>().updateGasMasks(&cell->second);

    /*
    if (hasNonGas) {
        unsigned int vertIndices[8];
        for (unsigned int i = 0; i < 2; i++) {
            for (unsigned int j = 0; j < 2; j++) {
                for (unsigned int k = 0; k < 2; k++) {
                    render::SceneManager::VertMutator vert = context.get<render::MeshUpdater>().getMeshHandle().createVert();
                    vert.shared.setPoint(cell->first.getCoord(i, j, k).toPoint());
                    vertIndices[i * 4 + j * 2 + k] = vert.index;
                }
            }
        }
        for (unsigned int i = 0; i < 100; i++) {
            render::SceneManager::FaceMutator face = context.get<render::MeshUpdater>().getMeshHandle().createFace();
            face.shared.verts[0] = vertIndices[rand() % 8];
            face.shared.verts[1] = vertIndices[rand() % 8];
            face.shared.verts[2] = vertIndices[rand() % 8];
        }
    }
    */
}

}
