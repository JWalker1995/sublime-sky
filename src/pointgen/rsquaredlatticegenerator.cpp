#include "rsquaredlatticegenerator.h"

#include "pointgen/rsquaredgenerator.h"

#include <random>

namespace pointgen {

RSquaredLatticeGenerator::RSquaredLatticeGenerator(game::GameContext &context, const SsProtocol::Config::RSquaredLatticeGenerator *config)
    : PointGenerator(false)
{
    (void) context;
    (void) config;
}

void RSquaredLatticeGenerator::generate(Chunk *dst, const spatial::CellKey &cellKey) {
    std::size_t seed = spatial::CellKeyHasher()(cellKey);

    std::minstd_rand rng(seed);
    unsigned int numSamples = std::poisson_distribution<unsigned int>(Chunk::size * Chunk::size * Chunk::size * 0.8f)(rng);
    RSquaredGenerator gen(glm::vec3(Chunk::size), numSamples, 0.38f, seed);

    std::fill_n(&dst->points[0][0][0], Chunk::size * Chunk::size * Chunk::size, glm::vec3(NAN));

    for (unsigned int i = 0; i < numSamples; i++) {
        glm::vec3 pt = gen.get();
        gen.advance();

        glm::vec3 &dstPt = dst->points[static_cast<unsigned int>(pt.x)][static_cast<unsigned int>(pt.y)][static_cast<unsigned int>(pt.z)];
        if (std::isnan(dstPt.x)) {
            dstPt = cellKey.grandChild<Chunk::sizeLog2>(0, 0, 0).getPoint(pt);
        } else {
            // TODO: Do something here
//            assert(false);
        }
    }
}

}
