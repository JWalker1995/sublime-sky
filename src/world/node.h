#pragma once

#include "spatial/cuberotation.h"
#include "spatial/cellkey.h"
#include "world/materialindex.h"

namespace game { class GameContext; }

namespace world {

class RigidBody;

class Node {
public:
    // TODO: Could probably use pointer packing here

    bool isLeaf = true;
    spatial::CubeRotation rotation = spatial::CubeRotation::identity();

    // TODO: Probably can remove this since we have RigidBody::vertIndices now
    unsigned int faceIds[6] = {
        static_cast<unsigned int>(-1),
        static_cast<unsigned int>(-1),
        static_cast<unsigned int>(-1),
        static_cast<unsigned int>(-1),
        static_cast<unsigned int>(-1),
        static_cast<unsigned int>(-1),
    };

    RigidBody *rigidBody = 0;

    Node *parent = 0;

    union {
        // If branch
        Node *children;

        // If leaf
        struct {
            MaterialIndex materialIndex;
        };
    };


    Node() {
        materialIndex = MaterialIndex::Null;
    }


    void setBranch(game::GameContext &context);

    spatial::CellKey getCellKey() const {
        if (parent) {
            spatial::CellKey parentKey = parent->getCellKey();
            unsigned int index = this - parent->children;
            assert(index < 8);
            return parentKey.grandChild<1>((index >> 2) & 1, (index >> 1) & 1, (index >> 0) & 1);
        } else {
            return spatial::CellKey::makeRoot();
        }
    }

    RigidBody *getClosestRigidBody() const {
        if (rigidBody) {
            return rigidBody;
        } else if (parent) {
            return parent->getClosestRigidBody();
        } else {
            return 0;
        }
    }

    struct NullVisitor {
        void beforeEnterParent(Node *node) {
            (void) node;
        }
        void beforeEnterChild(Node *node, unsigned int childIndex) {
            (void) node;
            (void) childIndex;
        }
    };

    template <typename VisitorType = NullVisitor>
    Node *getChild(spatial::CellKey cellKey, VisitorType visitor = VisitorType()) {
        if (cellKey.sizeLog2 == spatial::UintCoord::maxSizeLog2) {
            return this;
        }

        unsigned int bit = spatial::UintCoord::maxSizeLog2 - 1 - cellKey.sizeLog2;
        assert((cellKey.cellCoord.x >> bit) < 2);
        assert((cellKey.cellCoord.y >> bit) < 2);
        assert((cellKey.cellCoord.z >> bit) < 2);
        unsigned int childIndex = 0
                | ((cellKey.cellCoord.x >> bit) << 2)
                | ((cellKey.cellCoord.y >> bit) << 1)
                | ((cellKey.cellCoord.z >> bit) << 0);
        assert(childIndex < 8);

        visitor.beforeEnterChild(this, childIndex);

        if (isLeaf) {
            return this;
        } else {
            spatial::UintCoord::AxisType mask = ~(static_cast<spatial::UintCoord::AxisType>(1) << bit);
            cellKey.cellCoord.x &= mask;
            cellKey.cellCoord.y &= mask;
            cellKey.cellCoord.z &= mask;
            cellKey.sizeLog2++;

            return children[childIndex].getChild<VisitorType>(cellKey, visitor);
        }
    }

    template <signed int dx, signed int dy, signed int dz, typename VisitorType = NullVisitor>
    Node *getSibling(VisitorType visitor = VisitorType()) {
        static_assert((-1) >> 1 == -1, "Unexpected signed right shift behavior");

        if (dx == 0 && dy == 0 && dz == 0) {
            return this;
        }

        assert(parent);
        unsigned int thisIndex = this - parent->children;
        assert(thisIndex < 8);

        visitor.beforeEnterParent(this);

        Node *ps;
        switch (thisIndex) {
            case 0: ps = parent->getSibling<((dx+0) >> 1), ((dy+0) >> 1), ((dz+0) >> 1), VisitorType>(visitor); break;
            case 1: ps = parent->getSibling<((dx+0) >> 1), ((dy+0) >> 1), ((dz+1) >> 1), VisitorType>(visitor); break;
            case 2: ps = parent->getSibling<((dx+0) >> 1), ((dy+1) >> 1), ((dz+0) >> 1), VisitorType>(visitor); break;
            case 3: ps = parent->getSibling<((dx+0) >> 1), ((dy+1) >> 1), ((dz+1) >> 1), VisitorType>(visitor); break;
            case 4: ps = parent->getSibling<((dx+1) >> 1), ((dy+0) >> 1), ((dz+0) >> 1), VisitorType>(visitor); break;
            case 5: ps = parent->getSibling<((dx+1) >> 1), ((dy+0) >> 1), ((dz+1) >> 1), VisitorType>(visitor); break;
            case 6: ps = parent->getSibling<((dx+1) >> 1), ((dy+1) >> 1), ((dz+0) >> 1), VisitorType>(visitor); break;
            case 7: ps = parent->getSibling<((dx+1) >> 1), ((dy+1) >> 1), ((dz+1) >> 1), VisitorType>(visitor); break;
        }

        unsigned int childIndex = thisIndex ^ (dx % 2 ? 4 : 0) ^ (dy % 2 ? 2 : 0) ^ (dz % 2 ? 1 : 0);

        visitor.beforeEnterChild(ps, childIndex);

        if (ps->isLeaf) {
            return ps;
        } else {
            return ps->children + childIndex;
        }
    }
};

}
