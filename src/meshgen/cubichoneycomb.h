#pragma once

#include "meshgen/meshgenerator.h"

namespace game { class GameContext; }

namespace meshgen {

class CubicHoneycomb : public MeshGenerator {
public:
    CubicHoneycomb(game::GameContext &context);

    void generate(Request *request);

private:
    static void createFace(Request *request, unsigned int internalPointId, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3);
};

}
