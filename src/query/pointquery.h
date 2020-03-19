#pragma once

#include "graphics/glm.h"
#include <glm/vec3.hpp>

#include "geometry/sphere.h"

namespace query {

class PointQuery {
public:
    PointQuery(glm::vec3 point)
        : point(point)
    {}

    bool testSphere(geometry::Sphere sphere) const {
        return glm::distance2(point, sphere.center) < sphere.rad * sphere.rad;
    }

private:
    glm::vec3 point;
};

}
