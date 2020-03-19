#pragma once

#include <random>

#include "graphics/glm.h"
#include <glm/vec3.hpp>

namespace pointgen {

class RandomGenerator {
public:
    RandomGenerator(glm::vec3 size, unsigned int seed = 6487)
        : size(size)
        , rngGen(seed)
    {
        advance();
    }

    glm::vec3 get() {
        assert(res.x >= 0.0f);
        assert(res.y >= 0.0f);
        assert(res.z >= 0.0f);
        assert(res.x < size.x);
        assert(res.y < size.y);
        assert(res.z < size.z);

        return res;
    }

    void advance() {
        float x = std::uniform_real_distribution<float>(0.0f, size.x)(rngGen);
        float y = std::uniform_real_distribution<float>(0.0f, size.y)(rngGen);
        float z = std::uniform_real_distribution<float>(0.0f, size.z)(rngGen);

        res = glm::vec3(x, y, z);
    }

private:
    glm::vec3 size;
    std::mt19937 rngGen;
    glm::vec3 res;
};

}
