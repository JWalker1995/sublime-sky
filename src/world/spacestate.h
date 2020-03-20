#pragma once

#include <cstdint>

namespace world {

class SpaceState {
public:
    enum Value : std::uint8_t {
        Uninitialized, // TODO: Remove; might never be used?
        Empty, // No point in this cell; closest point must come from another cell
        Air,

        Generating = 64,
        SubdividedAsBranch,
        SubdividedAsChunk,

        Dirt,
        Rock,
        Water,
        TreeTrunk,
        TreeLeaf,
        Cloud,
    };

    SpaceState() {}

    SpaceState(Value value)
        : value(value)
    {}

    bool isTransparent() const {
        return value < 64;
    }

    bool operator==(const SpaceState other) const {
        return value == other.value;
    }
    bool operator!=(const SpaceState other) const {
        return value != other.value;
    }

    Value value;
};

}
