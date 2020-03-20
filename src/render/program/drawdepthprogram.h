#pragma once

#include "game/gamecontext.h"

#include "render/program/drawprogram.h"

namespace render {

class DrawDepthProgram : public DrawProgram {
public:
    DrawDepthProgram(game::GameContext &context);

    virtual void insertDefines(Defines &defines);

private:
};

}
