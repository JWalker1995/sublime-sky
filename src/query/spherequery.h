#pragma once

#include "graphics/glm.h"
#include <glm/vec3.hpp>

#include "geometry/sphere.h"

#include "spatial/cellkey.h"

namespace query {

class SphereQuery {
public:
    SphereQuery(geometry::Sphere sphere)
        : sphere(sphere)
    {}

    bool testPoint(glm::vec3 point) const {
        float distSq = glm::distance2(sphere.center, point);
        return distSq < sphere.rad * sphere.rad;
    }

    bool testSphere(geometry::Sphere other) const {
        float distSq = glm::distance2(sphere.center, other.center);
        float radSum = sphere.rad + other.rad;
        return distSq < radSum * radSum;
    }

    bool testCellKey(spatial::CellKey cellKey) {
        return glm::distance2(sphere.center, cellKey.constrainPointInside(sphere.center)) < sphere.rad * sphere.rad;
    }

private:
    geometry::Sphere sphere;
};

}
