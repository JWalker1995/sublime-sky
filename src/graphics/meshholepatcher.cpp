#include "meshholepatcher.h"

#include "graphics/meshholepuncher.h"

namespace graphics {

MeshHolePatcher::EdgeVert::EdgeVert(unsigned int vertIndex)
    : vertIndex(vertIndex)
{
    assert(vertIndex != static_cast<unsigned int>(-1));

#ifndef NDEBUG
    prev = nullptr;
    next = nullptr;
#endif
}

/*
void MeshHolePatcher::EdgeVert::moveTo(EdgeVert *dst) const {
    dst->prev = prev;
    dst->next = next;
    dst->nextFaceIndex = nextFaceIndex;
    dst->nextFaceEdgeDir = nextFaceEdgeDir;
    dst->vertIndex = vertIndex;

    prev->next = dst;
    next->prev = dst;
}

void MeshHolePatcher::EdgeVert::extrudeFace(unsigned int vertIndex) {
    // TODO: Optimize by minimzing allocation immediately followed by deallocation

    SceneManager::FaceMutator newFace = meshHolePatcher.meshHandle.createFace();

    meshHolePatcher.edgeVertPool.emplace_back(meshHolePatcher, newFace.index, 1, vertIndex);
    EdgeVert *extrude1 = &meshHolePatcher.edgeVertPool.back();
    meshHolePatcher.edgeVertPool.emplace_back(meshHolePatcher, newFace.index, 2, vertIndex);
    EdgeVert *extrude2 = &meshHolePatcher.edgeVertPool.back();

    prev->next = extrude1;
    extrude1->next = extrude2;
    extrude2->next = next;

    next->prev = extrude2;
    extrude2->prev = extrude1;
    extrude1->prev = prev;

    meshHolePatcher.meshHandle.getFaceBuffer().read(nextFaceIndex).local.oppositeFaces[nextFaceEdgeDir] = newFace.index;
    newFace.local.oppositeFaces[0] = nextFaceIndex;

    tryMerge(prev, extrude1);
    tryMerge(extrude2, next);

    meshHolePatcher.releaseEdge(this);
}

void MeshHolePatcher::EdgeVert::tryMerge(EdgeVert *first, EdgeVert *second) {
    assert(first->next == second);
    assert(second->prev == first);
    assert(first->getForwardVertIndex() == second->getBackwardVertIndex());
    if (first->getBackwardVertIndex() == second->getForwardVertIndex()) {
        meshHolePatcher.meshHandle.getFaceBuffer().read(first->nextFaceIndex).local.oppositeFaces[first->nextFaceEdgeDir] = second->nextFaceIndex;
        meshHolePatcher.meshHandle.getFaceBuffer().read(second->nextFaceIndex).local.oppositeFaces[second->nextFaceEdgeDir] = first->nextFaceIndex;

        first->prev->next = second->next;
        second->next->prev = first->prev;

        meshHolePatcher.releaseEdge(first);
        meshHolePatcher.releaseEdge(second);
    }
}
*/

void MeshHolePatcher::init(const MeshHolePuncher &meshHolePuncher) {
    assert(vertMap.empty());
    assert(edgeVertPool.empty());
    assert(unprocessedIter == 0);

    std::vector<unsigned int>::const_iterator destroyFacesIterator = meshHolePuncher.getDestroyFaces().cbegin();
    while (destroyFacesIterator != meshHolePuncher.getDestroyFaces().cend()) {
        SceneManager::FaceReader face = sceneManager.getFaceBuffer().read(*destroyFacesIterator);
        switch (face.local.destroyedVertsMask) {
            case 0: assert(false); break;
            case 1: addEdgeFromFace(face, 0); break;
            case 2: addEdgeFromFace(face, 1); break;
            case 3: break;
            case 4: addEdgeFromFace(face, 2); break;
            case 5: break;
            case 6: break;
            case 7: break;
            default: assert(false); break;
        }
        destroyFacesIterator++;
    }

#ifndef NDEBUG
    std::deque<EdgeVert>::const_iterator i = edgeVertPool.cbegin();
    while (i != edgeVertPool.cend()) {
        assert(i->prev);
        assert(i->next);
        assert(i->needsProcessing());
        i++;
    }
#endif
}

void MeshHolePatcher::finalize() {
    assert(unprocessedIter == edgeVertPool.size());

    vertMap.clear();
    edgeVertPool.clear();
    unprocessedIter = 0;
}

MeshHolePatcher::EdgeVert *MeshHolePatcher::getNextUnprocessedEdgeVert() {
    std::deque<EdgeVert>::iterator i = std::next(edgeVertPool.begin(), unprocessedIter);

    while (i != edgeVertPool.end()) {
        unprocessedIter++;

        if (i->needsProcessing()) {
            return &*i;
        }
        i++;
    }

    return nullptr;
}

void MeshHolePatcher::addEdgeFromFace(SceneManager::FaceReader face, unsigned int edgeDir) {
    unsigned int vert1 = face.shared.verts[(edgeDir + 1) % 3];
    unsigned int vert2 = face.shared.verts[(edgeDir + 2) % 3];
    EdgeVert *&edgeVert1 = vertMap[vert1];
    EdgeVert *&edgeVert2 = vertMap[vert2];

    if (!edgeVert1) {
        edgeVertPool.emplace_back(vert1);
        edgeVert1 = &edgeVertPool.back();
    }
    if (!edgeVert2) {
        edgeVertPool.emplace_back(vert2);
        edgeVert2 = &edgeVertPool.back();
    }

    edgeVert1->next = edgeVert2;
    edgeVert2->prev = edgeVert1;

    SceneManager::FaceReader oppositeFace = sceneManager.getFaceBuffer().read(face.local.oppositeFaces[edgeDir]);
    unsigned int oppositeEdgeDir = oppositeFace.local.oppositeFaces.find(face.index).get();
    edgeVert1->nextFaceIndex = oppositeFace.index;
    edgeVert1->nextFaceEdgeDir = oppositeEdgeDir;
}

}
