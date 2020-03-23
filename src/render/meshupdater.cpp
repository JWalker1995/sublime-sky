#include "meshupdater.h"

#include "util/pool.h"
#include "render/hashtreevertindex.h"
#include "render/camera.h"
#include "world/hashtreeworld.h"
#include "spatial/raycastlookuptable.h"

namespace render {

MeshUpdater::MeshUpdater(game::GameContext &context)
    : TickableBase(context)
    , facesVecManager(context.get<util::SmallVectorManager<unsigned int>>())
    , meshHandle(context.get<render::SceneManager>().createMesh())
{
    // TODO: Remove
    spatial::RaycastLookupTable &raycastLookupTable = context.get<spatial::RaycastLookupTable>();
    assert(false);
}

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
        vert.shared.setColor(188,143,143, 255);
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

        vert.local.facesVec.release(facesVecManager);
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

void MeshUpdater::fillHoles() {
    world::HashTreeWorld &hashTreeWorld = context.get<world::HashTreeWorld>();
    spatial::RaycastLookupTable &raycastLookupTable = context.get<spatial::RaycastLookupTable>();

    while (!holeEdges.empty()) {
        HoleEdge &edge = holeEdges.back();

        SceneManager::FaceReader face = meshHandle.readFace(edge.faceIndex);
        SceneManager::VertReader v0 = meshHandle.readVert(face.shared.verts[(edge.edgeDir + 0) % 3]);
        SceneManager::VertReader v1 = meshHandle.readVert(face.shared.verts[(edge.edgeDir + 1) % 3]);
        SceneManager::VertReader v2 = meshHandle.readVert(face.shared.verts[(edge.edgeDir + 2) % 3]);

        spatial::UintCoord c0 = spatial::UintCoord::fromPoint(v0.shared.getPoint());
        spatial::UintCoord c1 = spatial::UintCoord::fromPoint(v1.shared.getPoint());
        spatial::UintCoord c2 = spatial::UintCoord::fromPoint(v2.shared.getPoint());

        // This is the cell not on the edge that we need to extend
        spatial::UintCoord cc0[4] = {
            getConnectedCellCoord<0>(c0, v0.local.connectedCellLsbs),
            getConnectedCellCoord<1>(c0, v0.local.connectedCellLsbs),
            getConnectedCellCoord<2>(c0, v0.local.connectedCellLsbs),
            getConnectedCellCoord<3>(c0, v0.local.connectedCellLsbs),
        };

        spatial::UintCoord cc1[4] = {
            getConnectedCellCoord<0>(c1, v1.local.connectedCellLsbs),
            getConnectedCellCoord<1>(c1, v1.local.connectedCellLsbs),
            getConnectedCellCoord<2>(c1, v1.local.connectedCellLsbs),
            getConnectedCellCoord<3>(c1, v1.local.connectedCellLsbs),
        };

        spatial::UintCoord cc2[4] = {
            getConnectedCellCoord<0>(c2, v2.local.connectedCellLsbs),
            getConnectedCellCoord<1>(c2, v2.local.connectedCellLsbs),
            getConnectedCellCoord<2>(c2, v2.local.connectedCellLsbs),
            getConnectedCellCoord<3>(c2, v2.local.connectedCellLsbs),
        };

        // faceCells[1] and faceCells[2] are the cells used to define the face
        // faceCells[0] is the cell that defines the edge (in addition to the other two)
        spatial::UintCoord faceCells[3];
        unsigned int nextFaceCell = 1;

        unsigned int remainingCc1 = 0 ^ 1 ^ 2 ^ 3;
        unsigned int remainingCc2 = 0 ^ 1 ^ 2 ^ 3;

#ifndef NDEBUG
        bool foundLastEdgeCell = false;
#endif

        // TODO: Maybe just set indices?
        for (unsigned int i = 0; i < 4; i++) {
            unsigned int p1Loc = std::find(cc1, cc1 + 4, cc2[i]) - cc1;
            if (p1Loc != 4) {
                if (std::find(cc0, cc0 + 4, cc2[i]) != cc0 + 4) {
                    faceCells[0] = cc2[i];
#ifndef NDEBUG
                    foundLastEdgeCell = true;
#endif
                } else {
                    faceCells[nextFaceCell++] = cc2[i];
                }

                remainingCc1 ^= p1Loc;
                remainingCc2 ^= i;
            }
        }

        assert(nextFaceCell == 2);
        assert(foundLastEdgeCell);
        assert(hashTreeWorld.getSpaceState(faceCells[1]).isTransparent() != hashTreeWorld.getSpaceState(faceCells[2]).isTransparent());

        if (hashTreeWorld.getSpaceState(faceCells[0]) == hashTreeWorld.getSpaceState(faceCells[1])) {
            // New edge is defined by cc1[remainingCc1], faceCells[0], and faceCells[2]
            faceCells[1] = faceCells[2];
        } else {
            // New edge is defined by cc1[remainingCc1], faceCells[0], and faceCells[1]
        }

        glm::vec3 p0 = hashTreeWorld.getPoint(cc1[remainingCc1]);
        glm::vec3 p1 = hashTreeWorld.getPoint(faceCells[0]);
        glm::vec3 p2 = hashTreeWorld.getPoint(faceCells[1]);

        glm::vec3 edgeOrigin = v1.shared.getPoint();
        glm::vec3 edgeDir = glm::cross(p2 - p1, p1 - p0);
        assert(glm::dot(edgeDir, edgeOrigin - v0.shared.getPoint()) >= 0.0f); // Cross product might need to be negated if this fails

        struct CandidateCell {
            spatial::UintCoord coord;
            float dist;

            bool operator<(const CandidateCell &other) const {
                // We flip this because we want the SMALLEST dist coming first
                return dist > other.dist;
            }
        };
        std::priority_queue<CandidateCell, std::vector<CandidateCell>> queue;
        CandidateCell init;
        init.coord = c0;


        // Find v0 ray intersection


    }
}

}
