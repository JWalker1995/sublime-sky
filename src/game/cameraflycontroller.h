#pragma once

#include "graphics/glm.h"
#include <glm/glm.hpp>

#include "game/tickercontext.h"

namespace game {

class GameContext;

class CameraFlyController : public TickerContext::TickableBase<CameraFlyController> {
public:
    CameraFlyController(game::GameContext &context);

    void tick(game::TickerContext &tickerContext);

private:
    bool isEnabled = true;

    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 lookDir;

    glm::dvec2 prevMousePos;

    bool enableGravity = false;
};

}
