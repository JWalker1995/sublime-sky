#pragma once

#include "game/gamecontext.h"

#include "graphics/gpuprogram.h"

namespace graphics { class GlBufferBase; }

namespace render {

class Program : public graphics::GpuProgram {
public:
    Program(game::GameContext &context);
    virtual ~Program() {}

    void make();

protected:
    virtual void insertDefines(Defines &defines);
    virtual void setupProgram(const Defines &defines);
    virtual void linkProgram();

private:
#ifndef NDEBUG
    // Make sure each derived class calls BaseClass::insertDefines(defines) and BaseClass::setupProgram(defines)
    bool calledBaseInsertDefines;
    bool calledBaseSetupProgram;
    bool calledBaseLinkProgram;
#endif
};

}
