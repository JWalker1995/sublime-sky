#include "raydrawer.h"

#include <random>

#include "util/testrunner.h"

namespace world {

void test(game::GameContext &) {
    std::size_t seed = 1234;
    std::default_random_engine gen(seed);
    std::uniform_int_distribution<unsigned int> uniform(0, 15);
    std::normal_distribution<float> normal(0.0f, 256.0f);

    glm::vec3 origin(normal(gen), normal(gen), normal(gen));
    glm::vec3 dir(normal(gen), normal(gen), normal(gen));

    spatial::CellKey cellKey = spatial::CellKey::fromCoord(spatial::UintCoord::fromPoint(origin), 4);
    RayDrawer rayDrawer(origin, dir, cellKey);

    spatial::CellKey prevChunk = spatial::CellKey::fromCoord(rayDrawer.getCurCellKey().cellCoord, 3);
    for (unsigned int i = 0; i < 10000; i++) {
        unsigned int cmd = uniform(gen);
        if (rayDrawer.getCurCellKey().sizeLog2 > 0 && cmd == 0) {
            rayDrawer.enterChildCell();
        } else if (rayDrawer.getCurCellKey().sizeLog2 < spatial::UintCoord::maxSizeLog2 && cmd == 1) {
            rayDrawer.enterParentCell();
        } else {
            bool movedChunks = rayDrawer.step<3>();
            spatial::CellKey newChunk = spatial::CellKey::fromCoord(rayDrawer.getCurCellKey().cellCoord, 3);
            assert(movedChunks == (newChunk != prevChunk));
            prevChunk = newChunk;
        }
    }
}

static int _ = util::TestRunner::getInstance().registerGameTest(&test);

}
