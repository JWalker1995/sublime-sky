#pragma once

#include "graphics/gl.h"
#include "graphics/circulararray.h"

namespace graphics {

class GlVao;

class FaceShared {
public:
    CircularArray<GLuint, 3> verts;

    static void setupVao(GlVao &vao);
};

class FaceLocal {
public:
    CircularArray<unsigned int, 3> oppositeFaces;

    unsigned int destroyedVertsMask = 0;
    unsigned int subdivCount = 0;

    bool temp_IcosphereTracer_destroy = false;
    bool DualContouringTracer_pendingDelete = false;
};

}
