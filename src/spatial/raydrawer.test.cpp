#include "raydrawer.h"

#include <random>

#include "graphics/glm.h"
#include <glm/gtx/norm.hpp>

#include "util/testrunner.h"

namespace spatial {

void testRay(std::size_t seed, unsigned int cellSteps) {
    std::default_random_engine gen(seed);
    std::uniform_int_distribution<unsigned int> uniform(0, 15);
    std::normal_distribution<double> normal(0.0, 256.0);

    glm::dvec3 origin(normal(gen), normal(gen), normal(gen));
    glm::dvec3 dir(normal(gen), normal(gen), normal(gen));

    CellKey cellKey = CellKey::fromCoord(UintCoord::fromPoint(origin), 4);
    RayDrawer<glm::dvec3> rayDrawer(origin, dir, cellKey);

    dir /= glm::length(dir) * 64.0;

    glm::vec3 pos = origin;
    unsigned int raySteps = 0;
    for (unsigned int i = 0; i < cellSteps; i++) {
        CellKey prevCell = rayDrawer.getCurCellKey();
        glm::vec3 prevMin = prevCell.getCoord<0, 0, 0>().toPoint();
        glm::vec3 prevMax = prevCell.getCoord<1, 1, 1>().toPoint();

        unsigned int cmd = uniform(gen);
        if (cmd == 0 && rayDrawer.getCurCellKey().sizeLog2 > 0) {
            rayDrawer.enterChildCell();
        } else if (cmd == 1 && rayDrawer.getCurCellKey().sizeLog2 < 8) {
            rayDrawer.enterParentCell();
        } else {
            rayDrawer.step();
        }

        glm::vec3 newMin = rayDrawer.getCurCellKey().getCoord<0, 0, 0>().toPoint();
        glm::vec3 newMax = rayDrawer.getCurCellKey().getCoord<1, 1, 1>().toPoint();

        float distSq = std::numeric_limits<float>::infinity();
        do {
            // If this fails, that means we are not close to the prev cell or the next cell.
            // The ray drawer must have skipped a cell or two
            float prevCellDistSq = glm::distance2(pos, prevCell.constrainPointInside(pos));
            assert(prevCellDistSq < 1e-3);

            pos = origin + dir * static_cast<double>(++raySteps);
            float nextDistSq = glm::distance2(pos, rayDrawer.getCurCellKey().constrainPointInside(pos));

            // If this fails, that means we're moving away from the cell without ever being close (or inside) it.
            assert(nextDistSq <= distSq);

            distSq = nextDistSq;
        } while (distSq >= 1e-3);
    }
}

void test(game::GameContext &) {
    testRay(3456, 100);
    testRay(4567, 100);
//    for (std::size_t seed = 1234; seed < 2345; seed++) {
//        testRay(seed, 100);
//    }
}

static int _ = util::TestRunner::getInstance().registerGameTest(&test);

}
