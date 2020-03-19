#pragma once

#include <random>

#include "graphics/glm.h"
#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>

namespace pointgen {

class RandomBallVectorGenerator {
public:
    RandomBallVectorGenerator(float rad, unsigned int seed = 6487)
        : rngGen(seed)
        , rngDist(-rad, rad)
    {}

    glm::vec3 generate() {
        glm::vec3 offset;
        do {
            offset.x = rngDist(rngGen);
            offset.y = rngDist(rngGen);
            offset.z = rngDist(rngGen);
        } while (glm::length2(offset) > rngDist.max());
        return offset;
    }

private:
    std::mt19937 rngGen;
    std::uniform_real_distribution<float> rngDist;
};

}
