#pragma once

#include "pointgen/pointgenerator.h"

namespace game { class GameContext; }

namespace pointgen {

class RSquaredLatticeGenerator : public PointGenerator {
public:
    RSquaredLatticeGenerator(game::GameContext &context);

    void generate(Chunk *dst, const spatial::CellKey &cellKey);
};

}
