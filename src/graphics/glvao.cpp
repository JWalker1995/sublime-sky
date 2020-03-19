#include "glvao.h"

#include "defs/GLVAO_ASSERT_BINDINGS.h"

namespace graphics {

GlVao::GlVao() {
    glGenVertexArrays(1, &vaoId);
}

GlVao::~GlVao() {
    glDeleteVertexArrays(1, &vaoId);
}

void GlVao::bind() const {
    glBindVertexArray(vaoId);
}

void GlVao::assertBound() const {
#if GLVAO_ASSERT_BINDINGS
    GLint binding;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &binding);
    assert(static_cast<GLuint>(binding) == vaoId);
#endif
}

GLuint GlVao::prepareProgramAttribute(const char *name, GLuint locationSize) {
    std::vector<Define>::const_iterator i = preparedDefines.cbegin();
    while (i != preparedDefines.cend()) {
        if (std::strcmp(i->name, name) == 0) {
            return i->value;
        }
        i++;
    }

    Define attr;
    attr.name = name;
    attr.value = nextAttributeLocation;
    preparedDefines.push_back(attr);

    nextAttributeLocation += locationSize;

    return attr.value;
}

void GlVao::prepareDefine(const char *name, GLuint value) {
    Define attr;
    attr.name = name;
    attr.value = value;
    preparedDefines.push_back(attr);
}

void GlVao::insertDefines(GpuProgram::Defines &defines) const {
    std::vector<Define>::const_iterator i = preparedDefines.cbegin();
    while (i != preparedDefines.cend()) {
        defines.set(i->name, i->value);
        //glBindAttribLocation(programId, i->location, i->name);
        graphics::GL::catchErrors();
        i++;
    }
}

}
