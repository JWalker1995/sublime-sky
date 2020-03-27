#pragma once

#include <cstdint>

namespace world {

class SpaceState {
public:
    enum Value : std::uint8_t {
        Uninitialized = 0, // TODO: Remove; might never be used?
        Empt = 1, // No point in this cell; closest point must come from another cell
        Air = 2,

        Generating = 64,
        SubdividedAsBranch,
        SubdividedAsChunk,

        Dirt = 100,
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
