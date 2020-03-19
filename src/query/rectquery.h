#pragma once

#include "graphics/glm.h"
#include <glm/vec3.hpp>

#include "geometry/sphere.h"
#include "spatial/cellkey.h"

namespace query {

class RectQuery {
public:
    RectQuery(glm::vec3 min, glm::vec3 max)
        : min(min)
        , max(max)
    {}

    bool testPoint(glm::vec3 point) const {
        return true
                && min.x < point.x && point.x < max.x
                && min.y < point.y && point.y < max.y
                && min.z < point.z && point.z < max.z;
    }

    bool testSphere(geometry::Sphere sphere) const {
        glm::vec3 point = sphere.center;
        for (unsigned int i = 0; i < 3; i++) {
            if (point[i] < min[i] - sphere.rad) {return false;}
            else if (point[i] > max[i] + sphere.rad) {return false;}
            else if (point[i] < min[i]) {point[i] = min[i];}
            else if (point[i] > max[i]) {point[i] = max[i];}
        }
        return glm::distance2(point, sphere.center) <= sphere.rad * sphere.rad;
    }

    bool testCellKey(spatial::CellKey cellKey) {
        glm::vec3 cellMin = cellKey.getCoord<0, 0, 0>().toPoint();
        glm::vec3 cellMax = cellKey.getCoord<1, 1, 1>().toPoint();
        return true
                && min.x < cellMax.x && max.x > cellMin.x
                && min.y < cellMax.y && max.y > cellMin.y
                && min.z < cellMax.z && max.z > cellMin.z;
    }

private:
    glm::vec3 min;
    glm::vec3 max;
};

class RectCoordQuery {
public:
    RectCoordQuery(spatial::UintCoord min, spatial::UintCoord max)
        : min(min)
        , max(max)
    {}

    bool testPoint(spatial::UintCoord point) const {
        return true
                && min.x < point.x && point.x < max.x
                && min.y < point.y && point.y < max.y
                && min.z < point.z && point.z < max.z;
    }

    bool testCellKey(spatial::CellKey cellKey) {
        spatial::UintCoord cellMin = cellKey.getCoord<0, 0, 0>();
        spatial::UintCoord cellMax = cellKey.getCoord<1, 1, 1>();
        return true
                && min.x < cellMax.x && max.x > cellMin.x
                && min.y < cellMax.y && max.y > cellMin.y
                && min.z < cellMax.z && max.z > cellMin.z;
    }

private:
    spatial::UintCoord min;
    spatial::UintCoord max;
};

}
