#include "simplegenerator.h"

namespace worldgen {

SimpleGenerator::SimpleGenerator(game::GameContext &context)
    : context(context)
{}

void SimpleGenerator::generate(Request *request) {
    const pointgen::Chunk *points = request->getPoints();
    world::Chunk *dstChunk = request->getDstChunk();
    assert(pointgen::Chunk::size == world::Chunk::size);

    bool allSame = true;
    world::SpaceState allState = getState(points->points[0][0][0]);

    for (unsigned int i = 0; i < world::Chunk::size; i++) {
        for (unsigned int j = 0; j < world::Chunk::size; j++) {
            for (unsigned int k = 0; k < world::Chunk::size; k++) {
                world::SpaceState state = getState(points->points[i][j][k]);
                dstChunk->cells[i][j][k].type = state;

                allSame &= state == allState;
            }
        }
    }

    request->onComplete(allSame ? allState : world::SpaceState::SubdividedAsChunk);
}

world::SpaceState SimpleGenerator::getState(glm::vec3 point) {
    return point.z < point.y / 4.0f + point.x / 7.0f ? static_cast<world::SpaceState::Value>(100) : world::SpaceState::Air;
}

}
