#include "meshupdater.h"

#include "util/pool.h"
#include "render/hashtreevertindex.h"
#include "render/camera.h"

namespace render {

MeshUpdater::MeshUpdater(game::GameContext &context)
    : TickableBase(context)
    , facesVecManager(context.get<util::SmallVectorManager<unsigned int>>())
    , meshHandle(context.get<render::SceneManager>().createMesh())
{}

MeshUpdater::~MeshUpdater() {
    SceneManager::VertBuffer &vertBuffer = meshHandle.getSceneManager().getVertBuffer();
    for (std::size_t i = 0; i < vertBuffer.getExtentSize(); i++) {
        SceneManager::VertReader vert = vertBuffer.read(i);
        if (vert.shared.meshIndex == meshHandle.getMeshIndex()) {
            vert.local.facesVec.release(facesVecManager);
        }
    }
}

void MeshUpdater::tick(game::TickerContext &tickerContext) {
    (void) tickerContext;

    SceneManager::MeshMutator meshMutator = meshHandle.mutateMesh();
    meshMutator.shared.transform = context.get<render::Camera>().getTransform();
}

void MeshUpdater::update(glm::vec3 aabbMin, glm::vec3 aabbMax, std::vector<std::pair<unsigned int, glm::vec3>> &internalPoints, std::vector<std::pair<unsigned int, glm::vec3>> &externalPoints) {
    MeshGenRequest *request = context.get<util::Pool<MeshGenRequest>>().alloc(*this, aabbMin, aabbMax, internalPoints, externalPoints);
    context.get<meshgen::MeshGenerator>().generate(request);
}

void MeshUpdater::finishMeshGen(MeshGenRequest *meshGenRequest) {
    static thread_local std::unordered_map<glm::vec3, unsigned int> vertIndexMap;
    assert(vertIndexMap.empty());

    for (const MeshGenRequest::Face &faceReq : meshGenRequest->getDstFacesArray()) {
        vertIndexMap.emplace(faceReq.vertPositions[0], static_cast<unsigned int>(-1));
        vertIndexMap.emplace(faceReq.vertPositions[1], static_cast<unsigned int>(-1));
        vertIndexMap.emplace(faceReq.vertPositions[2], static_cast<unsigned int>(-1));
    }

    auto createFunc = [](SceneManager::VertMutator vert) {
        vert.shared.setNormal(glm::vec3(0.0f));
        vert.shared.setColor(255, 0, 0, 255);
    };

    auto destroyFunc = [this](SceneManager::VertReader vert) {
        unsigned int *faces = vert.local.facesVec.data(facesVecManager);
        unsigned int numFaces = vert.local.facesVec.size();

        for (unsigned int i = 0; i < numFaces; i++) {
            unsigned int f = faces[i];
            const unsigned int *verts = meshHandle.readFace(f).shared.verts;
            for (unsigned int j = 0; j < 3; j++) {
                unsigned int v = verts[j];
                if (v != vert.index) {
                    meshHandle.readVert(v).local.facesVec.remove(facesVecManager, f);
                }
            }
            meshHandle.destroyFace(f);
        }
    };

    glm::vec3 min = meshGenRequest->getRequestAabbMin();
    glm::vec3 max = meshGenRequest->getRequestAabbMax();
    context.get<HashTreeVertIndex>().updateRegion(min, max, vertIndexMap, createFunc, destroyFunc);

    for (const MeshGenRequest::Face &faceReq : meshGenRequest->getDstFacesArray()) {
        SceneManager::FaceMutator face = meshHandle.createFace();
        for (unsigned int i = 0; i < 3; i++) {
            std::unordered_map<glm::vec3, unsigned int>::const_iterator found = vertIndexMap.find(faceReq.vertPositions[i]);
            assert(found != vertIndexMap.cend());
            face.shared.verts[i] = found->second;
            meshHandle.readVert(found->second).local.facesVec.push_back(facesVecManager, face.index);
        }
    }

    vertIndexMap.clear();
}

}
