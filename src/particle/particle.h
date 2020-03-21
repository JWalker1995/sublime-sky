#pragma once

#include "graphics/glm.h"
#include <glm/vec3.hpp>

namespace world { class SpaceState; }

namespace particle {

class Particle {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    float mass;
    float energy;
    world::SpaceState *cell = 0;

    void tick();

//    virtual void onRecvMessage() {}
//    virtual void onRecvParticle() {}

    void scheduleTick(float time);
};

}
