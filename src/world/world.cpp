#include "world.h"

#include <array>

#include "schemas/config_client_generated.h"

#include "render/scenemanager.h"
#include "render/camera.h"
#include "util/pool.h"

namespace world {

World::World(game::GameContext &context, const SsProtocol::Config::World *config)
    : TickableBase(context)
{
    (void) config;

    root.isLeaf = true;
    root.isRigidBody = false;
    root.rotation = spatial::CubeRotation::identity();
    root.parent = 0;

    root.materialIndex = MaterialIndex::Null;

    spatial::CellKey objKey = spatial::CellKey::fromCoord(spatial::UintCoord::fromPoint(glm::vec3(0.0f, 0.0f, 0.0f)), 4);
    spatial::CellKey curKey = spatial::CellKey::makeRoot();
    Node *curNode = &root;
    while (curKey.sizeLog2 > objKey.sizeLog2) {
        if (curNode->isLeaf) {
            if (curNode->materialIndex == MaterialIndex::Null) {
                // TODO: Initiate generation here

                curNode->isLeaf = false;
                curNode->children = context.get<util::Pool<std::array<Node, 8>>>().alloc()->data();
            } else {
                assert(false);
            }
        }

        assert(!curNode->isLeaf);
        curKey = curKey.child<0, 0, 0>();

        bool cx = (objKey.cellCoord.x >> (curKey.sizeLog2 - objKey.sizeLog2)) & 1;
        bool cy = (objKey.cellCoord.y >> (curKey.sizeLog2 - objKey.sizeLog2)) & 1;
        bool cz = (objKey.cellCoord.z >> (curKey.sizeLog2 - objKey.sizeLog2)) & 1;

        curKey.cellCoord.x |= cx;
        curKey.cellCoord.y |= cy;
        curKey.cellCoord.z |= cz;

        curNode = curNode->children + ((cx * 4) + (cy * 2) + (cz * 1));
    }
    assert(curKey == objKey);

    curNode->isRigidBody = true;

    RigidBody obj;
    obj.parent = 0;
    obj.node = curNode;
    obj.mass = 1.0f;
    obj.transform.translate = glm::vec3(0.0f, 0.0f, 0.0f);
    obj.transform.rotate = glm::mat3x3(1.0f);
    obj.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    obj.meshIndex = context.get<render::SceneManager>().createMesh().getMeshIndex();
    rigidBodies.push_back(obj);
}

void World::tick(game::TickerContext &tickerContext) {
    (void) tickerContext;

    static constexpr float dt = 0.01f;

    render::SceneManager &sceneManager = context.get<render::SceneManager>();

    for (RigidBody &body : rigidBodies) {
        body.velocity.z -= 0.01f;
        body.velocity *= 0.999f;
        body.transform.translate += body.velocity * dt;

        if (body.transform.translate.x < 0.0f) {
            body.transform.translate.x += 1.0f;
            moveRigidBody<-1, 0, 0>(body);
        } else if (body.transform.translate.x >= 1.0f) {
            body.transform.translate.x -= 1.0f;
            moveRigidBody<+1, 0, 0>(body);
        } else if (body.transform.translate.y < 0.0f) {
            body.transform.translate.y += 1.0f;
            moveRigidBody<0, -1, 0>(body);
        } else if (body.transform.translate.y >= 1.0f) {
            body.transform.translate.y -= 1.0f;
            moveRigidBody<0, +1, 0>(body);
        } else if (body.transform.translate.z < 0.0f) {
            body.transform.translate.z += 1.0f;
            moveRigidBody<0, 0, -1>(body);
        } else if (body.transform.translate.z >= 1.0f) {
            body.transform.translate.z -= 1.0f;
            moveRigidBody<0, 0, +1>(body);
        }

        glm::mat4x4 transform = body.transform.toMat4x4();
        RigidBody *ancestor = &body;
        while (true) {
            ancestor = ancestor->parent;
            if (!ancestor) { break; }
            transform = ancestor->transform.toMat4x4() * transform;
        }

        render::SceneManager::MeshMutator mesh = sceneManager.getMesh(body.meshIndex).mutateMesh();
        mesh.shared.transform = context.get<render::Camera>().getTransform() * transform;
    }
}

}
