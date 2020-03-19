#pragma once

#include "graphics/camera.h"
#include "game/tickercontext.h"

namespace game { class GameContext; }

namespace render {

class Camera : public graphics::Camera, public game::TickerContext::TickableBase<Camera> {
public:
    Camera(game::GameContext &context);

    void tick(game::TickerContext &tickerContext);
};

}
