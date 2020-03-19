#pragma once

#include <unordered_set>

#include "graphics/scenemanager.h"

namespace graphics {

class SceneValidator {
public:
    SceneValidator(const SceneManager &sceneManager);

protected:
    const SceneManager &sceneManager;
    std::unordered_set<unsigned int> destroyedFaces;
    std::unordered_set<unsigned int> destroyedVerts;

    bool isFaceActive(unsigned int index) const;
    bool isVertActive(unsigned int index) const;

    void validateActiveFace(SceneManager::FaceBuffer::Viewer face);
    void validateDestroyedFace(SceneManager::FaceBuffer::Viewer face);

    void validateActiveVert(SceneManager::VertBuffer::Viewer vert);
    void validateDestroyedVert(SceneManager::VertBuffer::Viewer vert);
};

}
