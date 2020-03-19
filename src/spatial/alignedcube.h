#pragma once

#include "graphics/glm.h"
#include <glm/detail/type_vec3.hpp>

namespace spatial {

// TODO: Rename to Cube
class AlignedCube {
public:
    typedef std::int64_t AxisType;

    AlignedCube() {}

    AlignedCube(glm::tvec3<AxisType> center, AxisType radius)
        : center(center)
        , radius(radius)
    {
        assert(isValid());
    }

    glm::tvec3<AxisType> center;
    AxisType radius;

    bool isValid() {
        bool isPositiveRadius = radius > 0;
        bool isRadiusPowerOf2 = (radius & (radius - 1)) == 0;
        bool isCenterXAligned = (center.x & (radius - 1)) == 0;
        bool isCenterYAligned = (center.y & (radius - 1)) == 0;
        bool isCenterZAligned = (center.z & (radius - 1)) == 0;
        return isPositiveRadius && isRadiusPowerOf2 && isCenterXAligned && isCenterYAligned && isCenterZAligned;
    }

    AlignedCube getParentCubeByOctant(unsigned int octant) const {
        AlignedCube res;
        res.radius = radius * 2;
        res.center.x = getGteX(octant) ? (center.x + radius) : (center.x - radius);
        res.center.y = getGteY(octant) ? (center.y + radius) : (center.y - radius);
        res.center.z = getGteZ(octant) ? (center.z + radius) : (center.z - radius);
        return res;
    }

    AlignedCube getChildCubeByOctant(unsigned int octant) const {
        AlignedCube res;
        res.radius = radius / 2;
        assert(res.radius > 0);
        res.center.x = getGteX(octant) ? (center.x + res.radius) : (center.x - res.radius);
        res.center.y = getGteY(octant) ? (center.y + res.radius) : (center.y - res.radius);
        res.center.z = getGteZ(octant) ? (center.z + res.radius) : (center.z - res.radius);
        return res;
    }

    unsigned int getOctantContaining(glm::tvec3<AxisType> coord) const {
        bool gteX = coord.x >= center.x;
        bool gteY = coord.y >= center.y;
        bool gteZ = coord.z >= center.z;
        return getOctant(gteX, gteY, gteZ);
    }

    static constexpr unsigned int getOctant(bool gteX, bool gteY, bool gteZ) {
        return (gteX << 2) | (gteY << 1) | (gteZ << 0);
    }

    static constexpr bool getGteX(unsigned int octant) {
        return octant & 0x4;
    }
    static constexpr bool getGteY(unsigned int octant) {
        return octant & 0x2;
    }
    static constexpr bool getGteZ(unsigned int octant) {
        return octant & 0x1;
    }
};

}
