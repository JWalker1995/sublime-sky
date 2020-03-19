#pragma once

#include <unordered_map>
#include <deque>

#include "graphics/scenemanager.h"

namespace graphics {

class MeshHolePuncher;

class MeshHolePatcher {
public:
    class EdgeVert {
        friend class MeshHolePatcher;

    public:
        EdgeVert(unsigned int vertIndex);

        bool needsProcessing() const {
            return vertIndex != static_cast<unsigned int>(-1);
        }
        void setProcessed() {
            vertIndex = static_cast<unsigned int>(-1);
        }

        EdgeVert *getPrevEdge() const {
            return prev;
        }
        EdgeVert *getNextEdge() const {
            return next;
        }

        unsigned int getFaceIndex() const {
            return nextFaceIndex;
        }

        unsigned int getForwardVertIndex() const {
            return vertIndex;
        }
        unsigned int getBackwardVertIndex() const {
            return prev->vertIndex;
        }

    private:
        EdgeVert *prev;
        EdgeVert *next;
        unsigned int vertIndex;
        unsigned int nextFaceIndex;
        unsigned int nextFaceEdgeDir;
    };

    MeshHolePatcher(SceneManager &sceneManager)
        : sceneManager(sceneManager)
    {}

    void init(const MeshHolePuncher &meshHolePuncher);
    EdgeVert *getNextUnprocessedEdgeVert();
    void finalize();

private:
    SceneManager &sceneManager;
    std::unordered_map<unsigned int, EdgeVert *> vertMap;
    std::deque<EdgeVert> edgeVertPool;
    unsigned int unprocessedIter = 0;

    void addEdgeFromFace(SceneManager::FaceReader face, unsigned int edgeDir);
};

}
