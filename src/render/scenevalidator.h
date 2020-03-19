#pragma once

#include "game/gamecontext.h"

#include "graphics/scenevalidator.h"

namespace render {

class SceneValidator : public graphics::SceneValidator {
public:
    SceneValidator(game::GameContext &context);

    void validate();

private:
    game::GameContext &context;
};

}
