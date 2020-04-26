#pragma once

#include "worldgen/worldgenerator.h"

namespace SsProtocol {
namespace Config { struct ExternalWorldGenerator; }
}

namespace game { class GameContext; }
namespace SsProtocol { struct TerrainChunk; }

namespace worldgen {

class ExternalGenerator : public WorldGenerator {
public:
    ExternalGenerator(game::GameContext &context, const SsProtocol::Config::ExternalWorldGenerator *config);

    void generate(spatial::CellKey cube);

    void handleResponse(const SsProtocol::TerrainChunk *chunk, unsigned int materialOffset);

private:
    game::GameContext &context;
};

}
