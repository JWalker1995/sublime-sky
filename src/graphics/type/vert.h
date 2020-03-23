#pragma once

#include "graphics/glm.h"
#include <glm/vec3.hpp>

#include "graphics/gl.h"
#include "util/smallvectormanager.h"

namespace graphics {

class GlVao;

class VertShared {
public:
    static constexpr unsigned int renderFlagSelectedBit = 0;

    GLfloat point[3];
    GLfloat normal[3];

    GLuint meshIndex;
    GLuint renderFlags = 0;
    GLubyte color[4];

    glm::vec3 getPoint() const {
        return glm::vec3(point[0], point[1], point[2]);
    }
    void setPoint(glm::vec3 vec) {
        point[0] = vec.x;
        point[1] = vec.y;
        point[2] = vec.z;
    }

    glm::vec3 getNormal() const {
        return glm::vec3(normal[0], normal[1], normal[2]);
    }
    void setNormal(glm::vec3 vec) {
        normal[0] = vec.x;
        normal[1] = vec.y;
        normal[2] = vec.z;
    }

    void setColor(GLubyte r, GLubyte g, GLubyte b, GLubyte a) {
        color[0] = r;
        color[1] = g;
        color[2] = b;
        color[3] = a;
    }

    static void setupVao(GlVao &vao);
};

class VertLocal {
public:
    util::SmallVectorManager<unsigned int>::Ref facesVec;

    // Each vert's position comes from the point equidistant to 4 cell points.
    // Each byte of this 32-bit int defines the cell of one of those 4 points.
    // Using the UIntCoord of the position as a base, add the first 3 2-bit blocks onto each axis, respectively, and subtract 1.
    // So, if 01234567 is the byte, bits 01 are added to the X axis, 23 to Y, 45 to Z, and 67 are unused.
    // As an example, the byte 010010 applied to the coord (10, 20, 30) would result in the coord (10, 19, 31).
    std::uint32_t connectedCellLsbs;

    // Each vert's position comes from the point equidistant to 4 cell points.
    // This 32-bit int has 27 defined bits, each relating to a cell in the 3x3x3 grid around the position.
    // There should be 4 set bits, each relating to a cell point.
    std::uint32_t connectedCellBitmask;
};

}
