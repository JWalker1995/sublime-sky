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
        edgeDir /= glm::length(edgeDir);
        assert(glm::dot(edgeDir, edgeOrigin - v0.shared.getPoint()) >= 0.0f); // Cross product might need to be negated if this fails

        // These all should be pretty close
        float parallelDist0 = glm::dot(p0 - edgeOrigin, edgeDir);
        float parallelDist1 = glm::dot(p1 - edgeOrigin, edgeDir);
        float parallelDist2 = glm::dot(p2 - edgeOrigin, edgeDir);
        float parallelDist = (parallelDist0 + parallelDist1 + parallelDist2) / 3.0f;

        float perpDistSq0 = glm::distance2(p0 - edgeOrigin, parallelDist0 * edgeDir);
        float perpDistSq1 = glm::distance2(p1 - edgeOrigin, parallelDist1 * edgeDir);
        float perpDistSq2 = glm::distance2(p2 - edgeOrigin, parallelDist2 * edgeDir);
        float perpDistSq = (perpDistSq0 + perpDistSq1 + perpDistSq2) / 3.0f;

        float bestSurfaceDistance = std::numeric_limits<float>::infinity();
        glm::vec3 bestP3;

        const spatial::RaycastLookupTable::SearchSequence &seq = raycastLookupTable.lookup(edgeOrigin, edgeDir, parallelDist, perpDistSq);
        for (unsigned int i = 0; true; i++) {
            assert(i < spatial::RaycastLookupTable::sequenceLength);
            if (i == spatial::RaycastLookupTable::sequenceLength) {
                break;
            }

            const spatial::RaycastLookupTable::SearchSequence::Cell cell = seq.cells[i];
            if (cell.minSurfaceDistance >= bestSurfaceDistance) {
                break;
            }

            spatial::UintCoord coord(c1.x + cell.offsetX, c1.y + cell.offsetY, c1.z + cell.offsetZ);
            glm::vec3 cellPt = hashTreeWorld.getPoint(coord);
            float surfaceDistance = spatial::RaycastLookupTable::getSurfaceDistanceAlongRay<float>(edgeOrigin, edgeDir, parallelDist, perpDistSq, cellPt);
            assert(surfaceDistance <= cell.minSurfaceDistance);

            if (surfaceDistance < bestSurfaceDistance) {
                bestSurfaceDistance = surfaceDistance;
                bestP3 = cellPt;
            }
        }

        // We have a tetrahedron, and we need to find the circumcenter
        // https://en.wikipedia.org/wiki/Tetrahedron#Circumcenter
        glm::vec3 tetPoints[4] = { p0, p1, p2, bestP3 };

        // Sort them so we know everything's deterministic
        auto vecComparator = [](const glm::vec3 &a, const glm::vec3 &b) {
            if (a.x != b.x) {
                return a.x < b.x;
            } else if (a.y != b.y) {
                return a.y < b.y;
            } else {
                return a.z < b.z;
            }
        };
        std::sort(tetPoints, tetPoints + 4, vecComparator);

        glm::mat3x3 A(tetPoints[1] - tetPoints[0], tetPoints[2] - tetPoints[0], tetPoints[3] - tetPoints[0]);
        float lenSq0 = glm::length2(tetPoints[0]);
        glm::vec3 b(glm::length2(tetPoints[1]) - lenSq0, glm::length2(tetPoints[2]) - lenSq0, glm::length2(tetPoints[3]) - lenSq0);
        glm::vec3 circumcenter = glm::inverse(glm::transpose(A)) * b * 0.5f;

        float d0 = glm::distance(circumcenter, tetPoints[0]);
        float d1 = glm::distance(circumcenter, tetPoints[1]);
        float d2 = glm::distance(circumcenter, tetPoints[2]);
        float d3 = glm::distance(circumcenter, tetPoints[3]);
        assert(false);

        // Create triangle between v1, v2, and circumcenter
        SceneManager::FaceMutator newFace = meshHandle.createFace();

        newFace.shared.verts[0] = v1.index;
        v1.local.facesVec.push_back(facesVecManager, newFace.index);

        newFace.shared.verts[1] = v2.index;
        v2.local.facesVec.push_back(facesVecManager, newFace.index);

        std::pair<std::unordered_map<glm::vec3, unsigned int>::iterator, bool> found = vertIndices.emplace(circumcenter, 0);
        if (found.second) {
            SceneManager::VertMutator newVert = meshHandle.createVert();
            newVert.shared.setPoint(circumcenter);
            newVert.shared.setColor(0, 100, 0, 255);

            found.first->second = newVert.index;

            newFace.shared.verts[2] = newVert.index;
            newVert.local.facesVec.push_back(facesVecManager, newFace.index);
        } else {
            SceneManager::VertReader newVert = meshHandle.readVert(found.first->second);

            newFace.shared.verts[2] = newVert.index;
            newVert.local.facesVec.push_back(facesVecManager, newFace.index);
        }
    }
}

}
