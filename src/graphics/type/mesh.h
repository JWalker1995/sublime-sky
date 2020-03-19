#pragma once

#include "graphics/gl.h"
#include "geometry/transform.h"

namespace isosurface { class HashTreeIsosurface; }
namespace spheretree { class Object; }

namespace graphics {

class GlVao;

class MeshShared {
public:
    glm::mat4x4 transform;

    static void setupVao(GlVao &vao);
};

class MeshLocal {
public:
    isosurface::HashTreeIsosurface *patcher = nullptr;
    bool shouldValidate = false;
};

}
