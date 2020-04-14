#pragma once

#include "game/gamecontext.h"

#include "render/program/meshprogram.h"

namespace render {

class MeshDepthProgram : public MeshProgram {
public:
    MeshDepthProgram(game::GameContext &context);

    virtual void insertDefines(Defines &defines);
};

}
