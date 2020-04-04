#pragma once

#include "graphics/glm.h"
#include <glm/vec3.hpp>

#include "defs/CHUNK_SIZE_LOG2.h"

namespace game { class GameContext; }

namespace pointgen {

class Chunk {
public:
    static constexpr unsigned int sizeLog2 = CHUNK_SIZE_LOG2;
    static constexpr unsigned int size = static_cast<unsigned int>(1) << sizeLog2;

    glm::vec3 points[size][size][size];
    Chunk *moreRecentlyUsed;
    Chunk *lessRecentlyUsed;

    static void initializeDependencies(game::GameContext &context) {}
};

}
