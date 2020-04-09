#include "meshupdater.h"

#include "voro++-0.4.6/src/cell.hh"

#include "util/pool.h"
#include "render/hashtreevertindex.h"
#include "render/camera.h"
#include "world/hashtreeworld.h"
#include "spatial/raycastlookuptable.h"
#include "graphics/imgui.h"
#include "render/imguirenderer.h"

namespace render {

MeshUpdater::MeshUpdater(game::GameContext &context)
    : TickableBase(context)
    , facesVecManager(context.get<util::SmallVectorManager<unsigned int>>())
    , meshHandle(context.get<render::SceneManager>().createMesh())
{
    // TODO: Remove
//    fillHoles();
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
    tickerContext.get<render::ImguiRenderer::Ticker>();

    if (ImGui::Begin("Debug")) {
        ImGui::Text("Cell update queue size = %zu", cellUpdateQueue.size());
    }
    ImGui::End();

    SceneManager::MeshMutator meshMutator = meshHandle.mutateMesh();
    meshMutator.shared.transform = context.get<render::Camera>().getTransform();

    static constexpr unsigned int maxCellUpdatesPerTick = 256 * 8;
    for (unsigned int i = 0; i < maxCellUpdatesPerTick; i++) {
        if (cellUpdateQueue.empty()) {
            break;
        }

        spatial::CellKey front = cellUpdateQueue.front();
        cellUpdateQueue.pop();
        updateCell(front);
    }
}

/*
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
*/

void MeshUpdater::enqueueCellUpdate(spatial::CellKey cellKey) {
//    assert(false);
//    context.get<world::HashTreeWorld>().getNeedsRegen(coord) = true;
    cellUpdateQueue.push(cellKey);
}

void MeshUpdater::updateCell(spatial::CellKey cellKey) {
    static constexpr bool enableDestroyGeometry = false;

    world::HashTreeWorld &hashTreeWorld = context.get<world::HashTreeWorld>();

    spatial::CellKey chunk = cellKey.grandParent<world::Chunk::sizeLog2>();
    if (hashTreeWorld.shouldSubdivForView(chunk)) {
        updateCell(cellKey.child<0, 0, 0>());
        updateCell(cellKey.child<0, 0, 1>());
        updateCell(cellKey.child<0, 1, 0>());
        updateCell(cellKey.child<0, 1, 1>());
        updateCell(cellKey.child<1, 0, 0>());
        updateCell(cellKey.child<1, 0, 1>());
        updateCell(cellKey.child<1, 1, 0>());
        updateCell(cellKey.child<1, 1, 1>());
        return;
    } else if (!hashTreeWorld.shouldSubdivForView(chunk.parent())) {
        updateCell(cellKey.parent());
        return;
    }

    world::HashTreeWorld::Cell &chunkNode = hashTreeWorld.lookupChunk(chunk);

    unsigned int x = cellKey.cellCoord.x % world::Chunk::size;
    unsigned int y = cellKey.cellCoord.y % world::Chunk::size;
    unsigned int z = cellKey.cellCoord.z % world::Chunk::size;

    if (chunkNode.second.hasFaces.get((x * world::Chunk::size + y) * world::Chunk::size + z)) {
        return;
    }

//    bool &needsRegen = hashTreeWorld.getNeedsRegen(coord);
//    if (needsRegen) {
//        needsRegen = false;
//    } else {
//        return;
//    }

    world::MaterialIndex originMaterialIndex;
    if (chunkNode.second.chunk) {
        originMaterialIndex = chunkNode.second.chunk->cells[x][y][z].materialIndex;
    } else {
        originMaterialIndex = chunkNode.second.constantMaterialIndex;
        switch (originMaterialIndex) {
            case world::MaterialIndex::Null: hashTreeWorld.generateChunk(&chunkNode); return;
            case world::MaterialIndex::Generating: return;
        }
    }

    bool originIsTransparent = hashTreeWorld.isGas(originMaterialIndex);
    if (!enableDestroyGeometry && originIsTransparent) {
        return;
    }

    const pointgen::Chunk *pointChunk = hashTreeWorld.getChunkPoints(&chunkNode);
    glm::vec3 origin = pointChunk->points[x][y][z];
    if (std::isnan(origin.x)) {
        return;
    }

    voro::voronoicell_neighbor cell;
    cell.init(-2.0, 2.0, -2.0, 2.0, -2.0, 2.0);

    glm::vec3 transparentPosSum(0.0f);
    float transparentCount = 0.0f;
    bool hasSurface = false;

    spatial::UintCoord min = cellKey.cellCoord - spatial::UintCoord(2);
    spatial::UintCoord max = cellKey.cellCoord + spatial::UintCoord(2);
    spatial::CellKey neighborCell;
    neighborCell.sizeLog2 = cellKey.sizeLog2;
    for (neighborCell.cellCoord.x = min.x; neighborCell.cellCoord.x <= max.x; neighborCell.cellCoord.x++) {
        for (neighborCell.cellCoord.y = min.y; neighborCell.cellCoord.y <= max.y; neighborCell.cellCoord.y++) {
            for (neighborCell.cellCoord.z = min.z; neighborCell.cellCoord.z <= max.z; neighborCell.cellCoord.z++) {
                if (neighborCell.cellCoord == cellKey.cellCoord) {
                    continue;
                }

                glm::vec3 pt;
                world::MaterialIndex neighborMaterialIndex;

                unsigned int x = neighborCell.cellCoord.x % world::Chunk::size;
                unsigned int y = neighborCell.cellCoord.y % world::Chunk::size;
                unsigned int z = neighborCell.cellCoord.z % world::Chunk::size;

                spatial::CellKey neighborChunk = neighborCell.grandParent<world::Chunk::sizeLog2>();
                if (neighborChunk.cellCoord == chunk.cellCoord) {
                    pt = pointChunk->points[x][y][z];
                    neighborMaterialIndex = chunkNode.second.chunk->cells[x][y][z].materialIndex;
                } else {
                    world::HashTreeWorld::Cell &neighborNode = hashTreeWorld.lookupChunk(neighborChunk);

                    if (neighborNode.second.chunk) {
                        neighborMaterialIndex = neighborNode.second.chunk->cells[x][y][z].materialIndex;
                    } else {
                        neighborMaterialIndex = neighborNode.second.constantMaterialIndex;
                        switch (neighborMaterialIndex) {
                            case world::MaterialIndex::Null: hashTreeWorld.generateChunk(&neighborNode); return;
                            case world::MaterialIndex::Generating: return;
                        }
                    }

                    pt = hashTreeWorld.getChunkPoints(&neighborNode)->points[x][y][z];
                }

                if (std::isnan(pt.x)) {
                    continue;
                }

                bool isTransparent = neighborMaterialIndex != static_cast<world::MaterialIndex>(-1) && hashTreeWorld.isGas(neighborMaterialIndex);
                bool shouldHaveSurface = !originIsTransparent && isTransparent;
                hasSurface |= shouldHaveSurface;

                if (shouldHaveSurface) {
                    transparentPosSum += pt;
                    transparentCount += 1.0f;
                }

                pt -= origin;
                cell.nplane(pt.x, pt.y, pt.z, shouldHaveSurface);
            }
        }
    }

    if (!enableDestroyGeometry && !hasSurface) {
        return;
    }

    if (!originIsTransparent) {
        assert(hasSurface);
        transparentPosSum /= transparentCount;
        glm::vec3 camPos = context.get<render::Camera>().getEyePos();
        float distance = glm::distance(transparentPosSum, camPos);
        if (hashTreeWorld.testViewRay(transparentPosSum, camPos - transparentPosSum, distance).result == world::HashTreeWorld::RaytestResult::HitDistanceLimit) {
            spatial::UintCoord min = cellKey.cellCoord - spatial::UintCoord(1);
            spatial::UintCoord max = cellKey.cellCoord + spatial::UintCoord(1);
            spatial::CellKey neighborCell;
            neighborCell.sizeLog2 = cellKey.sizeLog2;
            for (neighborCell.cellCoord.x = min.x; neighborCell.cellCoord.x <= max.x; neighborCell.cellCoord.x++) {
                for (neighborCell.cellCoord.y = min.y; neighborCell.cellCoord.y <= max.y; neighborCell.cellCoord.y++) {
                    for (neighborCell.cellCoord.z = min.z; neighborCell.cellCoord.z <= max.z; neighborCell.cellCoord.z++) {
                        if (neighborCell.cellCoord == cellKey.cellCoord) {
                            continue;
                        }
                        cellUpdateQueue.push(neighborCell);
                    }
                }
            }
        }
    }

    std::vector<int> faceVerts;
    cell.face_vertices(faceVerts);

    std::vector<double> verts;
    cell.vertices(origin.x, origin.y, origin.z, verts);

    std::vector<int> neighbors;
    cell.neighbors(neighbors);

    // Lookup vertex indices
    // Position should be pretty accurate since the calculations were performed as doubles
    std::vector<unsigned int> vertIndices;
    std::vector<double>::const_iterator vertsIt = verts.cbegin();
    while (vertsIt != verts.cend()) {
        glm::vec3 pt(*vertsIt++, *vertsIt++, *vertsIt++);
        std::pair<std::unordered_map<glm::vec3, unsigned int>::iterator, bool> insert = vertIndexMap.emplace(pt, 0);

        /*
        std::unordered_map<glm::vec3, unsigned int>::const_iterator i = vertIndexMap.cbegin();
        while (i != vertIndexMap.cend()) {
            glm::vec3 pt2 = i->first;
            if (glm::distance2(pt2, pt) < 1e-3f) {
                assert(i == insert.first);
            }
            i++;
        }
        */

        if (insert.second) {
            // Create new vert
            SceneManager::VertMutator newVert = meshHandle.createVert();
            newVert.shared.setPoint(pt);
            insert.first->second = newVert.index;
        }
        vertIndices.push_back(insert.first->second);
    }
    assert(vertIndices.size() * 3 == verts.size());

    unsigned int cellId = chunkNode.second.chunkId + (x * world::Chunk::size + y) * world::Chunk::size + z;

    std::vector<int>::const_iterator faceVertsIt = faceVerts.cbegin();
    for (int neighbor : neighbors) {
        bool shouldHaveFace = neighbor;

        unsigned int numVerts = *faceVertsIt++;
        assert(numVerts >= 3);

        unsigned int minI = static_cast<unsigned int>(-1);
        for (unsigned int i = 0; i < numVerts; i++) {
            unsigned int vi = vertIndices[faceVertsIt[i]];
            unsigned int surfaceCellId = meshHandle.readVert(vi).local.surfaceForCell;
            if (surfaceCellId == static_cast<unsigned int>(-1)) {
                minI = i;
            } else if (surfaceCellId == cellId) {
                minI = i;
                break;

                // Could do some optimizations in this case because we know the surface already exists.
            }
        }

        // If this fails, then we've run out of provoking vertices.
        if (minI == static_cast<unsigned int>(-1)) {
            minI = 0;

            SceneManager::VertReader prevVert = meshHandle.readVert(vertIndices[faceVertsIt[minI]]);
            SceneManager::VertMutator newVert = meshHandle.createVert();
            newVert.shared.setPoint(prevVert.shared.getPoint());
            vertIndices[faceVertsIt[minI]] = newVert.index;
        }

        unsigned int baseVi = vertIndices[faceVertsIt[minI + 0]];
        SceneManager::VertMutator baseVert = meshHandle.mutateVert(baseVi);
        baseVert.shared.materialIndex = static_cast<unsigned int>(originMaterialIndex);
        baseVert.local.surfaceForCell = cellId;

        unsigned int prevVi = vertIndices[faceVertsIt[minI + 1 == numVerts ? 0 : minI + 1]];
        for (unsigned int i = minI + 2; i < numVerts; i++) {
            unsigned int vi = vertIndices[faceVertsIt[i]];

            bool hasFace = false;

            if (enableDestroyGeometry) {
                util::SmallVectorManager<unsigned int>::Ref &facesVec = meshHandle.readVert(vi).local.facesVec;
                const unsigned int *faces = facesVec.data(facesVecManager);
                for (unsigned int j = 0; j < facesVec.size(); j++) {
                    SceneManager::FaceReader face = meshHandle.readFace(faces[j]);
                    if (face.shared.verts[0] == baseVi && face.shared.verts[1] == vi && face.shared.verts[2] == prevVi) {
                        hasFace = true;
                        if (!shouldHaveFace) {
                            for (unsigned int k = 0; k < 3; k++) {
                                meshHandle.readVert(face.shared.verts[k]).local.facesVec.remove(facesVecManager, face.index);
                            }

                            meshHandle.destroyFace(face.index);
                        }
                        break;
                    }
                }
            }

            if (!hasFace && shouldHaveFace) {
                SceneManager::FaceMutator newFace = meshHandle.createFace();
                newFace.shared.verts[0] = baseVi;
                newFace.shared.verts[1] = vi;
                newFace.shared.verts[2] = prevVi;

                meshHandle.readVert(baseVi).local.facesVec.push_back(facesVecManager, newFace.index);
                meshHandle.readVert(prevVi).local.facesVec.push_back(facesVecManager, newFace.index);
                meshHandle.readVert(vi).local.facesVec.push_back(facesVecManager, newFace.index);
            }

            prevVi = vi;
        }

        for (unsigned int i = minI + 1 == numVerts ? 1 : 0; i < minI; i++) {
            unsigned int vi = vertIndices[faceVertsIt[i]];

            bool hasFace = false;

            if (enableDestroyGeometry) {
                util::SmallVectorManager<unsigned int>::Ref &facesVec = meshHandle.readVert(vi).local.facesVec;
                const unsigned int *faces = facesVec.data(facesVecManager);
                for (unsigned int j = 0; j < facesVec.size(); j++) {
                    SceneManager::FaceReader face = meshHandle.readFace(faces[j]);
                    if (face.shared.verts[0] == baseVi && face.shared.verts[1] == vi && face.shared.verts[2] == prevVi) {
                        hasFace = true;
                        if (!shouldHaveFace) {
                            for (unsigned int k = 0; k < 3; k++) {
                                meshHandle.readVert(face.shared.verts[k]).local.facesVec.remove(facesVecManager, face.index);
                            }

                            meshHandle.destroyFace(face.index);
                        }
                        break;
                    }
                }
            }

            if (!hasFace && shouldHaveFace) {
                SceneManager::FaceMutator newFace = meshHandle.createFace();
                newFace.shared.verts[0] = baseVi;
                newFace.shared.verts[1] = vi;
                newFace.shared.verts[2] = prevVi;

                meshHandle.readVert(baseVi).local.facesVec.push_back(facesVecManager, newFace.index);
                meshHandle.readVert(prevVi).local.facesVec.push_back(facesVecManager, newFace.index);
                meshHandle.readVert(vi).local.facesVec.push_back(facesVecManager, newFace.index);
            }

            prevVi = vi;
        }

        faceVertsIt += numVerts;
    }
    assert(faceVertsIt == faceVerts.cend());
}

/*
void MeshUpdater::fillHoles() {
    while (!holeEdges.empty()) {
        HoleEdge edge = holeEdges.front();
        holeEdges.pop();

        fillSingleHole(edge);
    }
}

void MeshUpdater::fillSingleHole(HoleEdge edge) {
    SceneManager::FaceReader face = meshHandle.readFace(edge.faceIndex);
    SceneManager::VertReader v0 = meshHandle.readVert(face.shared.verts[(edge.edgeDir + 0) % 3]);
    SceneManager::VertReader v1 = meshHandle.readVert(face.shared.verts[(edge.edgeDir + 1) % 3]);
    SceneManager::VertReader v2 = meshHandle.readVert(face.shared.verts[(edge.edgeDir + 2) % 3]);

    const unsigned int *v1_faces = v1.local.facesVec.data(facesVecManager);
    for (unsigned int i = 0; i < v1.local.facesVec.size(); i++) {
        if (v1_faces[i] != face.index) {
            const GLuint *faceVerts = meshHandle.readFace(v1_faces[i]).shared.verts;
            unsigned int j = std::find(faceVerts, faceVerts + 3, v1.index) - faceVerts;
            assert(j != 3);
            if (faceVerts[(j + 1) % 3] == v2.index) {
                // Face is already created; skip rest
                return;
            }
        }
    }

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

    world::HashTreeWorld &hashTreeWorld = context.get<world::HashTreeWorld>();

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

    spatial::RaycastLookupTable &raycastLookupTable = context.get<spatial::RaycastLookupTable>();
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

    std::pair<std::unordered_map<glm::vec3, unsigned int>::iterator, bool> found = vertIndexMap.emplace(circumcenter, 0);
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

    HoleEdge newEdge;
    newEdge.faceIndex = newFace.index;

    newEdge.edgeDir = 0;
    holeEdges.push(newEdge);

    newEdge.edgeDir = 1;
    holeEdges.push(newEdge);
}
*/

}
