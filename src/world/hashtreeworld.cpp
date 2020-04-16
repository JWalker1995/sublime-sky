#include "hashtreeworld.h"

#include <queue>

#include "schemas/config_client_generated.h"
#include "pointgen/chunkpointsmanager.h"
#include "worldgen/worldgenerator.h"
#include "render/meshupdater.h"
#include "render/camera.h"
#include "query/rectquery.h"
#include "util/pool.h"
#include "spatial/raydrawer.h"
#include "graphics/imgui.h"
#include "render/imguirenderer.h"

namespace world {

HashTreeWorld::HashTreeWorld(game::GameContext &gameContext, const SsProtocol::Config::HashTreeWorld *config)
    : TickableBase(gameContext)
    , viewChunkSubdivOffsetLog2(config->view_chunk_subdiv_offset_log2())
    , viewChunkLockSizeLog2(config->view_chunk_lock_size_log2())
    , cameraCoord(calcCameraCoord())
{}

void HashTreeWorld::tick(game::TickerContext &tickerContext) {
    tickerContext.get<render::ImguiRenderer::Ticker>();

    if (ImGui::Begin("Debug")) {
        ImGui::Text("Chunk hashtable size = %zu", getMap().size());
    }
    ImGui::End();

    assert(context.get<render::SceneManager>().getMaterialBuffer().getExtentSize() < std::numeric_limits<std::underlying_type<MaterialIndex>::type>::max());

    cameraCoord = calcCameraCoord();
}

spatial::UintCoord HashTreeWorld::calcCameraCoord() {
    return spatial::UintCoord::fromPoint(context.get<render::Camera>().getEyePos());
}

/*
HashTreeWorld::Cell &HashTreeWorld::lookupChunk(spatial::CellKey cellKey) {
    std::pair<std::unordered_map<spatial::CellKey, CellValue, spatial::CellKeyHasher>::iterator, bool> insert = getMap().emplace(cellKey, CellValue());
    if (insert.second) {
        // Inserted new chunk
        insert.first->second.initialize();

        // Insert parent node recursively
        lookupChunk(cellKey.parent());
    }
    return *insert.first;
}
*/

void HashTreeWorld::fixChunk(Cell *cell) {
    (void) cell;
}

void HashTreeWorld::removeChunk(Cell *cell) {
    context.get<util::Pool<world::Chunk>>().free(cell->second.chunk);
    cell->second.chunk = 0;

    pointgen::ChunkPointsManager &chunkPointsManager = context.get<pointgen::ChunkPointsManager>();
    chunkPointsManager.release(cell->second.points);
    cell->second.points = 0;

    cell->second.destroy();

    bool erased = getMap().erase(cell->first);
    assert(erased);
}

void HashTreeWorld::updateGasMasks(CellValue *cellValue) {
    using BitsetType = jw_util::Bitset<Chunk::size * Chunk::size * Chunk::size>;
    static_assert(BitsetType::size % BitsetType::wordBits == 0, "Bitset size is not a word multiple");

    if (cellValue->chunk) {
        cellValue->gasMasks[0].fill<false>();
        cellValue->gasMasks[1].fill<false>();
        cellValue->gasMasks[2].fill<false>();

        for (unsigned int i = 0; i < Chunk::size; i++) {
            for (unsigned int j = 0; j < Chunk::size; j++) {
                for (unsigned int k = 0; k < Chunk::size; k++) {
                    MaterialIndex material = cellValue->chunk->cells[i][j][k].materialIndex;
                    bool value = isGas(material);
                    if (value) {
                        cellValue->gasMasks[0].set<true>((i * Chunk::size + j) * Chunk::size + k);
                        cellValue->gasMasks[1].set<true>((j * Chunk::size + k) * Chunk::size + i);
                        cellValue->gasMasks[2].set<true>((k * Chunk::size + i) * Chunk::size + j);
                    }
                }
            }
        }
    } else {
        switch (cellValue->constantMaterialIndex) {
            case MaterialIndex::Null: assert(false); break;
            case MaterialIndex::Generating: assert(false); break;
            default:
                if (isGas(cellValue->constantMaterialIndex)) {
                    cellValue->gasMasks[0].fill<true>();
                    cellValue->gasMasks[1].fill<true>();
                    cellValue->gasMasks[2].fill<true>();
                } else {
                    cellValue->gasMasks[0].fill<false>();
                    cellValue->gasMasks[1].fill<false>();
                    cellValue->gasMasks[2].fill<false>();
                }
        }
    }
}

/*
CellValue &HashTreeWorld::getCellValueContaining(spatial::UintCoord coord) {
    return getLeafContaining(coord, Chunk::sizeLog2)->second;
}

MaterialIndex HashTreeWorld::getMaterialIndex(spatial::UintCoord coord) {
    Cell *leafNode = getLeafContaining(coord, Chunk::sizeLog2);
    unsigned int x = coord.x % Chunk::size;
    unsigned int y = coord.y % Chunk::size;
    unsigned int z = coord.z % Chunk::size;
    if (leafNode->second.type == CellValue::Type::LeafChunk) {
        return leafNode->second.chunk->cells[x][y][z].materialIndex;
    } else if (leafNode->second.type == CellValue::Type::LeafConstant) {
        return leafNode->second.constantMaterialIndex;
    } else {
        return static_cast<MaterialIndex>(-1);
    }
}

VoronoiCell &HashTreeWorld::getVoronoiCell(spatial::UintCoord coord) {
    Cell *leafNode = getLeafContaining(coord, Chunk::sizeLog2);
    unsigned int x = coord.x % Chunk::size;
    unsigned int y = coord.y % Chunk::size;
    unsigned int z = coord.z % Chunk::size;
    assert(leafNode->second.chunk);
    return leafNode->second.chunk->cells[x][y][z];
}

glm::vec3 HashTreeWorld::getPoint(spatial::UintCoord coord) {
    Cell *leafNode = getLeafContaining(coord, Chunk::sizeLog2);
    unsigned int x = coord.x % Chunk::size;
    unsigned int y = coord.y % Chunk::size;
    unsigned int z = coord.z % Chunk::size;
    return getChunkPoints(leafNode)->points[x][y][z];
}

bool &HashTreeWorld::getNeedsRegen(spatial::UintCoord coord) {
    Cell *leafNode = getLeafContaining(coord, Chunk::sizeLog2);
    unsigned int x = coord.x % Chunk::size;
    unsigned int y = coord.y % Chunk::size;
    unsigned int z = coord.z % Chunk::size;
    return leafNode->second.needsRegen[x][y][z];
}

unsigned int HashTreeWorld::getCellId(spatial::UintCoord coord) {
    Cell *leafNode = getLeafContaining(coord, Chunk::sizeLog2);
    unsigned int x = coord.x % Chunk::size;
    unsigned int y = coord.y % Chunk::size;
    unsigned int z = coord.z % Chunk::size;
    return leafNode->second.chunkId + ((x * Chunk::size) + y) * Chunk::size + z;
}
*/

HashTreeWorld::RaytestResult HashTreeWorld::testViewRay(glm::vec3 origin, glm::vec3 dir, float distanceLimit) {
    // Initialize sizeLog2 with a reasonable guess.
    // If it's incorrect, it'll be fixed.
    // On subsequent loop iterations, it'll just roll over. The previous size is the new guess.
    spatial::UintCoord initCoord = spatial::UintCoord::fromPoint(origin);
    unsigned int sizeLog2 = std::max(static_cast<signed int>(Chunk::sizeLog2), guessViewChunkSizeLog2(initCoord));

    // TODO: Iterate by chunk
    spatial::RayDrawer<glm::vec3> cellIt(origin, dir, spatial::CellKey::fromCoord(initCoord, sizeLog2 - Chunk::sizeLog2));

    while (true) {
        Cell *chunkNode;
        while (true) {
            chunkNode = getNodeContaining(cellIt.getCurCellKey().grandParent<Chunk::sizeLog2>());
            if (chunkNode->second.isLeaf()) {
                break;
            }
            cellIt.enterChildCell();
        }
        while (cellIt.getCurCellKey().sizeLog2 < chunkNode->first.sizeLog2 - Chunk::sizeLog2) {
            cellIt.enterParentCell();
        }

        if (chunkNode->second.chunk) {
            // In this case, we have cells for the chunk (as opposed to it being a constant chunk)

            spatial::RayDrawer<glm::vec3>::StepResult step;
            do {
                unsigned int x = cellIt.getCurCellKey().cellCoord.x % Chunk::size;
                unsigned int y = cellIt.getCurCellKey().cellCoord.y % Chunk::size;
                unsigned int z = cellIt.getCurCellKey().cellCoord.z % Chunk::size;

                MaterialIndex material = chunkNode->second.chunk->cells[x][y][z].materialIndex;
                RaytestResult res;
                switch (material) {
                case MaterialIndex::Null:
                    assert(false);
                    break;
                case MaterialIndex::Generating:
                    res.result = RaytestResult::HitGenerating;
                    return res;
                default:
                    if (!isGas(material)) {
                        res.result = RaytestResult::HitSurface;
                        res.surfaceMaterialIndex = material;
                        res.surfaceHitCell = cellIt.getCurCellKey();
                        return res;
                    }
                }

                step = cellIt.step();
                if (step.distance >= distanceLimit) {
                    res.result = RaytestResult::HitDistanceLimit;
                    res.surfaceMaterialIndex = material;
                    return res;
                }
            } while (!step.movedChunk);
        } else {
            MaterialIndex material = chunkNode->second.constantMaterialIndex;
            RaytestResult res;
            switch (material) {
            case MaterialIndex::Null:
                generateChunk(chunkNode);
                // Fall-through intentional
            case MaterialIndex::Generating:
                res.result = RaytestResult::HitGenerating;
                return res;
            default:
                if (isGas(material)) {
                    spatial::RayDrawer<glm::vec3>::StepResult step;
                    do {
                        step = cellIt.step();
                    } while (!step.movedChunk);
                    if (step.distance >= distanceLimit) {
                        res.result = RaytestResult::HitDistanceLimit;
                        res.surfaceMaterialIndex = material;
                        return res;
                    }
                } else {
                    res.result = RaytestResult::HitSurface;
                    res.surfaceMaterialIndex = material;
                    res.surfaceHitCell = cellIt.getCurCellKey();
                    return res;
                }
            }
        }
    }
}

/*
HashTreeWorld::RaytestResult HashTreeWorld::testRay(glm::vec3 origin, glm::vec3 dir, float distanceLimit) {
    dir /= glm::length(dir);
    glm::vec3 invDir = 1.0f / dir;

    auto calcStep = [](float originX, float dirX) {
        float t = std::floor(originX) - originX;
        if (dirX >= 0.0f) {
            t += 1.0f;
        }
        return t / dirX;
    };

    spatial::UintCoord coord = spatial::UintCoord::fromPoint(origin);
    glm::vec3 stepTime(calcStep(origin.x, dir.x), calcStep(origin.y, dir.y), calcStep(origin.z, dir.z));

    while (true) {
        if (stepTime.x < stepTime.y) {
            if (stepTime.x < stepTime.z) {
                if (dir.x >= 0.0f) {
                    coord.x++;
                    stepTime.x += invDir.x;
                } else {
                    coord.x--;
                    stepTime.x -= invDir.x;
                }
                if (stepTime.x > distanceLimit) { break; }
            } else {
                if (dir.z >= 0.0f) {
                    coord.z++;
                    stepTime.z += invDir.z;
                } else {
                    coord.z--;
                    stepTime.z -= invDir.z;
                }
                if (stepTime.z > distanceLimit) { break; }
            }
        } else {
            if (stepTime.y < stepTime.z) {
                if (dir.y >= 0.0f) {
                    coord.y++;
                    stepTime.y += invDir.y;
                } else {
                    coord.y--;
                    stepTime.y -= invDir.y;
                }
                if (stepTime.y > distanceLimit) { break; }
            } else {
                if (dir.z >= 0.0f) {
                    coord.z++;
                    stepTime.z += invDir.z;
                } else {
                    coord.z--;
                    stepTime.z -= invDir.z;
                }
                if (stepTime.z > distanceLimit) { break; }
            }
        }

        unsigned int materialIndex = getMaterialIndex(coord);
        if (materialIndex == static_cast<MaterialIndex>(-1)) {
            RaytestResult res;
            res.hitCoord = coord;
            res.materialIndex = static_cast<MaterialIndex>(-1);
            res.pointDistance = std::min({ stepTime.x, stepTime.y, stepTime.z });
            return res;
        } else if (!isTransparent(materialIndex)) {
            RaytestResult res;
            res.hitCoord = coord;
            res.materialIndex = materialIndex;
            res.pointDistance = std::min({ stepTime.x, stepTime.y, stepTime.z });
            return res;
        }
    }

    RaytestResult res;
    res.hitCoord = spatial::UintCoord(0);
    res.materialIndex = static_cast<MaterialIndex>(-1);
    res.pointDistance = distanceLimit;
    return res;
}

HashTreeWorld::RaytestResult HashTreeWorld::testRaySlow(glm::vec3 origin, glm::vec3 dir, float distanceLimit) {
    dir /= glm::length(dir);
    glm::vec3 invDir = 1.0f / dir;

    spatial::UintCoord coord = spatial::UintCoord::fromPoint(origin);

    struct EnqueuedCell {
        float distParallel; // Distance along ray
        float distPerpendicularSq; // Distance from ray
        spatial::UintCoord fromCell;
        MaterialIndex materialIndex;
    };
    std::vector<EnqueuedCell> cellQueue;

    static constexpr float maxDistanceChange = 2.0f;
    float curDistPll = 0.0f;
    float curDistPrpSq = 0.0f;
    float prevSurfaceDist = 0.0f;
    float loadDistance = -maxDistanceChange;

    while (true) {
        if (curDistPll < loadDistance) {
            // Use loaded cells

            float minSurfaceDist = std::numeric_limits<float>::infinity();
            EnqueuedCell *minCell;

            std::vector<EnqueuedCell>::iterator i = cellQueue.begin();
            while (i != cellQueue.end()) {
                float surfaceDist = 0.5f * (curDistPrpSq + curDistPll * curDistPll - i->distPerpendicularSq - i->distParallel * i->distParallel) / (curDistPll - i->distParallel);

                if (surfaceDist < prevSurfaceDist || surfaceDist < curDistPll) {
                    *i = cellQueue.back();
                    cellQueue.pop_back();
                    continue;
                }

                if (surfaceDist < minSurfaceDist) {
                    minSurfaceDist = surfaceDist;
                    minCell = &*i;
                }

                i++;
            }

            if (minSurfaceDist == std::numeric_limits<float>::infinity()) {
                break;
            }
            prevSurfaceDist = minSurfaceDist;

            if (!isTransparent(minCell->materialIndex)) {
                RaytestResult res;
                res.hitCoord = minCell->fromCell;
                res.materialIndex = minCell->materialIndex;
                // res.surfaceDistance = minSurfaceDist; // Not giving this means we can skip uniform chunks.
                res.pointDistance = minCell->distParallel;
                return res;
            }

//            assert(minCell->distParallel > curDistPll);

            curDistPll = minCell->distParallel;
            curDistPrpSq = minCell->distPerpendicularSq;

            *minCell = cellQueue.back();
            cellQueue.pop_back();
        } else {
            // Load next chunk

            // TODO: Make sure close-by chunks, even if not on the ray, are loaded

            Cell *leafNode = getLeafContaining(coord, Chunk::sizeLog2);
            if (leafNode->second.type == CellValue::Type::LeafChunk) {
                const pointgen::Chunk *pointChunk = getChunkPoints(leafNode);
                float maxDistPrpSq = 1.5f * (leafNode->first.getSize() / Chunk::size);
                maxDistPrpSq *= maxDistPrpSq;

                for (unsigned int i = 0; i < Chunk::size; i++) {
                    for (unsigned int j = 0; j < Chunk::size; j++) {
                        for (unsigned int k = 0; k < Chunk::size; k++) {
                            EnqueuedCell cellCandidate;
                            glm::vec3 x = pointChunk->points[i][j][k] - origin;
                            cellCandidate.distParallel = glm::dot(x, dir);
                            if (cellCandidate.distParallel < distanceLimit) {
                                cellCandidate.distPerpendicularSq = glm::distance2(x, cellCandidate.distParallel * dir);
                                if (cellCandidate.distPerpendicularSq < maxDistPrpSq) {
                                    cellCandidate.fromCell = leafNode->first.grandChild<Chunk::sizeLog2>(i, j, k).getCoord<0, 0, 0>();
                                    cellCandidate.materialIndex = leafNode->second.chunk->cells[i][j][k].materialIndex;
                                    cellQueue.push_back(cellCandidate);
                                }
                            }
                        }
                    }
                }
            } else if (leafNode->second.type == CellValue::Type::LeafGenerating) {
                assert(false); // TODO: Need to build this result correctly
                RaytestResult res;
                res.materialIndex = static_cast<MaterialIndex>(-1);
                // res.surfaceDistance = minSurfaceDist; // Not giving this means we can skip uniform chunks.
                res.pointDistance = loadDistance + maxDistanceChange;
                return res;
            } else if (!isTransparent(leafNode->second.constantMaterialIndex)) {
                assert(leafNode->second.type == CellValue::Type::LeafConstant);

                assert(false); // TODO: Need to build this result correctly
                RaytestResult res;
                res.materialIndex = leafNode->second.constantMaterialIndex;
                // res.surfaceDistance = minSurfaceDist; // Not giving this means we can skip uniform chunks.
                res.pointDistance = loadDistance + maxDistanceChange;
                return res;
            }

            glm::vec3 farLimits = leafNode->first.getCoord(dir.x > 0.0f, dir.y > 0.0f, dir.z > 0.0f).toPoint();
            glm::vec3 limitDists = (farLimits - origin) * invDir;
            float minDist = limitDists.x < limitDists.y ? limitDists.x : limitDists.y;
            minDist = minDist < limitDists.z ? minDist : limitDists.z;
            if (minDist > distanceLimit) {
                break;
            }

            if (minDist == limitDists.x) {
                coord = leafNode->first.getCoord(dir.x > 0.0f ? 1 : -1, 0, 0);
            } else if (minDist == limitDists.y) {
                coord = leafNode->first.getCoord(0, dir.y > 0.0f ? 1 : -1, 0);
            } else if (minDist == limitDists.z) {
                coord = leafNode->first.getCoord(0, 0, dir.z > 0.0f ? 1 : -1);
            } else {
                assert(false);
            }

            loadDistance = minDist - maxDistanceChange;
        }
    }

    RaytestResult res;
    res.hitCoord = spatial::UintCoord(0);
    res.materialIndex = static_cast<MaterialIndex>(-1);
    res.pointDistance = distanceLimit;
    return res;
}

spatial::UintCoord HashTreeWorld::getContainingCoord(glm::vec3 point) {
    spatial::UintCoord coord = spatial::UintCoord::fromPoint(point);

    float minDistSq = std::numeric_limits<float>::infinity();
    spatial::UintCoord closestCoord;

    spatial::UintCoord min = coord - spatial::UintCoord(1);
    spatial::UintCoord max = coord + spatial::UintCoord(1);
    spatial::UintCoord neighborCoord;
    for (neighborCoord.x = min.x; neighborCoord.x <= max.x; neighborCoord.x++) {
        for (neighborCoord.y = min.y; neighborCoord.y <= max.y; neighborCoord.y++) {
            for (neighborCoord.z = min.z; neighborCoord.z <= max.z; neighborCoord.z++) {
                glm::vec3 cellPoint = getPoint(neighborCoord);

                if (!std::isnan(cellPoint.x)) {
                    float distSq = glm::distance2(point, cellPoint);
                    if (distSq < minDistSq) {
                        minDistSq = distSq;
                        closestCoord = neighborCoord;
                    }
                }
            }
        }
    }

    return closestCoord;
}

void HashTreeWorld::emitMeshUpdate(glm::vec3 changedMin, glm::vec3 changedMax, float pointSpacing) {
    static thread_local std::vector<std::pair<unsigned int, glm::vec3>> separatedPoints[2];
    assert(separatedPoints[0].empty());
    assert(separatedPoints[1].empty());

    spatial::UintCoord min = spatial::UintCoord::fromPoint(changedMin - pointSpacing * 2.0f);
    spatial::UintCoord max = spatial::UintCoord::fromPoint(changedMax + pointSpacing * 2.0f) + spatial::UintCoord(1, 1, 1);
    query::RectCoordQuery query(min, max);

    std::size_t sizeGuess = (max.x - min.x) * (max.y - min.y) * (max.z - min.z);
    separatedPoints[0].resize(sizeGuess);
    separatedPoints[1].resize(sizeGuess);

    Iterator<query::RectCoordQuery, false> i(*this, query);
    spatial::CellKey initKey = spatial::CellKey::fromCoords(min, max);
    i.init(initKey);

    while (i.has()) {
        if (i.get()->second.state == SpaceState::SubdividedAsChunk) {
            Chunk *chunk = i.get()->second.chunk;
            const pointgen::Chunk *points = getChunkPoints(i.get());

            for (unsigned int i = 0; i < Chunk::size; i++) {
                for (unsigned int j = 0; j < Chunk::size; j++) {
                    for (unsigned int k = 0; k < Chunk::size; k++) {
                        SpaceState type = chunk->cells[i][j][k].type;
                        bool isTransparent = type.isTransparent();
                        separatedPoints[isTransparent].emplace_back(type.value, points->points[i][j][k]);
                    }
                }
            }
        }

        i.advance();
    }

    context.get<render::MeshUpdater>().update(changedMin - pointSpacing, changedMax + pointSpacing, separatedPoints[0], separatedPoints[1]);

    separatedPoints[0].clear();
    separatedPoints[1].clear();
}
*/

MaterialIndex HashTreeWorld::queryMaterialAt(glm::vec3 point) {
    spatial::UintCoord coord = spatial::UintCoord::fromPoint(point);

    float minDistSq = std::numeric_limits<float>::infinity();
    MaterialIndex closestMaterialIndex;

    spatial::UintCoord min = coord - spatial::UintCoord(1);
    spatial::UintCoord max = coord + spatial::UintCoord(1);
    spatial::UintCoord neighborCoord;
    for (neighborCoord.x = min.x; neighborCoord.x <= max.x; neighborCoord.x++) {
        for (neighborCoord.y = min.y; neighborCoord.y <= max.y; neighborCoord.y++) {
            for (neighborCoord.z = min.z; neighborCoord.z <= max.z; neighborCoord.z++) {
                Cell *cell = getLeafContaining(neighborCoord, Chunk::sizeLog2);
                unsigned int x = neighborCoord.x % Chunk::size;
                unsigned int y = neighborCoord.y % Chunk::size;
                unsigned int z = neighborCoord.z % Chunk::size;
                glm::vec3 cellPoint = getChunkPoints(cell)->points[x][y][z];

                if (!std::isnan(cellPoint.x)) {
                    float distSq = glm::distance2(point, cellPoint);
                    if (distSq < minDistSq) {
                        minDistSq = distSq;
                        closestMaterialIndex = cell->second.chunk ? cell->second.chunk->cells[x][y][z].materialIndex : cell->second.constantMaterialIndex;
                    }
                }
            }
        }
    }

    return closestMaterialIndex;
}

const pointgen::Chunk *HashTreeWorld::getChunkPoints(Cell *cell) {
    pointgen::ChunkPointsManager &chunkPointsManager = context.get<pointgen::ChunkPointsManager>();
    if (!cell->second.points) {
        cell->second.points = chunkPointsManager.generate(cell->first);
    }
    chunkPointsManager.use(cell->second.points);
    return cell->second.points;
}

bool HashTreeWorld::isGas(MaterialIndex materialIndex) const {
    assert(materialIndex != static_cast<MaterialIndex>(-1));

    // TODO: Maybe cache this lookup?
    render::SceneManager &sceneManager = context.get<render::SceneManager>();
    return sceneManager.readMaterial(static_cast<unsigned int>(materialIndex)).local.phase == graphics::MaterialLocal::Phase::Gas;
}

void HashTreeWorld::generateChunk(Cell *cell) {
    assert(cell->second.chunk == 0);
    assert(cell->second.constantMaterialIndex == MaterialIndex::Null);

    if (cell->first.sizeLog2 > Chunk::sizeLog2) {
        // There might be leaf-level chunks that we can deduce some materials from
        // There's really three cases here:
        // 1. There's no leaf-level chunks, so we can just set constantMaterialIndex = MaterialIndex::Generating.
        // 2. There's some leaf-level chunks, so we should allocate a new world::Chunk
        //      and fill each cell with either MaterialIndex::Generating or the leaf-level material.
        // 3. The entire cell's covered with leaf-level chunks, so we can fill this entire chunk and skip the generation request.
        // For now, just assume there's no leaf-level chunks.
    }

    cell->second.constantMaterialIndex = MaterialIndex::Generating;

    context.get<worldgen::WorldGenerator>().generate(cell->first, getChunkPoints(cell));
}

unsigned int CellValue::nextChunkId = 0;

}
