#include "raycastlookuptable.h"

#include <queue>

#include "spdlog/spdlog.h"
#include "game/gamecontext.h"

namespace spatial {

RaycastLookupTable::RaycastLookupTable(game::GameContext &context) {
    context.get<spdlog::logger>().debug("Sizeof(RaycastLookupTable) = {}", sizeof(RaycastLookupTable));

    for (std::size_t i = 0; i < sizeof(sequences) / sizeof(SearchSequence); i++) {
        SearchSequence &searchSequence = (&sequences[0][0][0][0][0][0][0])[i];

        auto cellComp = [](const SearchSequence::Cell &a, const SearchSequence::Cell &b) -> bool {
            return a.minSurfaceDistance > b.minSurfaceDistance;
        };

        std::vector<SearchSequence::Cell> finishedCells;
        std::priority_queue<SearchSequence::Cell, std::vector<SearchSequence::Cell>, decltype(cellComp)> cellQueue(cellComp);

        float offsets[10];
        std::fill_n(offsets, 10, 0.5f);
        glm::vec3 initDir = searchSequenceToLookupQuery(searchSequence, offsets).dir;

        SearchSequence::Cell initialCell;
        // TODO: make sure queue is unique. Better initialization.
        initialCell.offsetX = 0.5f + initDir.x * 1.8f;
        initialCell.offsetY = 0.5f + initDir.y * 1.8f;
        initialCell.offsetZ = 0.5f + initDir.z * 1.8f;
        initialCell.minSurfaceDistance = findSmallestSurfaceDistance(searchSequence, initialCell.offsetX, initialCell.offsetY, initialCell.offsetZ);
        cellQueue.push(initialCell);

        while (!cellQueue.empty()) {
            SearchSequence::Cell cell = cellQueue.top();
            cellQueue.pop();

            if (cell.minSurfaceDistance > -1e-3 && cell.minSurfaceDistance < 2.0f) {
                finishedCells.push_back(cell);

                cell.offsetX--;
                cell.minSurfaceDistance = findSmallestSurfaceDistance(searchSequence, cell.offsetX, cell.offsetY, cell.offsetZ);
                cellQueue.push(cell);
                cell.offsetX++;

                cell.offsetX++;
                cell.minSurfaceDistance = findSmallestSurfaceDistance(searchSequence, cell.offsetX, cell.offsetY, cell.offsetZ);
                cellQueue.push(cell);
                cell.offsetX--;

                cell.offsetY--;
                cell.minSurfaceDistance = findSmallestSurfaceDistance(searchSequence, cell.offsetX, cell.offsetY, cell.offsetZ);
                cellQueue.push(cell);
                cell.offsetY++;

                cell.offsetY++;
                cell.minSurfaceDistance = findSmallestSurfaceDistance(searchSequence, cell.offsetX, cell.offsetY, cell.offsetZ);
                cellQueue.push(cell);
                cell.offsetY--;

                cell.offsetZ--;
                cell.minSurfaceDistance = findSmallestSurfaceDistance(searchSequence, cell.offsetX, cell.offsetY, cell.offsetZ);
                cellQueue.push(cell);
                cell.offsetZ++;

                cell.offsetZ++;
                cell.minSurfaceDistance = findSmallestSurfaceDistance(searchSequence, cell.offsetX, cell.offsetY, cell.offsetZ);
                cellQueue.push(cell);
                cell.offsetZ--;
            }
        }

        sort(finishedCells.begin(), finishedCells.end(), cellComp);

        // We shouldn't ever get to the end, but if we do, this should stop the loop
        initialCell.minSurfaceDistance = std::numeric_limits<float>::infinity();
        finishedCells.resize(sequenceLength, initialCell);
        std::copy_n(finishedCells.cbegin(), sequenceLength, searchSequence.cells);
    }
}

const RaycastLookupTable::SearchSequence &RaycastLookupTable::lookup(glm::vec3 origin, glm::vec3 dir, float parallelDist, float perpDistSq) const {
    unsigned int osx;
    unsigned int osy;
    unsigned int osz;
    if (originSplits == 1) {
        osx = 0;
        osy = 0;
        osz = 0;
    } else {
        osx = (origin.x - std::floor(origin.x)) * originSplits;
        osy = (origin.y - std::floor(origin.y)) * originSplits;
        osz = (origin.z - std::floor(origin.z)) * originSplits;
        assert(osx < originSplits);
        assert(osy < originSplits);
        assert(osz < originSplits);
    }

    float dpx = std::atan2f(dir.y, dir.x); // [-pi, +pi]
    float dpy = dir.z / glm::length(dir); // [-1, 1]
    unsigned int dsx = (dpx + M_PI) * (0.5f * dirSplits / M_PI);
    unsigned int dsy = (dpy + 1.0f) * (0.5f * dirSplits);
    assert(dsx < dirSplits);
    assert(dsy < dirSplits);

    unsigned int pls;
    if (parallelDistSplits == 1) {
        pls = 0;
    } else {
        assert(parallelDist >= minParallelDist);
        assert(parallelDist < maxParallelDist);
        pls = (parallelDist - minParallelDist) * (parallelDistSplits / (maxParallelDist - minParallelDist));
        assert(pls < parallelDistSplits);
    }

    unsigned int prs;
    if (perpDistSplits == 1) {
        prs = 0;
    } else {
        assert(perpDistSq >= 0.0f);
        assert(perpDistSq < maxPerpDistSq);
        prs = perpDistSq * (perpDistSplits / maxPerpDistSq);
        assert(prs < perpDistSplits);
    }

    return sequences[osx][osy][osz][dsx][dsy][pls][prs];
}

RaycastLookupTable::Query RaycastLookupTable::searchSequenceToLookupQuery(const SearchSequence &searchSequence, float offs[7]) const {
    std::size_t index = &searchSequence - &sequences[0][0][0][0][0][0][0];
    unsigned int prs = index % perpDistSplits;
    index /= perpDistSplits;
    unsigned int pls = index % parallelDistSplits;
    index /= parallelDistSplits;
    unsigned int dsy = index % dirSplits;
    index /= dirSplits;
    unsigned int dsx = index % dirSplits;
    index /= dirSplits;
    unsigned int osz = index % originSplits;
    index /= originSplits;
    unsigned int osy = index % originSplits;
    index /= originSplits;
    unsigned int osx = index % originSplits;
    index /= originSplits;
    assert(index == 0);

    Query res;
    res.origin.x = (osx + offs[0]) / originSplits;
    res.origin.y = (osy + offs[1]) / originSplits;
    res.origin.z = (osz + offs[2]) / originSplits;

    float dpx = (dsx + offs[3]) / (0.5f * dirSplits / M_PI) - M_PI;
    float dpy = (dsy + offs[4]) / (0.5f * dirSplits) - 1.0f;
    float xyScale = std::sqrtf(1.0f - dpy * dpy);
    res.dir.x = std::cosf(dpx) * xyScale;
    res.dir.y = std::sinf(dpx) * xyScale;
    res.dir.z = dpy;

    // This should be pretty normalized, but normalize it again just to make sure.
    // getRayPointDistanceSq depends on it being normalized.
    float dirLen = glm::length(res.dir);
    assert(dirLen > 0.999f && dirLen < 1.001f);
    res.dir /= dirLen;

    res.parallelDist = (pls + offs[5]) / (parallelDistSplits / (maxParallelDist - minParallelDist)) + minParallelDist;

    res.perpDistSq = (prs + offs[6]) / (perpDistSplits / maxPerpDistSq);

#ifndef NDEBUG
    // This should be true most of the time, but on the boundaries weird stuff can happen.
    bool testLookup = true;
    for (unsigned int i = 0; i < 7; i++) {
        testLookup &= offs[i] >= 1e-3f && offs[i] < 1 - 1e-3f;
    }
    if (testLookup) {
        assert(&lookup(res.origin, res.dir, res.parallelDist, res.perpDistSq) == &searchSequence);
    }
#endif

    return res;
}

float RaycastLookupTable::getSurfaceDistanceAlongRay(glm::vec3 origin, glm::vec3 dir, float parallelDist, float perpDistSq, glm::vec3 cellPoint) {
    cellPoint -= origin;

    float cellDistParallel = glm::dot(cellPoint, dir);
    float cellDistPerpSq = glm::distance2(cellPoint, cellDistParallel * dir);
    float surfaceDist = 0.5f * (parallelDist * parallelDist + perpDistSq - cellDistParallel * cellDistParallel - cellDistPerpSq) / (parallelDist - cellDistParallel);

    return surfaceDist;
}

float RaycastLookupTable::findSmallestSurfaceDistance(const SearchSequence &searchSequence, signed int offsetX, signed int offsetY, signed int offsetZ) {
    glm::vec3 cellPoint(offsetX, offsetY, offsetZ);

    static constexpr float eps = 1e-3f;
    float offsets[10];
    std::fill_n(offsets, 10, 0.5f);

    float temperature = 100.0f;
    for (unsigned int j = 0; j < 1024; j++) {
        Query query = searchSequenceToLookupQuery(searchSequence, offsets);
        float baseSd = getSurfaceDistanceAlongRay(query.origin, query.dir, query.parallelDist, query.perpDistSq, cellPoint + glm::vec3(offsets[7], offsets[8], offsets[9]));
        // We don't want to go below zero, so square it.
        baseSd *= baseSd;

        float ds[10];
        for (unsigned int k = 0; k < 10; k++) {
            offsets[k] += eps;

            Query dQuery = searchSequenceToLookupQuery(searchSequence, offsets);
            float dSd = getSurfaceDistanceAlongRay(dQuery.origin, dQuery.dir, dQuery.parallelDist, dQuery.perpDistSq, cellPoint + glm::vec3(offsets[7], offsets[8], offsets[9]));
            dSd *= dSd;
            ds[k] = (dSd - baseSd) / eps;

            offsets[k] -= eps;
        }

        for (unsigned int k = 0; k < 10; k++) {
            offsets[k] -= ds[k] * temperature;
            if (offsets[k] < 0.0f) {
                offsets[k] = 0.0f;
            } else if (offsets[k] > 1.0f) {
                offsets[k] = 1.0f;
            }
        }

        temperature *= 0.99f;
    }

    Query query = searchSequenceToLookupQuery(searchSequence, offsets);
    float finalSd = getSurfaceDistanceAlongRay(query.origin, query.dir, query.parallelDist, query.perpDistSq, cellPoint);
    return finalSd;
}

}
