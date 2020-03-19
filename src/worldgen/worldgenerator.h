#pragma once

#include "spatial/cellkey.h"
#include "pointgen/chunk.h"
#include "world/chunk.h"

namespace worldgen {

class WorldGenerator {
public:
    virtual ~WorldGenerator() {}

    class Request {
    public:
        virtual ~Request() {}

        virtual spatial::CellKey getCube() const = 0;
        virtual const pointgen::Chunk *getPoints() const = 0;
        virtual world::Chunk *getDstChunk() const = 0;

        virtual void onComplete(world::SpaceState chunkState) = 0;
    };

    virtual void generate(Request *request) = 0;
};

}
