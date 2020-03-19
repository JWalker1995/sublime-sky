#pragma once

#include <vector>

#include "graphics/scenemanager.h"

namespace graphics {

class MeshHolePuncher {
public:
    MeshHolePuncher();
    ~MeshHolePuncher();

    const std::vector<unsigned int> &getDestroyFaces() const {
        return destroyFaces;
    }
    const std::vector<unsigned int> &getDestroyVerts() const {
        return destroyVerts;
    }

    void init();
    void enqueueDestroyVert(SceneManager::MeshHandle meshHandle, SceneManager::VertReader vert);
    void finalize(SceneManager::MeshHandle meshHandle);

private:
    std::vector<unsigned int> destroyFaces;
    std::vector<unsigned int> destroyVerts;
};

}
