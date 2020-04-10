#pragma once

#include "pointgen/pointgenerator.h"

namespace SsProtocol {
namespace Config { struct CubicLatticeGenerator; }
}

namespace game { class GameContext; }

namespace pointgen {

class CubicLatticeGenerator : public PointGenerator {
public:
    CubicLatticeGenerator(game::GameContext &context, const SsProtocol::Config::CubicLatticeGenerator *config);

    void generate(Chunk *dst, const spatial::CellKey &cellKey);
};

}
