#include "raycastlookuptable.h"

#include <fstream>
#include <deque>

#include <nlopt.h>

#include <glm/gtx/norm.hpp>

#include "spdlog/spdlog.h"
#include "game/gamecontext.h"

namespace spatial {

RaycastLookupTable::RaycastLookupTable(game::GameContext &context)
    : context(context)
{}

/*
static const char *filename = "raycastlookuptable_cache.bin";

if (!loadSequences(filename)) {
    generateSequences();
    writeSequences(filename);
}
*/

bool RaycastLookupTable::loadSequences(const char *filename) {
    context.get<spdlog::logger>().info("Attempting to read sequences from file '{}'...", filename);

    std::ifstream file;
    file.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        std::size_t size = file.tellg();
        if (size == sizeof(sequences)) {
            file.seekg(0, std::ios::beg);
            file.read(reinterpret_cast<char *>(sequences), sizeof(sequences));
            file.close();

            context.get<spdlog::logger>().info("Read sequences from file '{}'!", filename);
            return true;
        }
    }

    context.get<spdlog::logger>().warn("Could not read sequences from file '{}'!", filename);
    return false;
}

void RaycastLookupTable::writeSequences(const char *filename) {
    context.get<spdlog::logger>().info("Writing sequences to file '{}'...", filename);

    std::ofstream file;
    file.open(filename, std::ios::out | std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char *>(sequences), sizeof(sequences));
        file.close();

        context.get<spdlog::logger>().info("Finished writing sequences to file '{}'!", filename);
    } else {
        context.get<spdlog::logger>().warn("Could not write sequences to file '{}'!", filename);
    }
}

void RaycastLookupTable::generateSequences() {
    context.get<spdlog::logger>().debug("Sizeof(RaycastLookupTable::sequences) = {}", sizeof(sequences));

    static constexpr std::size_t numSequences = sizeof(sequences) / sizeof(SearchSequence);
    for (std::size_t i = 0; i < numSequences; i++) {
        if (i % 1000 == 0) {
            context.get<spdlog::logger>().info("Generated {} / {} sequences...", i, numSequences);
        }

        generateSingleSequence((&sequences[0][0][0][0][0][0][0])[i]);
    }

    context.get<spdlog::logger>().info("Done! Generated {} sequences!", numSequences);
}

const RaycastLookupTable::SearchSequence &RaycastLookupTable::lookup(glm::vec3 origin, glm::vec3 dir, float parallelDist, float perpDistSq) {
    SearchSequence &res = getSequence(origin, dir, parallelDist, perpDistSq);
    if (!res.isGenerated()) {
        generateSingleSequence(res);
        assert(res.isGenerated());
    }
    return res;
}

RaycastLookupTable::SearchSequence &RaycastLookupTable::getSequence(glm::vec3 origin, glm::vec3 dir, float parallelDist, float perpDistSq) {
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

void RaycastLookupTable::generateSingleSequence(SearchSequence &searchSequence) {
    std::deque<SearchSequence::Cell> cells;

    double offsets[7];
    std::fill_n(offsets, 7, 0.5);
    glm::vec3 initDir = searchSequenceToLookupQuery(searchSequence, offsets).dir;
    SearchSequence::Cell initialCell(0.5f + initDir.x * 1.8f, 0.5f + initDir.y * 1.8f, 0.5f + initDir.z * 1.8f, NAN);
    cells.push_back(initialCell);

    for (std::size_t j = 0; j < cells.size();) {
        SearchSequence::Cell &cell = cells[j];

        if (std::find(cells.cbegin(), cells.cbegin() + j, cell) == cells.cbegin() + j) {
            setMinSurfaceDistance(cell, searchSequence);
            if (cell.minSurfaceDistance >= surfaceDistanceLowerBound && cell.minSurfaceDistance < surfaceDistanceUpperBound) {
                cells.emplace_back(cell.offsetX - 1, cell.offsetY, cell.offsetZ, NAN);
                cells.emplace_back(cell.offsetX + 1, cell.offsetY, cell.offsetZ, NAN);
                cells.emplace_back(cell.offsetX, cell.offsetY - 1, cell.offsetZ, NAN);
                cells.emplace_back(cell.offsetX, cell.offsetY + 1, cell.offsetZ, NAN);
                cells.emplace_back(cell.offsetX, cell.offsetY, cell.offsetZ - 1, NAN);
                cells.emplace_back(cell.offsetX, cell.offsetY, cell.offsetZ + 1, NAN);

                j++;
                continue;
            }
        }

        // Erase and retry
        cell = cells.back();
        cells.pop_back();
    }

    auto cellComp = [](const SearchSequence::Cell &a, const SearchSequence::Cell &b) -> bool {
        return a.minSurfaceDistance < b.minSurfaceDistance;
    };
    std::sort(cells.begin(), cells.end(), cellComp);

    // Pad with sentinels
    initialCell.minSurfaceDistance = std::numeric_limits<float>::infinity();
    cells.resize(sequenceLength, initialCell);
    std::copy_n(cells.cbegin(), sequenceLength, searchSequence.cells);
}

RaycastLookupTable::Query RaycastLookupTable::searchSequenceToLookupQuery(const SearchSequence &searchSequence, const double offs[7]) {
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

    double dpx = (dsx + offs[3]) / (0.5 * dirSplits / M_PI) - M_PI;
    double dpy = (dsy + offs[4]) / (0.5 * dirSplits) - 1.0;
    double xyScale = dpy > -1.0 && dpy < 1.0 ? std::sqrt(1.0 - dpy * dpy) : 0.0;
    res.dir.x = std::cos(dpx) * xyScale;
    res.dir.y = std::sin(dpx) * xyScale;
    res.dir.z = dpy;

    // This should be pretty normalized, but normalize it again just to make sure.
    // getRayPointDistanceSq depends on it being normalized.
    double dirLen = glm::length(res.dir);
    assert(dirLen > 0.999 && dirLen < 1.001);
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
        assert(&getSequence(res.origin, res.dir, res.parallelDist, res.perpDistSq) == &searchSequence);
    }
#endif

    return res;
}

void RaycastLookupTable::setMinSurfaceDistance(SearchSequence::Cell &cell, const SearchSequence &searchSequence) {
    NloptUserData userData;
    userData.raycastLookupTable = this;
    userData.searchSequence = &searchSequence;

    nlopt_opt opt = nlopt_create(NLOPT_LD_MMA, nloptArgCount);
    nlopt_set_min_objective(opt, nloptFunc, &userData);

    double lb[nloptArgCount];
    std::fill_n(lb, nloptArgCount, 0.0);
    lb[nloptArgCount - 3] = cell.offsetX;
    lb[nloptArgCount - 2] = cell.offsetY;
    lb[nloptArgCount - 1] = cell.offsetZ;
    nlopt_set_lower_bounds(opt, lb);

    double ub[nloptArgCount];
    std::fill_n(ub, nloptArgCount, 1.0);
    ub[nloptArgCount - 3] = cell.offsetX + 1;
    ub[nloptArgCount - 2] = cell.offsetY + 1;
    ub[nloptArgCount - 1] = cell.offsetZ + 1;
    nlopt_set_upper_bounds(opt, ub);

    nlopt_set_stopval(opt, 0.0);
    nlopt_set_xtol_abs1(opt, 1e-9);
    nlopt_set_maxeval(opt, 1024);
//    nlopt_set_maxtime(opt, 1.0);

    double x[nloptArgCount];
    for (unsigned int i = 0; i < nloptArgCount; i++) {
        x[i] = (lb[i] + ub[i]) * 0.5;
    }

    double minVal;
    nlopt_result res = nlopt_optimize(opt, x, &minVal);
    if (res < 0) {
        throw BuildException("Nlopt error! Code is " + std::to_string(res));
    }

    cell.minSurfaceDistance = getSurfaceDistForParams(x, &userData);

    nlopt_destroy(opt);
}

double RaycastLookupTable::nloptFunc(unsigned int n, const double *x, double *grad, void *userData) {
    assert(n == nloptArgCount);

    double sd = getSurfaceDistForParams(x, static_cast<NloptUserData *>(userData));

    // We don't want to go below zero, so square it.
    double loss = sd * sd;

    if (grad) {
        static constexpr double eps = 1e-6;

        double xm[nloptArgCount];
        std::copy_n(x, nloptArgCount, xm);

        for (unsigned int i = 0; i < nloptArgCount; i++) {
            xm[i] += eps;
            grad[i] = (nloptFunc(n, xm, 0, userData) - loss) / eps;
            xm[i] = x[i];
        }
    }

    return loss;
};

double RaycastLookupTable::getSurfaceDistForParams(const double *x, NloptUserData *userData) {
    assert(nloptArgCount == 10);
    Query query = userData->raycastLookupTable->searchSequenceToLookupQuery(*userData->searchSequence, x);
    return userData->raycastLookupTable->getSurfaceDistanceAlongRay<double>(query.origin, query.dir, query.parallelDist, query.perpDistSq, glm::dvec3(x[7], x[8], x[9]));
}

}
