#pragma once

#include <assert.h>

namespace graphics {

template <unsigned int size>
class CircularIndex {
public:
    CircularIndex(unsigned int value)
        : value(value)
    {
        assert(value < size);
    }

    unsigned int get() const {
        return value;
    }

    CircularIndex next() {
        return CircularIndex((value + 1) % size);
    }
    CircularIndex prev() {
        return CircularIndex((value + (size - 1)) % size);
    }

    CircularIndex oppositeCeil() {
        return CircularIndex((value + (size / 2 + 1)) % size);
    }
    CircularIndex oppositeFloor() {
        return CircularIndex((value + (size / 2)) % size);
    }

private:
    unsigned int value;
};

}
