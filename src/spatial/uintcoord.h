#pragma once

#include <cstdint>
#include <cmath>

#include "graphics/glm.h"
#include <glm/vec3.hpp>

namespace spatial {

class UintCoord : public glm::tvec3<std::uint32_t> {
public:
    typedef std::uint32_t AxisType;
    typedef std::int32_t SignedAxisType;

    static constexpr unsigned int maxSizeLog2 = std::numeric_limits<AxisType>::digits - 1;
    // This can't be the width of AxisType because bit shifting by equal to or greater than the bit width is undefined
    static_assert(maxSizeLog2 == 15 || maxSizeLog2 == 31 || maxSizeLog2 == 63, "Unexpected maxSizeLog2");

private:
    static constexpr AxisType center = (0x5555555555555555ull & ((1ull << (maxSizeLog2 - 1)) - 1)) | (1ull << (maxSizeLog2 - 2));

public:
    static AxisType floatToUint(float x) {
        return static_cast<SignedAxisType>(std::floor(x)) + center;
    }
    static float uintToFloat(AxisType x) {
        return static_cast<SignedAxisType>(x - center);
    }

    static UintCoord fromPoint(glm::vec3 point) {
        return UintCoord(floatToUint(point.x), floatToUint(point.y), floatToUint(point.z));
    }
    glm::vec3 toPoint() const {
        return glm::vec3(uintToFloat(x), uintToFloat(y), uintToFloat(z));
    }

    struct Hasher {
        std::size_t operator()(const UintCoord &coord) const {
            // This hash function should only be used right before a prime modulus
            // If right before a power of two modulus, need to do proper hashing here
            std::size_t res = 0;
            res ^= static_cast<std::size_t>(coord.x) << 0;
            res ^= static_cast<std::size_t>(coord.y) << 16;
            res ^= static_cast<std::size_t>(coord.z) << 32;
            return res;
        }
    };

    using glm::tvec3<AxisType>::tvec3;

    UintCoord operator+(const UintCoord &other) const {
        return UintCoord(x + other.x, y + other.y, z + other.z);
    }
    UintCoord operator-(const UintCoord &other) const {
        return UintCoord(x - other.x, y - other.y, z - other.z);
    }

    static UintCoord middle(const UintCoord &a, const UintCoord &b) {
        return UintCoord((a.x >> 1) + (b.x >> 1), (a.y >> 1) + (b.y >> 1), (a.z >> 1) + (b.z >> 1));
    }
};

}
