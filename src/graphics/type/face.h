#pragma once

#include "graphics/gl.h"
//#include "graphics/circulararray.h"

namespace graphics {

class GlVao;

class FaceShared {
public:
    GLuint verts[3];
//    CircularArray<GLuint, 3> verts;

    static void setupVao(GlVao &vao);
};

class FaceLocal {
public:
};

}
