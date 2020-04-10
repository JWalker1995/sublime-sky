#pragma once

#include "pointgen/pointgenerator.h"

namespace SsProtocol {
namespace Config { struct RSquaredLatticeGenerator; }
}

namespace game { class GameContext; }

namespace pointgen {

class RSquaredLatticeGenerator : public PointGenerator {
public:
    RSquaredLatticeGenerator(game::GameContext &context, const SsProtocol::Config::RSquaredLatticeGenerator *config);

    void generate(Chunk *dst, const spatial::CellKey &cellKey);
};

}
