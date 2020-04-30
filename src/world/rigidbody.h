#pragma once

#include <vector>
#include <unordered_map>

#include "geometry/transform.h"
#include "spatial/cellkey.h"
#include "render/scenemanager.h"

namespace game { class GameContext; }

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

    render::SceneManager::MeshHandle meshHandle;
    std::unordered_map<spatial::UintCoord, unsigned int, spatial::UintCoord::Hasher> vertIndices;
    std::vector<Node *> changeMaterialQueue;


    RigidBody(game::GameContext &context);

    void tick(game::GameContext &context, float dt);

private:
    template <unsigned int faceIdIndex>
    void updateFaces(game::GameContext &context, Node *node, bool isOpaque);

    unsigned int lookupVertex(spatial::UintCoord coord);
};

}
