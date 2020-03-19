#include "hashtreeworld.h"

#include <queue>

#include "pointgen/chunkpointsmanager.h"
#include "worldgen/worldgenerator.h"
#include "util/pool.h"

namespace world {

HashTreeWorld::HashTreeWorld(game::GameContext &gameContext)
    : TickableBase(gameContext)
{

}

void HashTreeWorld::tick(game::TickerContext &tickerContext) {
    (void) tickerContext;
}

CellValue &HashTreeWorld::getCellValueContaining(glm::vec3 point) {
    return getLeafContaining(spatial::UintCoord::fromPoint(point), Chunk::sizeLog2)->second;
}

HashTreeWorld::RaytestResult HashTreeWorld::testRay(glm::vec3 origin, glm::vec3 dir, float distanceLimit) {
    dir /= glm::length(dir);
    glm::vec3 invDir = 1.0f / dir;

    spatial::UintCoord coord = spatial::UintCoord::fromPoint(origin);

    struct EnqueuedCell {
        float distParallel; // Distance along ray
        float distPerpendicularSq; // Distance from ray
        spatial::UintCoord fromCell;
        SpaceState state;
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

                if (surfaceDist < prevSurfaceDist) {
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

            if (!minCell->state.isTransparent()) {
                RaytestResult res;
                res.state = minCell->state;
                // res.surfaceDistance = minSurfaceDist; // Not giving this means we can skip uniform chunks.
                res.pointDistance = minCell->distParallel;
                return res;
            }

            assert(minCell->distParallel > curDistPll);
            assert(minCell->distParallel - curDistPll <= maxDistanceChange);

            curDistPll = minCell->distParallel;
            curDistPrpSq = minCell->distPerpendicularSq;

            *minCell = cellQueue.back();
            cellQueue.pop_back();
        } else {
            // Load next chunk

            // TODO: Make sure close-by chunks, even if not on the ray, are loaded

            Cell *leafNode = getLeafContaining(coord, Chunk::sizeLog2);
            if (leafNode->second.state == SpaceState::SubdividedAsChunk) {
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
                                    cellCandidate.state = leafNode->second.chunk->cells[i][j][k].type;
                                    cellQueue.push_back(cellCandidate);
                                }
                            }
                        }
                    }
                }
            } else if (!leafNode->second.state.isTransparent()) {
                RaytestResult res;
                res.state = leafNode->second.state;
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
    res.state = SpaceState::Air;
    res.pointDistance = distanceLimit;
    return res;
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

    Chunk *chunk = context.get<util::Pool<Chunk>>().alloc();
    WorldGenRequest *request = context.get<util::Pool<WorldGenRequest>>().alloc(*this, cell->first, getChunkPoints(cell), chunk);
    context.get<worldgen::WorldGenerator>().generate(request);
}

void HashTreeWorld::finishWorldGen(const WorldGenRequest *worldGenRequest, SpaceState chunkState) {
    Cell *node = getNodeContaining(worldGenRequest->getCube());
    assert(node->second.state == SpaceState::Generating);

    node->second.state = chunkState;
    if (chunkState == SpaceState::SubdividedAsChunk) {
        node->second.chunk = worldGenRequest->getDstChunk();
    } else {
        context.get<util::Pool<Chunk>>().free(worldGenRequest->getDstChunk());
    }

    context.get<util::Pool<WorldGenRequest>>().free(worldGenRequest);
}

}
