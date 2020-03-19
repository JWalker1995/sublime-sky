#pragma once

#include "graphics/scenemanager.h"

namespace graphics {

class VertCirculator {
public:
    VertCirculator(SceneManager::MeshHandle meshHandle, unsigned int centerVertIndex)
        : VertCirculator(meshHandle, centerVertIndex, meshHandle.readVert(centerVertIndex).local.someFaceIndex)
    {}

    VertCirculator(SceneManager::MeshHandle meshHandle, unsigned int centerVertIndex, unsigned int startFaceIndex)
        : meshHandle(meshHandle)
        , centerVertIndex(centerVertIndex)
        , startFaceIndex(startFaceIndex)
        , curFaceIndex(startFaceIndex)
        , curFaceOriginVertDir(meshHandle.readFace(curFaceIndex).shared.verts.find(centerVertIndex).get())
    {
        assert(centerVertIndex != static_cast<unsigned int>(-1));
        assert(startFaceIndex != static_cast<unsigned int>(-1));
    }

    bool isAtBeginning() const {
        return curFaceIndex == startFaceIndex;
    }

    unsigned int getFaceIndex() const {
        return curFaceIndex;
    }

    unsigned int getFaceOriginVertDir() const {
        return curFaceOriginVertDir;
    }

    unsigned int getCenterVertIndex() const {
        return centerVertIndex;
    }

    void advance() {
        curFaceIndex = meshHandle.readFace(curFaceIndex).local.oppositeFaces[(curFaceOriginVertDir + 1) % 3];
        curFaceOriginVertDir = meshHandle.readFace(curFaceIndex).shared.verts.find(centerVertIndex).get();
    }

private:
    SceneManager::MeshHandle meshHandle;
    unsigned int centerVertIndex;
    unsigned int startFaceIndex;
    unsigned int curFaceIndex;
    unsigned int curFaceOriginVertDir;
};

}
