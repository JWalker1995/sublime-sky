#pragma once

#include "meshgen/meshgenerator.h"

namespace game { class GameContext; }

namespace meshgen {

class CubicHoneycomb : public MeshGenerator {
public:
    CubicHoneycomb(game::GameContext &context);

    void generate(Request *request);

private:
    void createFace(Request *request);
};

}
