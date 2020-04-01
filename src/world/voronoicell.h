#pragma once

#include <cstdint>

#include "graphics/glm.h"
#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>

namespace world {

class VoronoiCell {
public:
    typedef std::uint16_t MaterialIndex;

    MaterialIndex materialIndex = 0;

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
