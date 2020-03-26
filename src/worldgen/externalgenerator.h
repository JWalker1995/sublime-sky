#pragma once

#include "worldgen/worldgenerator.h"

namespace game { class GameContext; }
namespace SublimeSky { struct Chunk; }

namespace worldgen {

class ExternalGenerator : public WorldGenerator {
public:
    ExternalGenerator(game::GameContext &context);

    void generate(spatial::CellKey cube, const pointgen::Chunk *points);

    void handleResponse(const SublimeSky::Chunk *chunk);

private:
    game::GameContext &context;
};

}
