#pragma once

#include "spatial/cuberotation.h"
#include "spatial/cellkey.h"
#include "world/materialindex.h"

namespace world {

class Material {};

class Node {
public:
    // TODO: Could probably use pointer packing here

    bool isLeaf;
    bool isRigidBody;
    spatial::CubeRotation rotation;

    Node *parent;

    union {
        // If branch
        Node *children;

        // If leaf
        struct {
            MaterialIndex materialIndex;
        };
    };


    spatial::CellKey getCellKey() const {
        if (parent) {
            spatial::CellKey parentKey = parent->getCellKey();
            unsigned int index = this - parent->children;
            return parentKey.grandChild<1>(index & 4, index & 2, index & 1);
        } else {
            return spatial::CellKey::makeRoot();
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
    template <signed int dx, signed int dy, signed int dz, typename VisitorType = NullVisitor>
    Node *getSibling(VisitorType visitor) {
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

        return ps->children + childIndex;
    }
};

}
