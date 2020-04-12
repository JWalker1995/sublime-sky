#pragma once

#include "graphics/glm.h"
#include <glm/vec3.hpp>

#include "graphics/gl.h"

namespace graphics {

class GlVao;

class VoronoiCellShared {
public:
    static constexpr unsigned int neighborCellsPerUvec4 = 2 * 4;
    static constexpr unsigned int neighborCellCount = neighborCellsPerUvec4 * 13;

    GLuint cellPosition[3];

    GLushort neighborCells[neighborCellCount];

    GLuint meshIndex;
    GLuint materialIndex = 0;

    static void setupVao(GlVao &vao);
};

class VoronoiCellLocal {
public:
};

}
