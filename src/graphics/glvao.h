#pragma once

#include <vector>

#include "graphics/gl.h"
#include "graphics/gpuprogram.h"

namespace graphics {

class GlVao {
public:
    GlVao();
    ~GlVao();

    GLuint getId() const {
        return vaoId;
    }

    void bind() const;
    void assertBound() const;

    GLuint prepareProgramAttribute(const char *name, GLuint locationSize);
    void prepareDefine(const char *name, GLuint value);

    void insertDefines(GpuProgram::Defines &defines) const;

private:
    GLuint vaoId;
    GLuint nextAttributeLocation = 0;

    struct Define {
        const char *name;
        GLuint value;
    };

    std::vector<Define> preparedDefines;
};

}
