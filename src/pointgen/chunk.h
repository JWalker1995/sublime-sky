#pragma once

#include "graphics/glm.h"
#include <glm/vec3.hpp>

#include "world/chunk.h"

namespace game { class GameContext; }

namespace pointgen {

class Chunk {
public:
    static constexpr unsigned int sizeLog2 = world::Chunk::sizeLog2;
    static constexpr unsigned int size = world::Chunk::size;

    glm::vec3 points[size][size][size];
    Chunk *moreRecentlyUsed;
    Chunk *lessRecentlyUsed;

    static void initializeDependencies(game::GameContext &context) {}
};

}
