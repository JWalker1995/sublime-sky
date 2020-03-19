#pragma once

#include <type_traits>
#include <stack>

#include "jw_util/hash.h"
#include "jw_util/pool.h"

#include "spatial/alignedcube.h"

namespace std {

template <>
struct hash<glm::tvec3<blob_planet::spatial::AlignedCube::AxisType>>
{
    std::size_t operator()(const glm::tvec3<blob_planet::spatial::AlignedCube::AxisType>& coord) const
    {
        std::size_t hash = 0;
        hash = jw_util::Hash::combine(hash, coord.x);
        hash = jw_util::Hash::combine(hash, coord.y);
        hash = jw_util::Hash::combine(hash, coord.z);
        return hash;
    }
};

}

namespace spatial {

template <typename LeafDataType>
class Octree {
public:
    typedef glm::tvec3<AlignedCube::AxisType> Coord;

    class BranchNode;
    class LeafNode;

    class Node {
    public:
        virtual ~Node() {}
        virtual BranchNode *toBranch() = 0;
        virtual LeafNode *toLeaf() = 0;
    };

    class BranchNode : public Node {
    public:
        BranchNode *toBranch() {
            return this;
        }
        LeafNode *toLeaf() {
            return 0;
        }

        Node *&getChild(unsigned int index) {
            return children[index];
        }

        void setChild(unsigned int index, Node *child) {
            children[index] = child;
        }

    private:
        Node *children[8];
    };

    class LeafNode : public Node {
    public:
        template <typename... LeafDataConstructionArgs>
        LeafNode(LeafDataConstructionArgs... leafDataConstructionArgs)
            : data(std::forward<LeafDataConstructionArgs>(leafDataConstructionArgs)...)
        {}

        BranchNode *toBranch() {
            return 0;
        }
        LeafNode *toLeaf() {
            return this;
        }

        LeafDataType &getData() {
            return data;
        }

    private:
        LeafDataType data;
    };

    class NodeGeom : public AlignedCube {
    public:
        Node **nodePtr;

        Node *getNode() const {
            return *nodePtr;
        }

        void setNode(Node *node) const {
            *nodePtr = node;
        }

        NodeGeom getChildContaining(Coord coord) const {
            return getChildByOctant(getOctantContaining(coord));
        }

        NodeGeom getChildByOctant(unsigned int octant) const {
            NodeGeom res;
            *static_cast<AlignedCube *>(&res) = getChildCubeByOctant(octant);
            res.nodePtr = &getNode()->toBranch()->getChild(octant);
            return res;
        }
    };

    /*
    template <typename QueryType>
    class Iterator {
    public:
        Iterator(const QueryType query)
            : query(query)
        {}

        void init(const Octree &octree) {
            pushGeom(octree.getRootGeom());
        }

        bool has() const {
            return !geomStack.empty();
        }

        unsigned int get() const {
            const std::pair<NodeGeom, unsigned int> &top = geomStack.top();
            return top.first.getNode()->getLeafElement(top.second);
        }

        void advance() {
            const std::pair<NodeGeom, unsigned int> &top = geomStack.top();
            top.second++;
            if (top.second == top.first.getNode()->countLeafElements()) {
                popStack();
            }
            if (!query.testElement(top.first.getNode()->getLeafElement(top.second))) {
                advance();
            }
        }

    private:
        QueryType query;
        std::stack<std::pair<NodeGeom, unsigned int>> geomStack;

        void popStack() {
            geomStack.pop();
            advanceTopBranch();
        }

        void advanceTopBranch() {
            const std::pair<NodeGeom, unsigned int> &top = geomStack.top();
            top.second++;
            if (top.second == 8) {
                popStack();
            } else {
                NodeGeom childGeom = top.first.getChildByOctant(top.second);
                pushGeom(childGeom);
            }
        }

        void pushGeom(NodeGeom geom) {
            if (!query.testGeom(geom)) {
                advanceTopBranch();
                return;
            }

            if (geom.getNode()->isLeaf()) {
                if (geom.getNode()->countLeafElements() == 0) {
                    // Quick exit
                    advanceTopBranch();
                } else {
                    geomStack.emplace(geom, 0);
                }
            } else {
                geomStack.emplace(geom, 0);
                pushGeom(geom.getChildByOctant(0));
            }
        }
    };
    */

    Octree() {
        setRootGeom(AlignedCube(glm::tvec3<AlignedCube::AxisType>(0), 1), 0);
        rootGeom.nodePtr = &root;
    }

    jw_util::Pool<BranchNode> &getBranchNodePool() {
        return branchNodePool;
    }
    jw_util::Pool<LeafNode> &getLeafNodePool() {
        return leafNodePool;
    }

    NodeGeom getRootGeom() const {
        return rootGeom;
    }

    void setRootGeom(AlignedCube initialCube, Node *newRoot) {
        root = newRoot;
        static_cast<AlignedCube &>(rootGeom) = initialCube;
    }

    bool containsCoord(Coord coord) const {
        for (unsigned int i = 0; i < 3; i++) {
            if (coord[i] < -rootGeom.radius || coord[i] >= rootGeom.radius) {
                return false;
            }
        }

        return true;
    }

private:
    jw_util::Pool<BranchNode> branchNodePool;
    jw_util::Pool<LeafNode> leafNodePool;

    Node *root;
    NodeGeom rootGeom;
};

}
