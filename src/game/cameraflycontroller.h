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

    enum class GameMode : int { Creative, Spectator, Orbital };
    int gameMode = static_cast<int>(GameMode::Orbital);

    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 lookDir;
    glm::vec3 upDir;

    glm::dvec2 prevMousePos;

    bool enableGravity = false;

    void tickModeCreative();
    void tickModeSpectator();
    void tickModeOrbital();

    void tickPhysicalCamera();
};

}
