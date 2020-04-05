#include "raydrawer.h"

#include <random>

#include "util/testrunner.h"

namespace world {

void testRay(std::size_t seed, unsigned int steps) {
    std::default_random_engine gen(seed);
    std::uniform_int_distribution<unsigned int> uniform(0, 15);
    std::normal_distribution<float> normal(0.0f, 256.0f);

    glm::vec3 origin(normal(gen), normal(gen), normal(gen));
    glm::vec3 dir(normal(gen), normal(gen), normal(gen));

    spatial::CellKey cellKey = spatial::CellKey::fromCoord(spatial::UintCoord::fromPoint(origin), 4);
    RayDrawer rayDrawer(origin, dir, cellKey);

    dir /= glm::length(dir) * 64.0f;

    for (unsigned int i = 0; i < steps; i++) {
        spatial::CellKey prevCell = rayDrawer.getCurCellKey();

        unsigned int cmd = uniform(gen);
        if (cmd == 0 && rayDrawer.getCurCellKey().sizeLog2 > 0) {
            rayDrawer.enterChildCell();
        } else if (cmd == 1 && rayDrawer.getCurCellKey().sizeLog2 < spatial::UintCoord::maxSizeLog2) {
            rayDrawer.enterParentCell();
        } else {
            rayDrawer.step();
        }

        float distSq = std::numeric_limits<float>::infinity();
        do {
            // If this fails, that means we are not close to the prev cell or the next cell.
            // The ray drawer must have skipped a cell or two
            float prevCellDistSq = glm::distance2(origin, prevCell.constrainPointInside(origin));
            assert(prevCellDistSq < 1e-6);

            origin += dir;
            float nextDistSq = glm::distance2(origin, rayDrawer.getCurCellKey().constrainPointInside(origin));

            // If this fails, that means we're moving away from the cell without ever being close (or inside) it.
            assert(nextDistSq < distSq);

            distSq = nextDistSq;
        } while (distSq >= 1e-6);
    }
}

void test(game::GameContext &) {
    for (std::size_t seed = 1234; seed < 2345; seed++) {
        testRay(seed, 1000);
    }
}

static int _ = util::TestRunner::getInstance().registerGameTest(&test);

}
