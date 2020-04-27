#include "world.h"

#include "schemas/config_client_generated.h"

#include "util/pool.h"

namespace world {

World::World(game::GameContext &context, const SsProtocol::Config::World *config)
    : TickableBase(context)
{
    (void) config;

    spatial::CellKey objKey = spatial::CellKey::fromCoord(spatial::UintCoord::fromPoint(glm::vec3(0.0f, 0.0f, 0.0f)), 4);
    spatial::CellKey curKey = spatial::CellKey::makeRoot();
    Node *curNode = &root;
    while (curKey.sizeLog2 > objKey.sizeLog2) {
        if (curNode->isLeaf) {
//            if (curNode->materialIndex == MaterialIndex::Null) {
                // TODO: Initiate generation here

            curNode->setBranch(context);
//            } else {
//                assert(false);
//            }
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
    curNode->materialIndex = static_cast<MaterialIndex>(3);

    RigidBody body(context);
    body.parent = 0;
    body.node = curNode;
    body.mass = 1.0f;
    body.transform.translate = glm::vec3(0.0f, 0.0f, 0.0f);
    body.transform.rotate = glm::mat3x3(1.0f);
    body.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    body.changeMaterialQueue.push_back(curNode);
    rigidBodies.push_back(body);
}

void World::tick(game::TickerContext &tickerContext) {
    (void) tickerContext;

    static constexpr float dt = 0.01f;

    for (RigidBody &body : rigidBodies) {
//        body.tick(context, dt);
    }
}

}
