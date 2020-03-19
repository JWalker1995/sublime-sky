#pragma once

#include "graphics/glm.h"
#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>

namespace geometry {

class Sphere {
public:
    Sphere() {}

    Sphere(glm::vec3 center, float rad)
        : center(center)
        , rad(rad)
    {}

    glm::vec3 center;
    float rad;

    float calcRadDeltaToContain(const Sphere &outsider) {
        return glm::distance(center, outsider.center) + outsider.rad - rad;
    }

    bool contains(glm::vec3 point) {
        return glm::distance2(center, point) < rad * rad;
    }

    static bool intersects(const Sphere &a, const Sphere &b) {
        float radSum = a.rad + b.rad;
        return glm::distance2(a.center, b.center) < radSum * radSum;
    }

    static Sphere erode(const Sphere &a, const Sphere &b) {
        Sphere res;
        res.center = b.center - a.center;
        res.rad = a.rad + b.rad;
        return res;
    }

    static Sphere enclose(const Sphere &a, const Sphere &b) {
        glm::vec3 dNorm = b.center - a.center;
        float dLen = glm::length(dNorm);
        if (dLen > 1e-6f) {
            dNorm /= dLen;
        } else {
            dNorm = glm::vec3(1.0f, 0.0f, 0.0f);
        }

        if (b.rad + dLen < a.rad) {
            // If b is completely contained by a
            return a;
        } else if (a.rad + dLen < b.rad) {
            // If a is completely contained by b
            return b;
        } else {
            glm::vec3 min = a.center - dNorm * a.rad;
            glm::vec3 max = b.center + dNorm * b.rad;

            Sphere res;
            res.center = (max + min) / 2.0f;
            res.rad = glm::length(max - min) / 2.0f;
            return res;
        }
    }
};

}
