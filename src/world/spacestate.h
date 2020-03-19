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
    };

    SpaceState() {}

    SpaceState(Value value)
        : value(value)
    {}

    bool isTransparent() const {
        return value < 64;
    }

    bool operator==(const Value other) const {
        return value == other;
    }
    bool operator!=(const Value other) const {
        return value != other;
    }

    Value value;
};

}
