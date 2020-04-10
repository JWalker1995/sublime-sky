#include "cubiclatticegenerator.h"

#include <random>

namespace pointgen {

CubicLatticeGenerator::CubicLatticeGenerator(game::GameContext &context, const SsProtocol::Config::CubicLatticeGenerator *config) {
    (void) context;
    (void) config;
}

void CubicLatticeGenerator::generate(Chunk *dst, const spatial::CellKey &cellKey) {
    std::default_random_engine rng(spatial::CellKeyHasher()(cellKey));
    std::normal_distribution<float> dist(0.5f, 0.0f);

    for (unsigned int x = 0; x < Chunk::size; x++) {
        for (unsigned int y = 0; y < Chunk::size; y++) {
            for (unsigned int z = 0; z < Chunk::size; z++) {
                glm::vec3 pt(dist(rng), dist(rng), dist(rng));
                if (pt.x < 0.0f) { pt.x = 0.0f; }
                else if (pt.x > 1.0f) { pt.x = 1.0f; }
                if (pt.y < 0.0f) { pt.y = 0.0f; }
                else if (pt.y > 1.0f) { pt.y = 1.0f; }
                if (pt.z < 0.0f) { pt.z = 0.0f; }
                else if (pt.z > 1.0f) { pt.z = 1.0f; }

                dst->points[x][y][z] = cellKey.grandChild<Chunk::sizeLog2>(x, y, z).getPoint(pt);
            }
        }
    }
}

}
