#pragma once

#include "worldgen/worldgenerator.h"
#include "worldgen/simplexnoise.h"

namespace game { class GameContext; }

namespace worldgen {

class SimpleGenerator : public WorldGenerator {
public:
    SimpleGenerator(game::GameContext &context);

    void generate(spatial::CellKey cube, const pointgen::Chunk *points);

private:
    game::GameContext &context;
    SimplexNoise noise;

    unsigned int airMaterialIndex;
    unsigned int groundMaterialIndex;

    bool getState(glm::vec3 point);
};

}
