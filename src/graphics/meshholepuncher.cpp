#include "meshholepuncher.h"

#include "graphics/iterator/vertcirculator.h"

namespace graphics {

MeshHolePuncher::MeshHolePuncher()
{}

MeshHolePuncher::~MeshHolePuncher() {
    assert(destroyFaces.empty());
    assert(destroyVerts.empty());
}

void MeshHolePuncher::init() {
    assert(destroyFaces.empty());
    assert(destroyVerts.empty());
}

void MeshHolePuncher::enqueueDestroyVert(SceneManager::MeshHandle meshHandle, SceneManager::VertReader vert) {
    assert(false);
    VertCirculator vertCirculator(meshHandle, vert.index, 0);
    do {
        SceneManager::FaceReader face = meshHandle.readFace(vertCirculator.getFaceIndex());
        assert(!(face.local.destroyedVertsMask & (1 << vertCirculator.getFaceOriginVertDir())));
        if (!face.local.destroyedVertsMask) {
            destroyFaces.push_back(face.index);
        }
        face.local.destroyedVertsMask |= 1 << vertCirculator.getFaceOriginVertDir();
        vertCirculator.advance();
    } while (!vertCirculator.isAtBeginning());

    destroyVerts.push_back(vert.index);
}

void MeshHolePuncher::finalize(SceneManager::MeshHandle meshHandle) {
    std::vector<unsigned int>::const_iterator destroyFacesIterator = destroyFaces.cbegin();
    while (destroyFacesIterator != destroyFaces.cend()) {
        // TODO: Make sure FaceLocal::destroyedVertsMask is reset when a new face is created
        meshHandle.destroyFace(*destroyFacesIterator);
        destroyFacesIterator++;
    }
    destroyFaces.clear();

    std::vector<unsigned int>::const_iterator destroyVertsIterator = destroyVerts.cbegin();
    while (destroyVertsIterator != destroyVerts.cend()) {
        meshHandle.destroyVert(*destroyVertsIterator);
        destroyVertsIterator++;
    }
    destroyVerts.clear();
}

}
