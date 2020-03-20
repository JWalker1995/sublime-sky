#pragma once

#include <vector>

#include "graphics/glm.h"
#include <glm/vec3.hpp>

#include "game/tickercontext.h"
#include "meshgen/meshgenerator.h"
#include "render/scenemanager.h"

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
    render::SceneManager::MeshHandle meshHandle;
};

}
