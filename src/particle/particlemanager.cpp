#include "particlemanager.h"

#include "world/hashtreeworld.h"

namespace particle {

ParticleManager::ParticleManager(game::GameContext &context)
    : TickableBase(context)
{}

Particle &ParticleManager::createParticle() {
    particles.emplace_back();
    return particles.back();
}

void ParticleManager::addBond(Particle &particle, glm::vec3 point) {
    bonds.emplace_back(&particle, point);
}

void ParticleManager::tick(game::TickerContext &tickerContext) {
    world::HashTreeWorld &hashTreeWorld = context.get<world::HashTreeWorld>();

    std::vector<std::pair<Particle *, glm::vec3>>::const_iterator i = bonds.cbegin();
    while (i != bonds.cend()) {
        i->first->velocity += (i->second - i->first->position) * 1e-3f;
        i++;
    }

    std::deque<Particle>::iterator j = particles.begin();
    while (j != particles.end()) {
        j->velocity.z -= 0.01f;
        j->velocity *= 0.999f;

        float lenSq = glm::length2(j->velocity);
        if (lenSq > 1.0f) {
            j->velocity /= std::sqrt(lenSq);
        }

        j->position += j->velocity;

        world::SpaceState &newCell = hashTreeWorld.getClosestPointState(j->position);
        if (&newCell != j->cell) {
            if (newCell != world::SpaceState::Air) {
            }
            if (j->cell) {
                newCell = *j->cell;
                *j->cell = world::SpaceState::Air;
            } else {
                newCell = world::SpaceState::Rock;
            }
            j->cell = &newCell;

            glm::vec3 min = j->position;
            glm::vec3 max = min - j->velocity;
            if (min.x > max.x) {std::swap(min.x, max.x);}
            if (min.y > max.y) {std::swap(min.y, max.y);}
            if (min.z > max.z) {std::swap(min.z, max.z);}
            hashTreeWorld.emitMeshUpdate(min, max, 1.0f);
        }

        j++;
    }
}

}
