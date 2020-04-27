#include "rigidbody.h"

#include "render/scenemanager.h"
#include "render/camera.h"
#include "world/node.h"

namespace world {

RigidBody::RigidBody(game::GameContext &context)
    : meshHandle(context.get<render::SceneManager>().createMesh())
{}

void RigidBody::tick(game::GameContext &context, float dt) {
    velocity.z -= 0.01f;
    velocity *= 0.999f;
    transform.translate += velocity * dt;

    /*
    if (transform.translate.x < 0.0f) {
        transform.translate.x += 1.0f;
//        moveRigidBody<-1, 0, 0>(body);
    } else if (transform.translate.x >= 1.0f) {
        transform.translate.x -= 1.0f;
//        moveRigidBody<+1, 0, 0>(body);
    } else if (transform.translate.y < 0.0f) {
        transform.translate.y += 1.0f;
//        moveRigidBody<0, -1, 0>(body);
    } else if (transform.translate.y >= 1.0f) {
        transform.translate.y -= 1.0f;
//        moveRigidBody<0, +1, 0>(body);
    } else if (transform.translate.z < 0.0f) {
        transform.translate.z += 1.0f;
//        moveRigidBody<0, 0, -1>(body);
    } else if (transform.translate.z >= 1.0f) {
        transform.translate.z -= 1.0f;
//        moveRigidBody<0, 0, +1>(body);
    }
    */

    glm::mat4x4 curTrans = transform.toMat4x4();
    RigidBody *ancestor = this;
    while (true) {
        ancestor = ancestor->parent;
        if (!ancestor) { break; }
        curTrans = ancestor->transform.toMat4x4() * curTrans;
    }

    render::SceneManager &sceneManager = context.get<render::SceneManager>();
    meshHandle.mutateMesh().shared.transform = context.get<render::Camera>().getTransform() * curTrans;

    for (Node *node : changeMaterialQueue) {
        if (node->isLeaf) {
            render::SceneManager::MaterialReader material = sceneManager.readMaterial(static_cast<unsigned int>(node->materialIndex));
            bool isOpaque = material.local.phase != graphics::MaterialLocal::Phase::Gas;
            updateFaces<0>(node, isOpaque);
            updateFaces<1>(node, isOpaque);
            updateFaces<2>(node, isOpaque);
            updateFaces<3>(node, isOpaque);
            updateFaces<4>(node, isOpaque);
            updateFaces<5>(node, isOpaque);
        }
    }
    changeMaterialQueue.clear();
}

template <unsigned int faceIdIndex>
void RigidBody::updateFaces(Node *node, bool isOpaque) {
    Node *sibling = node->getSibling<
            faceIdIndex / 3 == 0 ? faceIdIndex % 2 ? +1 : -1 : 0,
            faceIdIndex / 3 == 1 ? faceIdIndex % 2 ? +1 : -1 : 0,
            faceIdIndex / 3 == 2 ? faceIdIndex % 2 ? +1 : -1 : 0
        >();

    bool isSiblingOpaque;
    if (sibling->isLeaf) {
        render::SceneManager::MaterialReader material = meshHandle.getSceneManager().readMaterial(static_cast<unsigned int>(sibling->materialIndex));
        isSiblingOpaque = material.local.phase != graphics::MaterialLocal::Phase::Gas;
    } else {
        isSiblingOpaque = false;
    }

    bool shouldHaveFace = isOpaque && !isSiblingOpaque;
    bool hasFace = node->faceIds[faceIdIndex] != static_cast<unsigned int>(-1);

    if (shouldHaveFace && !hasFace) {
        render::SceneManager::FaceMutator f1 = meshHandle.createFace();
        render::SceneManager::FaceMutator f2 = meshHandle.createFace();
        assert(f1.index + 1 == f2.index);
        node->faceIds[faceIdIndex] = f1.index;

        spatial::CellKey cellKey = node->getCellKey();

        unsigned int vi[4];
        unsigned int d[3] = {0};
        d[faceIdIndex / 2] = faceIdIndex % 2;
        vi[0] = lookupVertex(cellKey.getCoord(d[0], d[1], d[2]));
        d[(faceIdIndex / 2 + 1) % 3] = 1;
        vi[1] = lookupVertex(cellKey.getCoord(d[0], d[1], d[2]));
        d[(faceIdIndex / 2 + 2) % 3] = 1;
        vi[2] = lookupVertex(cellKey.getCoord(d[0], d[1], d[2]));
        d[(faceIdIndex / 2 + 1) % 3] = 0;
        vi[3] = lookupVertex(cellKey.getCoord(d[0], d[1], d[2]));

        if (faceIdIndex % 2 == 0) {
            std::swap(vi[1], vi[3]);
        }

        unsigned int j = 0;
        while (meshHandle.readVert(vi[j]).local.surfaceForCell != static_cast<unsigned int>(-1)) {
            j++;
            if (j == 4) {
                // TODO: Create vert that can be the provoking vertex
                j = 0;
                break;
            }
        }

        render::SceneManager::VertMutator baseVert = meshHandle.mutateVert(vi[j]);
        baseVert.shared.materialIndex = static_cast<unsigned int>(node->materialIndex);
        baseVert.local.surfaceForCell = 123456;

        f1.shared.verts[0] = vi[j];
        f1.shared.verts[1] = vi[(j + 1) % 4];
        f1.shared.verts[2] = vi[(j + 2) % 4];

        f2.shared.verts[0] = vi[j];
        f2.shared.verts[1] = vi[(j + 2) % 4];
        f2.shared.verts[2] = vi[(j + 3) % 4];
    } else if (!shouldHaveFace && hasFace) {
        unsigned int index = node->faceIds[faceIdIndex];
        meshHandle.destroyFace(index + 1);
        meshHandle.destroyFace(index);
        node->faceIds[faceIdIndex] = static_cast<unsigned int>(-1);
    }
}

unsigned int RigidBody::lookupVertex(spatial::UintCoord coord) {
    std::pair<std::unordered_map<spatial::UintCoord, unsigned int, spatial::UintCoord::Hasher>::iterator, bool> insert = vertIndices.emplace(coord, 0);
    if (insert.second) {
        // Inserted

        render::SceneManager::VertMutator vert = meshHandle.createVert();
        insert.first->second = vert.index;

        vert.shared.setPoint(coord.toPoint());
    }

    return insert.first->second;
}

}
