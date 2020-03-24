#pragma once

#include <vector>
#include <queue>

#include "graphics/glm.h"
#include <glm/vec3.hpp>
#include <glm/gtx/hash.hpp>

#include "game/tickercontext.h"
#include "meshgen/meshgenerator.h"
#include "render/scenemanager.h"
#include "spatial/uintcoord.h"

namespace game { class GameContext; }

namespace render {

class MeshUpdater : public game::TickerContext::TickableBase<MeshUpdater> {
public:
    MeshUpdater(game::GameContext &context);
    ~MeshUpdater();

    void tick(game::TickerContext &tickerContext);

    void update(glm::vec3 aabbMin, glm::vec3 aabbMax, std::vector<std::pair<unsigned int, glm::vec3> > &internalPoints, std::vector<std::pair<unsigned int, glm::vec3> > &externalPoints);

    render::SceneManager::MeshHandle getMeshHandle() const {
        return meshHandle;
    }

private:
    class MeshGenRequest : public meshgen::MeshGenerator::Request {
    public:
        MeshGenRequest(MeshUpdater &meshUpdater, glm::vec3 requestAabbMin, glm::vec3 requestAabbMax, std::vector<std::pair<unsigned int, glm::vec3>> &internalPoints, std::vector<std::pair<unsigned int, glm::vec3>> &externalPoints)
            : meshUpdater(meshUpdater)
            , requestAabbMin(requestAabbMin)
            , requestAabbMax(requestAabbMax)
            , internalPoints(std::move(internalPoints))
            , externalPoints(std::move(externalPoints))
        {}

        glm::vec3 getRequestAabbMin() const {
            return requestAabbMin;
        }
        glm::vec3 getRequestAabbMax() const {
            return requestAabbMax;
        }
        const std::vector<std::pair<unsigned int, glm::vec3>> &getInternalPoints() const {
            return internalPoints;
        };
        const std::vector<std::pair<unsigned int, glm::vec3>> &getExternalPoints() const {
            return externalPoints;
        }
        std::vector<Face> &getDstFacesArray() {
            return dstFaces;
        }

        void onComplete() {
            meshUpdater.finishMeshGen(this);
        }

    private:
        MeshUpdater &meshUpdater;
        glm::vec3 requestAabbMin;
        glm::vec3 requestAabbMax;
        std::vector<std::pair<unsigned int, glm::vec3>> internalPoints;
        std::vector<std::pair<unsigned int, glm::vec3>> externalPoints;
        std::vector<Face> dstFaces;
    };

    void finishMeshGen(MeshGenRequest *meshGenRequest);

    util::SmallVectorManager<unsigned int> &facesVecManager;
    SceneManager::MeshHandle meshHandle;




    std::unordered_map<glm::vec3, unsigned int> vertIndices;

    struct HoleEdge {
        unsigned int faceIndex;
        unsigned int edgeDir;
    };
    std::queue<HoleEdge> holeEdges;

    void fillHoles();
    void fillSingleHole(HoleEdge edge);

    template <unsigned int neighborIndex>
    spatial::UintCoord getConnectedCellCoord(spatial::UintCoord base, std::uint32_t connectedCellLsbs) {
        spatial::UintCoord res;
        res.x = base.x + static_cast<spatial::UintCoord::AxisType>((connectedCellLsbs >> (neighborIndex * 8 + 0)) & 3) - 1;
        res.y = base.y + static_cast<spatial::UintCoord::AxisType>((connectedCellLsbs >> (neighborIndex * 8 + 2)) & 3) - 1;
        res.z = base.z + static_cast<spatial::UintCoord::AxisType>((connectedCellLsbs >> (neighborIndex * 8 + 4)) & 3) - 1;
        return res;
    }
};

}
