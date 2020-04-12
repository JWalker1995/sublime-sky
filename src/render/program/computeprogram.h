#pragma once

#if 0

#include "game/gamecontext.h"

#include "render/program/program.h"

namespace render {

class ComputeProgram : public Program {
public:
    ComputeProgram(game::GameContext &context);

    virtual void insertDefines(Defines &defines);
    virtual void setupProgram(const Defines &defines);
};

}

#endif
