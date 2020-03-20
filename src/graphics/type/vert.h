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
};

}
