#pragma once

#include "graphics/scenemanager.h"

namespace game { class GameContext; }

namespace render {

class SceneManager : public graphics::SceneManager {
public:
    SceneManager(game::GameContext &context);
};

}
