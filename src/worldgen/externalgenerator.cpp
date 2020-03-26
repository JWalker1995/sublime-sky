#include "externalgenerator.h"

#include "network/connectionmanager.h"

namespace worldgen {

ExternalGenerator::ExternalGenerator(game::GameContext &context)
    : context(context)
{}

void ExternalGenerator::generate(Request *request) {
    context.get<network::ConnectionManager>().send();

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

}
