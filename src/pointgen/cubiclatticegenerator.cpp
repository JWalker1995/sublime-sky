#include "cubiclatticegenerator.h"

namespace pointgen {

CubicLatticeGenerator::CubicLatticeGenerator(game::GameContext &context) {
    (void) context;
}

void CubicLatticeGenerator::generate(Chunk *dst, const spatial::CellKey &cellKey) {
    for (unsigned int x = 0; x < Chunk::size; x++) {
        for (unsigned int y = 0; y < Chunk::size; y++) {
            for (unsigned int z = 0; z < Chunk::size; z++) {
                dst->points[x][y][z] = cellKey.grandChild<Chunk::sizeLog2>(x, y, z).getPoint(glm::vec3(0.5f));
            }
        }
    }
}

}
