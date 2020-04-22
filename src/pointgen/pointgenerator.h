#pragma once

#include "pointgen/chunk.h"
#include "spatial/cellkey.h"

namespace pointgen {

class PointGenerator {
public:
    PointGenerator(bool isCubical)
        : isCubical(isCubical)
    {}

    virtual ~PointGenerator() {}

    virtual void generate(Chunk *dst, const spatial::CellKey &cellKey) = 0;

    const bool isCubical;
};

}
