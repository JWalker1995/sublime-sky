#pragma once

#include "graphics/glm.h"
#include <glm/vec3.hpp>

namespace game { class GameContext; }

namespace spatial {

class EyePosTracker {
public:
    EyePosTracker(game::GameContext &context);
    ~EyePosTracker();

    EyePosTracker(const EyePosTracker&) = delete;
    EyePosTracker(EyePosTracker&&) = delete;

    EyePosTracker& operator=(const EyePosTracker&) = delete;
    EyePosTracker& operator=(EyePosTracker&&) = delete;

    glm::vec3 getEyePos() const {
        return eyePos;
    }

private:
    game::GameContext &context;

    glm::vec3 eyePos;
};

}
