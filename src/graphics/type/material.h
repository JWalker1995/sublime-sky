#pragma once

#include "graphics/gl.h"

namespace graphics {

class GlVao;

class MaterialShared {
public:
    GLfloat colorAmbient[4];
    GLfloat colorDiffuse[4];
    GLfloat colorSpecular[4];
    GLfloat shininess;

    enum class RenderModel: GLuint {
        Invisible,
        Phong,
        Blinn,
    };
    RenderModel renderModel;

    static void setupVao(GlVao &vao);

private:
    std::uint32_t _padding[2];
};
static_assert(sizeof(MaterialShared) % 16 == 0, "MaterialShared size won't match up with OpenGL interface block layout std140");

class MaterialLocal {
public:
    std::string name;
    unsigned int originalIndex = static_cast<unsigned int>(-1);

    enum class Phase {
        Unknown,
        Solid,
        Liquid,
        Gas,
    };
    Phase phase;

    float mass;
};

}
