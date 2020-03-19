#pragma once

#include "pointgen/chunk.h"
#include "spatial/cellkey.h"

namespace pointgen {

class PointGenerator {
public:
    virtual ~PointGenerator() {}

    virtual void generate(Chunk *dst, const spatial::CellKey &cellKey) = 0;
};

}
