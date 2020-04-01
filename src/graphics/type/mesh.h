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

class MeshLocal {
public:
    bool shouldValidate = false;
};

}
