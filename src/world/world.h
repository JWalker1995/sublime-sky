#pragma once

#include <assert.h>

#include "game/tickercontext.h"
#include "world/rigidbody.h"
#include "world/node.h"

namespace SsProtocol {
namespace Config { struct World; }
}

namespace world {

class World : public game::TickerContext::TickableBase<World> {
public:
    World(game::GameContext &context, const SsProtocol::Config::World *config);

    void tick(game::TickerContext &tickerContext);

    struct RaycastResponse {};
    RaycastResponse raycast(glm::vec3 origin, glm::vec3 dir, float distanceLimit);

//    bool shouldSubdiv(spatial::CellKey cellKey) const;

    Node *getRoot() {
        return &root;
    }

private:
    Node root;

    std::vector<RigidBody> rigidBodies;

    signed int viewChunkSubdivOffsetLog2;
    unsigned int viewChunkLockSizeLog2;

    spatial::UintCoord cameraCoord;
    spatial::UintCoord calcCameraCoord();


    template <signed int dx, signed int dy, signed int dz>
    void moveRigidBody(RigidBody &body) {
        static_assert((!!dx) + (!!dy) + (!!dz) > 0, "Must move at least one step");

        struct MoveVisitor {
            void beforeEnterParent(Node *node) {
                (void) node;
            }

            void beforeEnterChild(Node *node, unsigned int childIndex) {
                (void) node;
                (void) childIndex;
            }
        };
        MoveVisitor visitor;

        Node *dst = body.node->getSibling<dx, dy, dz, MoveVisitor &>(visitor);

        Node *parent = dst->parent;
        *dst = *body.node;
        dst->parent = parent;
    }

//    signed int guessViewChunkSizeLog2(spatial::UintCoord coord) const;
};

}
