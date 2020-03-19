#include "simplegenerator.h"

namespace worldgen {

SimpleGenerator::SimpleGenerator(game::GameContext &context)
    : context(context)
{}

void SimpleGenerator::generate(Request *request) {
    const pointgen::Chunk *points = request->getPoints();
    world::Chunk *dstChunk = request->getDstChunk();
    assert(pointgen::Chunk::size == world::Chunk::size);

    for (unsigned int i = 0; i < world::Chunk::size; i++) {
        for (unsigned int j = 0; j < world::Chunk::size; j++) {
            for (unsigned int k = 0; k < world::Chunk::size; k++) {
                glm::vec3 point = points->points[i][j][k];
                world::SpaceState type = point.z < point.y / 4.0f + point.z / 7.0f ? static_cast<world::SpaceState::Value>(100) : world::SpaceState::Air;
                dstChunk->cells[i][j][k].type = type;
            }
        }
    }

    request->onComplete(world::SpaceState::SubdividedAsChunk);
}

}
