#pragma once

#include "game/gamecontext.h"

#include "render/program/drawprogram.h"

namespace render {

class DrawColorProgram : public DrawProgram {
public:
    DrawColorProgram(game::GameContext &context);

    virtual void insertDefines(Defines &defines);
    virtual void linkProgram();

    void bind();

private:
    GLuint showTrianglesLocation;
    bool showTrianglesValue = false;
    bool showTrianglesDirty = true;

    GLuint eyePosLocation;
};

}
