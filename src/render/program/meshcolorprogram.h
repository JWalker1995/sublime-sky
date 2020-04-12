#pragma once

#include "game/gamecontext.h"

#include "render/program/meshprogram.h"

namespace render {

class MeshColorProgram : public MeshProgram {
public:
    MeshColorProgram(game::GameContext &context);

    virtual void insertDefines(Defines &defines);
    virtual void linkProgram();

    void draw();

private:
    GLint showTrianglesLocation;
    bool showTrianglesValue = false;
    bool showTrianglesDirty = true;

    GLint eyePosLocation;
};

}
