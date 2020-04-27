#pragma once

#include <vector>
#include <queue>

#include "graphics/glm.h"
#include <glm/vec3.hpp>
#include <glm/gtx/hash.hpp>

#include "game/tickercontext.h"
#include "render/scenemanager.h"
#include "spatial/cellkey.h"

namespace SsProtocol {
namespace Config { struct MeshGenerator; }
}

namespace game { class GameContext; }

namespace world { class CellValue; }

namespace render {

class MeshUpdater : public game::TickerContext::TickableBase<MeshUpdater> {
public:
    MeshUpdater(game::GameContext &context, const SsProtocol::Config::MeshGenerator *config);
    ~MeshUpdater();

    void tick(game::TickerContext &tickerContext);

    void enqueueCellUpdate(spatial::CellKey cellKey);

//    template <bool enableDestroyGeometry>
    void updateCell(spatial::CellKey cellKey);

    render::SceneManager::MeshHandle getMeshHandle() const {
        return meshHandle;
    }

private:
    util::SmallVectorManager<unsigned int> &facesVecManager;
    SceneManager::MeshHandle meshHandle;

    unsigned int cellUpdatesPerTick;

    class CellKeyComparator {
    public:
        bool operator()(const spatial::CellKey &a, const spatial::CellKey &b) const {
            spatial::UintCoord ca = a.sizeLog2 > 0 ? a.child<1, 1, 1>().getCoord<0, 0, 0>() : a.getCoord<0, 0, 0>();
            spatial::UintCoord cb = b.sizeLog2 > 0 ? b.child<1, 1, 1>().getCoord<0, 0, 0>() : b.getCoord<0, 0, 0>();
            return spatial::UintCoord::distanceSq(ca, center) > spatial::UintCoord::distanceSq(cb, center);
        }

        spatial::UintCoord center;
    };

    CellKeyComparator cellKeyComparator;
    std::priority_queue<spatial::CellKey, std::vector<spatial::CellKey>, CellKeyComparator &> cellUpdateQueue;
    std::queue<spatial::CellKey> delayedCellUpdateQueue;
};

}
