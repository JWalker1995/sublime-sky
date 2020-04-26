#pragma once

#include "spatial/cellkey.h"

namespace worldgen {

class WorldGenerator {
public:
    virtual ~WorldGenerator() {}

    virtual void generate(spatial::CellKey cube) = 0;
};

}
