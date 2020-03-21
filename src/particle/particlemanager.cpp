#include "particlemanager.h"

#include "world/hashtreeworld.h"

namespace particle {

ParticleManager::ParticleManager(game::GameContext &context)
    : TickableBase(context)
{}

void ParticleManager::addParticle(const Particle &particle) {
    particles.push_back(particle);
}

void ParticleManager::tick(game::TickerContext &tickerContext) {
    world::HashTreeWorld &hashTreeWorld = context.get<world::HashTreeWorld>();

    std::vector<Particle>::iterator i = particles.begin();
    while (i != particles.end()) {
        i->velocity.z -= 0.01f;
        i->velocity *= 0.999f;

        float lenSq = glm::length2(i->velocity);
        if (lenSq > 1.0f) {
            i->velocity /= std::sqrt(lenSq);
        }

        i->position += i->velocity;

        world::SpaceState &newCell = hashTreeWorld.getClosestPointState(i->position);
        if (&newCell != i->cell) {
            if (i->cell) {
                newCell = *i->cell;
                *i->cell = world::SpaceState::Air;
            } else {
                newCell = world::SpaceState::Rock;
            }
            i->cell = &newCell;

            hashTreeWorld.emitMeshUpdate(i->position, i->position, 2.0f);
        }

        i++;
    }
}

}
