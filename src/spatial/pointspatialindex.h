#pragma once

#include <assert.h>

#include "spatial/octree.h"

namespace spatial {

class PointSpatialIndex {
public:
    class PointDetail {
    public:
        virtual glm::vec3 getPosition(unsigned int index) = 0;
    };

    PointSpatialIndex(const PointDetail *pointDetail)
        : pointDetail(pointDetail)
        , octree(AlignedCube(glm::tvec3<AlignedCube::AxisType>(0), 1))
    {}

    void insertPoint(unsigned int index) {
        glm::vec3 pos = pointDetail->getPosition(index);
        Octree<Node>::OctreeNodeGeom nodeGeom = octree.getLeafContaining(pos);
        insertPointIntoLeaf(index, pos, nodeGeom);
    }

private:
    class Node {
    public:
        static Node leafFlagObj;

        // TODO: Test if artificially lowering this value improves performance
        static constexpr unsigned int numPoints = (sizeof(Node *) * 7) / sizeof(unsigned int) - 1;

        Node *&getChild(unsigned int index) {
            assert(!isLeaf());
            return children[index];
        }

        void setChild(unsigned int index, Node *child) {
            assert(!isLeaf());
            children[index] = child;
        }

        bool isLeaf() const {
            return leafFlag == leafFlagObj;
        }

        void appendIndex(unsigned int index) {
            assert(isLeaf());
            assert(nextOpenIndex != Node::numPoints);

            pointIndices[nextOpenIndex] = index;
            nextOpenIndex++;
        }

        static void splitLeaf(jw_util::Pool<Node> &nodePool, Octree<Node>::OctreeNodeGeom nodeGeom) {
            Node *leafNode = nodeGeom.getNode();
            nodeGeom.setNode(nodePool.alloc());

            for (unsigned int i = 0; i < Node::numPoints; i++) {
                unsigned int peerIndex = leafNode->pointIndices[i];
                glm::vec3 peerPos = pointDetail->getPosition(peerIndex);
                nodeGeom.getChildContaining(peerPos).getNode()->appendIndex(peerIndex);
            }

            nodePool.free(leafNode);
        }

        union {
            // Branch
            Node *children[8];

            // Leaf
            struct {
                Node *leafFlag;
                unsigned int nextOpenIndex;
                unsigned int pointIndices[numPoints];
            };
        };
    };
    static_assert(sizeof(Node) == sizeof(Node*) * 8, "PointSpatialIndex::Node has an unexpected size!");

    const PointDetail *pointDetail;
    Octree<Node> octree;

    void insertPointIntoLeaf(unsigned int index, glm::vec3 pos, Octree<Node>::OctreeNodeGeom nodeGeom) {
        Node &leaf = nodeGeom.getNode();
        assert(leaf.isLeaf());
        if (leaf.nextOpenIndex != Node::numPoints) {
            leaf.appendIndex(index);
        } else {
            octree.splitLeaf(nodeGeom);
            insertPointIntoLeaf(index, pos, nodeGeom.getChildContaining(pos));
        }
    }
};

}
