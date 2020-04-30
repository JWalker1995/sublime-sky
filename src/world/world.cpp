#include "world.h"

#include <queue>

#include "schemas/config_client_generated.h"

#include "util/pool.h"
#include "render/camera.h"
#include "worldgen/worldgenerator.h"
#include "spatial/raydrawer.h"

namespace world {

World::World(game::GameContext &context, const SsProtocol::Config::World *config)
    : TickableBase(context)
    , viewChunkSubdivOffsetLog2(config->view_chunk_subdiv_offset_log2())
    , viewChunkLockSizeLog2(config->view_chunk_lock_size_log2())
    , cameraCoord(calcCameraCoord())
{
    (void) config;

    /*
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
    */


    RigidBody body(context);
    body.parent = 0;
    body.node = &root;
    body.mass = 0.0f;
    body.transform.translate = glm::vec3(0.0f, 0.0f, 0.0f);
    body.transform.rotate = glm::mat3x3(1.0f);
    body.velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    rigidBodies.push_back(body);
    root.rigidBody = &rigidBodies.back();
}

void World::tick(game::TickerContext &tickerContext) {
    (void) tickerContext;

    static constexpr float dt = 0.01f;

    for (RigidBody &body : rigidBodies) {
        body.tick(context, dt);
    }
}

World::RaycastResponse World::raycast(glm::vec3 origin, glm::vec3 dir, float distanceLimit) {
    static constexpr float distanceOffset = 0.0f;

    dir /= glm::length(dir);

    struct CellRayCollision {
        float collisionDistance;
        spatial::CellKey cellKey;
        Node *node;

        bool operator<(const CellRayCollision &other) const {
            // Sort by least to greatest
            return collisionDistance > other.collisionDistance;
        }
    };
    std::priority_queue<CellRayCollision> cellQueue;

    CellRayCollision init;
    init.collisionDistance = 0.0f;
    init.cellKey = spatial::CellKey::makeRoot();
    init.node = &root;
    cellQueue.push(init);

    do {
        spatial::CellKey parentKey = cellQueue.top().cellKey;
        Node *parentNode = cellQueue.top().node;
        cellQueue.pop();

        if (parentNode->isLeaf) {
            if (parentNode->materialIndex == MaterialIndex::Null) {
                parentNode->materialIndex = MaterialIndex::Generating;
                context.get<worldgen::WorldGenerator>().generate(parentKey);
                RaycastResponse resp;
                return resp;
            } else if (parentNode->materialIndex == MaterialIndex::Generating) {
                RaycastResponse resp;
                return resp;
            } else if (context.get<render::SceneManager>().readMaterial(static_cast<unsigned int>(parentNode->materialIndex)).local.phase != graphics::MaterialLocal::Phase::Gas) {
                RaycastResponse resp;
                return resp;
            } else {
                continue;
            }
        }

        for (unsigned int i = 0; i < 2; i++) {
            for (unsigned int j = 0; j < 2; j++) {
                for (unsigned int k = 0; k < 2; k++) {
                    CellRayCollision child;
                    child.cellKey = parentKey.grandChild<1>(i, j, k);

                    glm::vec3 nearCorner = child.cellKey.getCoord(dir.x <= 0, dir.y <= 0, dir.z <= 0).toPoint();
                    glm::vec3 farCorner = child.cellKey.getCoord(dir.x > 0, dir.y > 0, dir.z > 0).toPoint();
                    glm::vec3 entryTimes = (nearCorner - origin) / dir;
                    glm::vec3 exitTimes = (farCorner - origin) / dir;
                    child.collisionDistance = glm::max(entryTimes.x, glm::max(entryTimes.y, entryTimes.z));
                    if (child.collisionDistance <= distanceLimit) {
                        float exitDistance = glm::min(exitTimes.x, glm::min(exitTimes.y, exitTimes.z));

                        if (exitDistance >= distanceOffset && child.collisionDistance < exitDistance) {
                            child.node = parentNode->children + i * 4 + j * 2 + k * 1;
                            cellQueue.push(child);
                        }
                    }
                }
            }
        }
    } while (!cellQueue.empty());

    RaycastResponse resp;
    return resp;
}

spatial::UintCoord World::calcCameraCoord() {
    return spatial::UintCoord::fromPoint(context.get<render::Camera>().getEyePos());
}

/*
bool World::shouldSubdiv(spatial::CellKey cellKey) const {
    if (cellKey.sizeLog2 == 0) {
        return false;
    }

    signed int desiredSizeLog2 = guessViewChunkSizeLog2(cellKey.child<1, 1, 1>().getCoord<0, 0, 0>());
    return static_cast<signed int>(cellKey.sizeLog2) > desiredSizeLog2;
}

signed int World::guessViewChunkSizeLog2(spatial::UintCoord coord) const {
    if (viewChunkLockSizeLog2) {
        return viewChunkLockSizeLog2;
    }

    spatial::UintCoord::SignedAxisType dx = coord.x - cameraCoord.x;
    spatial::UintCoord::SignedAxisType dy = coord.y - cameraCoord.y;
    spatial::UintCoord::SignedAxisType dz = coord.z - cameraCoord.z;
    std::uint64_t dSqX = static_cast<std::int64_t>(dx) * static_cast<std::int64_t>(dx);
    std::uint64_t dSqY = static_cast<std::int64_t>(dy) * static_cast<std::int64_t>(dy);
    std::uint64_t dSqZ = static_cast<std::int64_t>(dz) * static_cast<std::int64_t>(dz);
    std::uint64_t dSq = dSqX + dSqY + dSqZ;

    if (!dSq) {
        return viewChunkSubdivOffsetLog2;
    }

    unsigned int numSigBits = sizeof(dSq) * CHAR_BIT - __builtin_clzll(dSq);
    signed int desiredSizeLog2 = static_cast<signed int>(numSigBits / 2) + viewChunkSubdivOffsetLog2;
    return desiredSizeLog2;
}
*/

}
