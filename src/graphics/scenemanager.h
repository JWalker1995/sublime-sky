#pragma once

#include "graphics/splitbuffer.h"
#include "graphics/type/mesh.h"
#include "graphics/type/vert.h"
#include "graphics/type/face.h"
#include "graphics/type/material.h"
#include "graphics/type/voronoicell.h"
#include "graphics/type/point.h"

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
    typedef SplitBuffer<VoronoiCellShared, VoronoiCellLocal> VoronoiCellBuffer;
    typedef SplitBuffer<PointShared, PointLocal> PointBuffer;

    typedef MeshBuffer::Reader MeshReader;
    typedef MeshBuffer::Mutator MeshMutator;
    typedef VertBuffer::Reader VertReader;
    typedef VertBuffer::Mutator VertMutator;
    typedef FaceBuffer::Reader FaceReader;
    typedef FaceBuffer::Mutator FaceMutator;
    typedef MaterialBuffer::Reader MaterialReader;
    typedef MaterialBuffer::Mutator MaterialMutator;
    typedef VoronoiCellBuffer::Reader VoronoiCellReader;
    typedef VoronoiCellBuffer::Mutator VoronoiCellMutator;
    typedef PointBuffer::Reader PointReader;
    typedef PointBuffer::Mutator PointMutator;

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

        VoronoiCellMutator createVoronoiCell() {
            VoronoiCellMutator res = sceneManager.voronoiCellBuffer.create();
            res.shared.meshIndex = meshIndex;
            return res;
        }
        void destroyVoronoiCell(unsigned int index) {
            sceneManager.voronoiCellBuffer.destroy(index);
        }
        VoronoiCellReader readVoronoiCell(unsigned int index) const {
            return sceneManager.voronoiCellBuffer.read(index);
        }
        VoronoiCellMutator mutateVoronoiCell(unsigned int index) const {
            return sceneManager.voronoiCellBuffer.mutate(index);
        }

        PointMutator createPoint() {
            PointMutator res = sceneManager.pointBuffer.create();
            res.shared.meshIndex = meshIndex;
            return res;
        }
        void destroyPoint(unsigned int index) {
            sceneManager.pointBuffer.destroy(index);
        }
        PointReader readPoint(unsigned int index) const {
            return sceneManager.pointBuffer.read(index);
        }
        PointMutator mutatePoint(unsigned int index) const {
            return sceneManager.pointBuffer.mutate(index);
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
    VoronoiCellBuffer &getVoronoiCellBuffer() {
        return voronoiCellBuffer;
    }
    PointBuffer &getPointBuffer() {
        return pointBuffer;
    }

    const MeshBuffer &getMeshBuffer() const {
        return meshBuffer;
    }
    const VertBuffer &getVertBuffer() const {
        return vertBuffer;
    }
    const FaceBuffer &getFaceBuffer() const {
        return faceBuffer;
    }
    const MaterialBuffer &getMaterialBuffer() const {
        return materialBuffer;
    }
    const VoronoiCellBuffer &getVoronoiCellBuffer() const {
        return voronoiCellBuffer;
    }
    const PointBuffer &getPointBuffer() const {
        return pointBuffer;
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
    VoronoiCellBuffer voronoiCellBuffer;
    PointBuffer pointBuffer;
};

}
