#pragma once

#include "graphics/glm.h"
#include <glm/vec3.hpp>

namespace world { class VoronoiCell; }

namespace particle {

class Particle {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    float mass;
    float energy;
    world::VoronoiCell *cell = 0;

    void tick();

//    virtual void onRecvMessage() {}
//    virtual void onRecvParticle() {}

    void scheduleTick(float time);
};

}
