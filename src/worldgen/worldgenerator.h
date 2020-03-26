#pragma once

#include "spatial/cellkey.h"
#include "pointgen/chunk.h"
#include "world/chunk.h"

namespace worldgen {

class WorldGenerator {
public:
    virtual ~WorldGenerator() {}

    virtual void generate(spatial::CellKey cube, const pointgen::Chunk *points) = 0;
};

}
