#pragma once

#include "geometry/transform.h"

namespace world {

class Node;

class RigidBody {
public:
    RigidBody *parent;
    Node *node;

    float mass;

    geometry::Transform transform;
    glm::vec3 velocity;

    // glm::vec3 angularVelocity;
    // glm::mat3x3 inertiaMatrix;

    unsigned int meshIndex;
};

}
