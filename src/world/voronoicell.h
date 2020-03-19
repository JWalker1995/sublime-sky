#pragma once

#include <cstdint>

#include "graphics/glm.h"
#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>

#include "world/spacestate.h"
#include "world/particle.h"

namespace world {

class VoronoiCell {
public:
    SpaceState type = SpaceState::Uninitialized;

    float getMass() const {
        switch (type.value) {
            case SpaceState::Uninitialized: return 0.0f;
            case SpaceState::Generating: return 0.0f;
            case SpaceState::Air: return 0.01f;
            default: return 1.0f;
        }
    }

//    glm::vec3 center;
//    glm::vec3 velocity;

    // dynamic pressure == kinetic energy == 0.5 * mass * glm::length2(velocity)

    void tick(float dt) {
//        Cell *dst = getCellTowards(dynamicPressure);
//        dst->velocity += velocity;


        /*
        static constexpr float coef_restitution = 0.5f;

        Cell *dst = getCellTowards(velocityCellsPerSecond);

        glm::vec3 dx = center - dst->center;
        glm::vec3 dv = velocityCellsPerSecond - dst->velocityCellsPerSecond;
        float massSum = getMass() + dst->getMass();
        glm::vec3 impulse = (glm::dot(dx, dv) / (glm::length2(dx) * massSum) * ((1.0f + coef_restitution) / 2.0f)) * dx;
        velocityCellsPerSecond -= impulse / dst->getMass();
        dst->velocityCellsPerSecond += impulse / getMass();
        */
    }
};

}
