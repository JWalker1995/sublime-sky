#pragma once

#include "graphics/gl.h"

namespace graphics {

class GlVao;

class MaterialShared {
public:
    enum class RenderModel: GLuint {
        Invisible,
        Phong,
        Blinn,
    };
    RenderModel renderModel;

    GLfloat colorDiffuse[4];
    GLfloat colorSpecular[4];
    GLfloat shininess;

    static void setupVao(GlVao &vao);
};

class MaterialLocal {
public:
    std::string name;

    enum class Phase {
        Solid,
        Liquid,
        Gas,
    };
    Phase phase;

    float mass;
};

}
