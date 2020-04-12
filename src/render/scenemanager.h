#pragma once

#include "graphics/scenemanager.h"
#include "game/tickercontext.h"

namespace game { class GameContext; }

namespace render {

class SceneManager : public graphics::SceneManager, public game::TickerContext::TickableBase<SceneManager> {
public:
    SceneManager(game::GameContext &context);

    void tick(game::TickerContext &tickerContext);

private:
    void createNullMaterial();
};

}
