#pragma once

#include <algorithm>

#include "graphics/glm.h"
#include <glm/vec3.hpp>

#include "geometry/sphere.h"
#include "spatial/cellkey.h"

namespace query {

class RayQuery {
public:
    RayQuery(glm::vec3 origin, glm::vec3 dir)
        : origin(origin)
        , dir(dir)
        , a(glm::length2(dir))
    {}

    bool testSphere(geometry::Sphere sphere) {
        /*
        length2(a + b*t) = d * d
        (a.x + b.x * t)^2 + (a.y + b.y * t)^2 + (a.z + b.z * t)^2 = d^2
        (a.x + b.x * t)^2 + (a.y + b.y * t)^2 + (a.z + b.z * t)^2 = d^2
        a.x^2 + 2 * a.x * b.x * t + b.x^2 * t^2  +  a.y^2 + 2 * a.y * b.y * t + b.y^2 * t^2  +  a.z^2 + 2 * a.z * b.z * t + b.z^2 * t^2 = d^2

        a.x^2 + a.y^2 + a.z^2
        + 2 * a.x * b.x * t + 2 * a.y * b.y * t + 2 * a.z * b.z * t
        + b.x^2 * t^2 + b.y^2 * t^2 + b.z^2 * t^2
        = d^2

        a.x^2 + a.y^2 + a.z^2 - d * d // c
        + (2 * a.x * b.x + 2 * a.y * b.y + 2 * a.z * b.z) * t // b
        + (b.x^2 + b.y^2 + b.z^2) * t^2 // a
        = 0

        a = glm::distance2(dir);
        b = 2.0f * glm::dot(origin - sphere.center, dir);
        c = glm::distance2(origin - sphere.center) - d * d;

        t = (-b +- sqrt(b^2 - 4*a*c)) / (2a)
        t = (-2.0f * glm::dot(origin - sphere.center, dir) +- sqrt(b^2 - 4.0f * a * c)) / (2.0f * a)
        */

        sphere.center -= origin;
        float b = glm::dot(sphere.center, dir);
        float c = glm::length2(sphere.center) - sphere.rad * sphere.rad;

        float sqrtInner = b * b - a * c;
        if (sqrtInner < 0.0f) {
            return false;
        }

        d = glm::dot(sphere.center, dir);
        e = std::sqrt(sqrtInner);

        if (d + e < 0.0f) {
            // Ray exits before the ray begins
            return false;
        }

        return true;
    }

    bool testCellKey(spatial::CellKey cellKey) {
        glm::vec3 cellMin = cellKey.getCoord<0, 0, 0>().toPoint() - origin;
        glm::vec3 cellMax = cellKey.getCoord<1, 1, 1>().toPoint() - origin;

        float minExitDist = std::min({
            (dir.x >= 0.0f ? cellMax.x : cellMin.x) / dir.x,
            (dir.y >= 0.0f ? cellMax.y : cellMin.y) / dir.y,
            (dir.z >= 0.0f ? cellMax.z : cellMin.z) / dir.z,
        });

        if (minExitDist < 0.0f) {
            return false;
        }

        float maxEnterDist = std::max({
            (dir.x >= 0.0f ? cellMin.x : cellMax.x) / dir.x,
            (dir.y >= 0.0f ? cellMin.y : cellMax.y) / dir.y,
            (dir.z >= 0.0f ? cellMin.z : cellMax.z) / dir.z,
        });

        return maxEnterDist < minExitDist;
    }

    // These do NOT depend on the magnitude of dir
    float getLastSphereEnterDist() const {
        return (d - e) / a;
    }
    float getLastSphereExitDist() const {
        return (d + e) / a;
    }

private:
    glm::vec3 origin;
    glm::vec3 dir;
    float a;

    float d;
    float e;
};

}
