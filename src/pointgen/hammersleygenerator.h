#pragma once

#include "pointgen/jitterer.h"

namespace pointgen {

class HammersleyGenerator3d : private Jitterer {
public:
    HammersleyGenerator3d(glm::vec3 size, unsigned int numPoints, float jitterFactor = 0.38f, unsigned int seed = 6487)
        : Jitterer(size, numPoints, jitterFactor, seed)
        , factorX(size.x / numPoints)
        , sizeY(size.y)
        , sizeZ(size.z)
        , index(1)
    {}

    glm::vec3 get() {
        return glm::vec3(index * factorX, reverseDigits<2>(index, sizeY), reverseDigits<3>(index, sizeZ)) + getJitter();
    }

    void advance() {
        index++;
    }

    void advanceSteps(unsigned int steps) {
        index += steps;
    }

private:
    float factorX;
    float sizeY;
    float sizeZ;
    unsigned int index;

    template <unsigned int base>
    float reverseDigits(unsigned int index, float factor = 1.0f) {
        float res = 0.0f;

        while (index > 0) {
            factor /= base;
            res += factor * (index % base);
            index /= base;
        }

        return res;
    }
};

}
