#pragma once

#include "world/voronoicell.h"

namespace world {

class Chunk {
public:
    static constexpr unsigned int sizeLog2 = 4;
    static constexpr unsigned int size = static_cast<unsigned int>(1) << sizeLog2;

    VoronoiCell cells[size][size][size];
};

}
