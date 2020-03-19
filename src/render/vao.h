#pragma once

#include "game/gamecontext.h"

#include "graphics/glvao.h"

namespace render {

class Vao : public graphics::GlVao {
public:
    Vao(game::GameContext &context);
};

}
