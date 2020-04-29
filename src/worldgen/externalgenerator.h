#pragma once

#include <vector>

#include "worldgen/worldgenerator.h"
#include "game/tickercontext.h"

namespace SsProtocol {
namespace Config { struct ExternalWorldGenerator; }
}

namespace game { class GameContext; }
namespace SsProtocol { struct TerrainChunk; }

namespace worldgen {

class ExternalGenerator : public WorldGenerator, public game::TickerContext::TickableBase<ExternalGenerator> {
public:
    ExternalGenerator(game::GameContext &context, const SsProtocol::Config::ExternalWorldGenerator *config);
    ~ExternalGenerator();

    void tick(game::TickerContext &tickerContext);

    void generate(spatial::CellKey cube);

    void handleResponse(const SsProtocol::TerrainChunk *chunk, const std::vector<unsigned int> &materialMap);

private:
    std::vector<spatial::CellKey> generateQueue;

    void sendEnqueuedGenerationRequests();
};

}
