#pragma once

#include "pointgen/pointgenerator.h"

namespace game { class GameContext; }

namespace pointgen {

class CubicLatticeGenerator : public PointGenerator {
public:
    CubicLatticeGenerator(game::GameContext &context);

    void generate(Chunk *dst, const spatial::CellKey &cellKey);
};

}
