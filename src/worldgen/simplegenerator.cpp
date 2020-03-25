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

    glm::vec3 particlePosition(0.0f, 0.0f, 100.0f);
    if (request->getCube().contains(spatial::UintCoord::fromPoint(particlePosition))) {
        particle::Particle &p = context.get<particle::ParticleManager>().createParticle();
        p.position = particlePosition;
        p.velocity = glm::vec3(0.1f, 0.0f, 0.0f);
        p.mass = 1.0f;
        p.energy = 0.0f;
    }

    allSame = false;
    request->onComplete(allSame ? allState : world::SpaceState::SubdividedAsChunk);
}

world::SpaceState SimpleGenerator::getState(glm::vec3 point) {
    if (std::isnan(point.x)) {
        return world::SpaceState::Uninitialized;
    }

//    glm::vec2 treeCenter(point.x, point.y);
//    float v = 10.0f;
//    while (true) {
//        float newV = noise.octave_noise_2d(2, 0.5f, 0.1f, treeCenter.x, treeCenter.y);
//        if (std::fabs(newV - v) < 1e-6) {
//            break;
//        }
//        v = newV;
//        float dx = (noise.octave_noise_2d(2, 0.5f, 0.1f, treeCenter.x + 1e-3, treeCenter.y) - v) / 1e-3;
//        float dy = (noise.octave_noise_2d(2, 0.5f, 0.1f, treeCenter.x, treeCenter.y + 1e-3) - v) / 1e-3;

//        float scale = (1.0f + (rand() % 256) / 256.0f) * 1.0f;
//        treeCenter.x += dx * scale;
//        treeCenter.y += dy * scale;
//    }

    static constexpr float scaleXY = 1.0f;
    static constexpr float scaleZ = 10.0f;
    float z = noise.octave_noise_2d(8, 0.5f, 0.001f, point.x / scaleXY, point.y / scaleXY) * scaleZ;

    if (point.z >= 20.0f && point.z < 22.0f) {
        if (noise.octave_noise_2d(2, 0.5f, 0.1f, point.x, point.y) > 0.6f) {
            if (rand() % 64 == 0) {
                particle::Particle &p = context.get<particle::ParticleManager>().createParticle();
                p.position = point - glm::vec3(0.0f, 0.0f, 4.0f);
                p.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
                p.mass = 1.0f;
                p.energy = 0.0f;

                context.get<particle::ParticleManager>().addBond(p, point);
            }

            return world::SpaceState::TreeTrunk;
        }
    }

//    if (glm::distance2(treeCenter, glm::vec2(point.x, point.y)) < 1.0f) {
//        z += 4.0f;
//    }

    return point.z < z ? static_cast<world::SpaceState::Value>(100) : world::SpaceState::Air;
}

}
