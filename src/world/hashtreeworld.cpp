#include "hashtreeworld.h"

#include <queue>

#include "pointgen/chunkpointsmanager.h"
#include "worldgen/worldgenerator.h"
#include "render/meshupdater.h"
#include "query/rectquery.h"
#include "util/pool.h"

namespace world {

HashTreeWorld::HashTreeWorld(game::GameContext &gameContext)
    : TickableBase(gameContext)
{

}

void HashTreeWorld::tick(game::TickerContext &tickerContext) {
    (void) tickerContext;

    assert(context.get<render::SceneManager>().getMaterialBuffer().getExtentSize() < std::numeric_limits<decltype(VoronoiCell::materialIndex)>::max());
}

CellValue &HashTreeWorld::getCellValueContaining(spatial::UintCoord coord) {
    return getLeafContaining(coord, Chunk::sizeLog2)->second;
}

VoronoiCell::MaterialIndex HashTreeWorld::getMaterialIndex(spatial::UintCoord coord) {
    Cell *leafNode = getLeafContaining(coord, Chunk::sizeLog2);
    unsigned int x = coord.x % Chunk::size;
    unsigned int y = coord.y % Chunk::size;
    unsigned int z = coord.z % Chunk::size;
    if (leafNode->second.type == CellValue::Type::LeafChunk) {
        return leafNode->second.chunk->cells[x][y][z].materialIndex;
    } else if (leafNode->second.type == CellValue::Type::LeafConstant) {
        return leafNode->second.constantMaterialIndex;
    } else {
        return static_cast<VoronoiCell::MaterialIndex>(-1);
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
        if (materialIndex == static_cast<VoronoiCell::MaterialIndex>(-1)) {
            RaytestResult res;
            res.hitCoord = coord;
            res.materialIndex = static_cast<VoronoiCell::MaterialIndex>(-1);
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
    res.materialIndex = static_cast<VoronoiCell::MaterialIndex>(-1);
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
        VoronoiCell::MaterialIndex materialIndex;
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
                res.materialIndex = static_cast<VoronoiCell::MaterialIndex>(-1);
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
    res.materialIndex = static_cast<VoronoiCell::MaterialIndex>(-1);
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

const pointgen::Chunk *HashTreeWorld::getChunkPoints(Cell *cell) {
    pointgen::ChunkPointsManager &chunkPointsManager = context.get<pointgen::ChunkPointsManager>();
    if (!cell->second.points) {
        cell->second.points = chunkPointsManager.generate(cell->first);
    }
    chunkPointsManager.use(cell->second.points);
    return cell->second.points;
}

void HashTreeWorld::requestWorldGen(Cell *cell) {
    // TODO: Hopefully the point chunk doesn't get re-allocated before the request is finished
    context.get<worldgen::WorldGenerator>().generate(cell->first, getChunkPoints(cell));
}

void HashTreeWorld::finishWorldGen(spatial::CellKey cube, VoronoiCell::MaterialIndex constantMaterialIndex, Chunk *chunk) {
    Cell *node = getNodeContaining(cube);
    assert(node->second.type == CellValue::Type::LeafGenerating);

    if (chunk) {
        node->second.type = CellValue::Type::LeafChunk;
        node->second.chunk = chunk;
    } else {
        node->second.type = CellValue::Type::LeafConstant;
        node->second.constantMaterialIndex = constantMaterialIndex;
    }

    std::fill_n(&node->second.needsRegen[0][0][0], Chunk::size * Chunk::size * Chunk::size, true);

    /*
    if (node->first.sizeLog2 == 4) {
        render::MeshUpdater &meshUpdater = context.get<render::MeshUpdater>();

        static constexpr unsigned int padding = 2;
        for (unsigned int i = padding; i < world::Chunk::size - padding; i++) {
            for (unsigned int j = padding; j < world::Chunk::size - padding; j++) {
                for (unsigned int k = padding; k < world::Chunk::size - padding; k++) {
                    world::SpaceState state = node->second.chunk->cells[i][j][k].type;
                    if (!state.isTransparent()) {
                        meshUpdater.updateCell<false>(node->first.getCoord<0, 0, 0>() + spatial::UintCoord(i, j, k));
                    }
                }
            }
        }
    }
    */

//    glm::vec3 changedMin = worldGenRequest->getCube().getCoord<0, 0, 0>().toPoint();
//    glm::vec3 changedMax = worldGenRequest->getCube().getCoord<1, 1, 1>().toPoint();
//    float pointSpacing = worldGenRequest->getCube().getSize() / Chunk::size;
//    emitMeshUpdate(changedMin, changedMax, pointSpacing);
}

/*
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

bool HashTreeWorld::isTransparent(VoronoiCell::MaterialIndex materialIndex) const {
    assert(materialIndex != static_cast<VoronoiCell::MaterialIndex>(-1));

    // TODO: Maybe cache this lookup?
    render::SceneManager &sceneManager = context.get<render::SceneManager>();
    return sceneManager.readMaterial(materialIndex).local.phase == graphics::MaterialLocal::Phase::Gas;
}

unsigned int CellValue::nextChunkId = 0;

}
