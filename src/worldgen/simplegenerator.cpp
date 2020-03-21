#include "simplegenerator.h"

#include "particle/particlemanager.h"

namespace worldgen {

SimpleGenerator::SimpleGenerator(game::GameContext &context)
    : context(context)
    , noise(123)
{}

void SimpleGenerator::generate(Request *request) {
    const pointgen::Chunk *points = request->getPoints();
    world::Chunk *dstChunk = request->getDstChunk();
    assert(pointgen::Chunk::size == world::Chunk::size);

    bool allSame = true;
    world::SpaceState allState = getState(points->points[0][0][0]);

    for (unsigned int i = 0; i < world::Chunk::size; i++) {
        for (unsigned int j = 0; j < world::Chunk::size; j++) {
            for (unsigned int k = 0; k < world::Chunk::size; k++) {
                world::SpaceState state = getState(points->points[i][j][k]);
                dstChunk->cells[i][j][k].type = state;

                allSame &= state == allState;
            }
        }
    }

    particle::Particle p;
    p.position = glm::vec3(0.0f, 0.0f, 100.0f);
    p.velocity = glm::vec3(0.1f, 0.0f, 0.0f);
    p.mass = 1.0f;
    p.energy = 0.0f;
    if (request->getCube().contains(spatial::UintCoord::fromPoint(p.position))) {
        context.get<particle::ParticleManager>().addParticle(p);
    }

    allSame = false;
    request->onComplete(allSame ? allState : world::SpaceState::SubdividedAsChunk);
}

world::SpaceState SimpleGenerator::getState(glm::vec3 point) {
    static constexpr float scaleXY = 1.0f;
    static constexpr float scaleZ = 10.0f;
    float z = noise.octave_noise_2d(8, 0.5f, 0.001f, point.x / scaleXY, point.y / scaleXY) * scaleZ;
    return point.z < z ? static_cast<world::SpaceState::Value>(100) : world::SpaceState::Air;
}

}
