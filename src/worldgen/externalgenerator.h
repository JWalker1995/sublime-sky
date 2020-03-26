#pragma once

#include "worldgen/worldgenerator.h"

namespace game { class GameContext; }

namespace worldgen {

class ExternalGenerator : public WorldGenerator {
public:
    ExternalGenerator(game::GameContext &context);

    void generate(Request *request);

private:
    game::GameContext &context;
};

}
