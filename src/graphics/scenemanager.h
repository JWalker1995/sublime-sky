#pragma once

#include "graphics/splitbuffer.h"
#include "graphics/type/mesh.h"
#include "graphics/type/vert.h"
#include "graphics/type/face.h"
#include "graphics/type/material.h"

namespace graphics {

class SceneManager {
public:
    /*
    VertShared must implement:
        void setMeshIndex(unsigned int index);
    */

    typedef SplitBuffer<MeshShared, MeshLocal> MeshBuffer;
    typedef SplitBuffer<VertShared, VertLocal> VertBuffer;
    typedef SplitBuffer<FaceShared, FaceLocal> FaceBuffer;
    typedef SplitBuffer<MaterialShared, MaterialLocal> MaterialBuffer;

    typedef MeshBuffer::Reader MeshReader;
    typedef MeshBuffer::Mutator MeshMutator;
    typedef VertBuffer::Reader VertReader;
    typedef VertBuffer::Mutator VertMutator;
    typedef FaceBuffer::Reader FaceReader;
    typedef FaceBuffer::Mutator FaceMutator;
    typedef MaterialBuffer::Reader MaterialReader;
    typedef MaterialBuffer::Mutator MaterialMutator;

    class MeshHandle {
    public:
        MeshHandle(SceneManager &sceneManager, unsigned int meshIndex)
            : sceneManager(sceneManager)
            , meshIndex(meshIndex)
        {}

        MeshHandle &operator=(const MeshHandle &other) {
            assert(&sceneManager == &other.sceneManager);
            meshIndex = other.meshIndex;
            return *this;
        }

        void destroyMesh() {
            sceneManager.meshBuffer.destroy(meshIndex);
            meshIndex = static_cast<unsigned int>(-1);
        }

        MeshReader readMesh() const {
            return sceneManager.meshBuffer.read(meshIndex);
        }
        MeshMutator mutateMesh() const {
            return sceneManager.meshBuffer.mutate(meshIndex);
        }

        VertMutator createVert() {
            VertMutator res = sceneManager.vertBuffer.create();
            res.shared.meshIndex = meshIndex;
            return res;
        }
        void destroyVert(unsigned int index) {
            sceneManager.vertBuffer.destroy(index);
        }
        VertReader readVert(unsigned int index) const {
            return sceneManager.vertBuffer.read(index);
        }
        VertMutator mutateVert(unsigned int index) const {
            return sceneManager.vertBuffer.mutate(index);
        }

        FaceMutator createFace() {
            return sceneManager.faceBuffer.create();
        }
        void destroyFace(unsigned int index) {
            sceneManager.faceBuffer.destroy(index);

            // Make triangle degenerate:
            std::fill_n(mutateFace(index).shared.verts, 3, 0);
        }
        FaceReader readFace(unsigned int index) const {
            return sceneManager.faceBuffer.read(index);
        }
        FaceMutator mutateFace(unsigned int index) const {
            return sceneManager.faceBuffer.mutate(index);
        }

        SceneManager &getSceneManager() const {
            return sceneManager;
        }

        unsigned int getMeshIndex() const {
            return meshIndex;
        }

    private:
        SceneManager &sceneManager;
        unsigned int meshIndex;
    };

    SceneManager();

    MeshBuffer &getMeshBuffer() {
        return meshBuffer;
    }
    VertBuffer &getVertBuffer() {
        return vertBuffer;
    }
    FaceBuffer &getFaceBuffer() {
        return faceBuffer;
    }
    MaterialBuffer &getMaterialBuffer() {
        return materialBuffer;
    }

    const MeshBuffer &getMeshBuffer() const {
        return meshBuffer;
    }
    const VertBuffer &getVertBuffer() const {
        return vertBuffer;
    }
    const FaceBuffer &getFaceBuffer() const{
        return faceBuffer;
    }
    const MaterialBuffer &getMaterialBuffer() const {
        return materialBuffer;
    }

    MeshHandle createMesh() {
        return MeshHandle(*this, meshBuffer.create().index);
    }
    MeshHandle getMesh(unsigned int meshIndex) {
        return MeshHandle(*this, meshIndex);
    }

    MaterialMutator createMaterial() {
        return materialBuffer.create();
    }
    MaterialReader readMaterial(unsigned int index) {
        return materialBuffer.read(index);
    }
    MaterialMutator mutateMaterial(unsigned int index) {
        return materialBuffer.mutate(index);
    }

private:
    MeshBuffer meshBuffer;
    VertBuffer vertBuffer;
    FaceBuffer faceBuffer;
    MaterialBuffer materialBuffer;
};

}
