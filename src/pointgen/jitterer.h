#pragma once

#include "pointgen/randomballvectorgenerator.h"

namespace pointgen {

class Jitterer : private RandomBallVectorGenerator {
public:
    Jitterer(glm::vec3 size, unsigned int numPoints, float jitterFactor = 0.38f, unsigned int seed = 6487)
        : RandomBallVectorGenerator(std::cbrt(size.x * size.y * size.z / numPoints) * jitterFactor, seed)
    {}

    glm::vec3 getJitter() {
        return generate();
    }
};

}
