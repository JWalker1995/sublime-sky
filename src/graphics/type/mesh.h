#pragma once

#include "graphics/gl.h"
#include "geometry/transform.h"

namespace graphics {

class GlVao;

class MeshShared {
public:
    glm::mat4x4 transform;

    static void setupVao(GlVao &vao);
};
static_assert(sizeof(MeshShared) % 16 == 0, "MeshShared size won't match up with OpenGL interface block layout std140");

class MeshLocal {
public:
    bool shouldValidate = false;
};

}
