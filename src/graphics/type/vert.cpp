#include "vert.h"

#include "graphics/glvao.h"

namespace graphics {

void VertShared::setupVao(GlVao &vao) {
    // glVertexAttribPointer
    GLuint positionLocation = vao.prepareProgramAttribute("POSITION_LOCATION", 1);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertShared), reinterpret_cast<void *>(offsetof(VertShared, point)));
    glEnableVertexAttribArray(positionLocation);
    graphics::GL::catchErrors();

    GLuint normalLocation = vao.prepareProgramAttribute("NORMAL_LOCATION", 1);
    glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertShared), reinterpret_cast<void *>(offsetof(VertShared, normal)));
    glEnableVertexAttribArray(normalLocation);
    graphics::GL::catchErrors();

    GLuint meshIndexLocation = vao.prepareProgramAttribute("MESH_INDEX_LOCATION", 1);
    glVertexAttribIPointer(meshIndexLocation, 1, GL_UNSIGNED_INT, sizeof(VertShared), reinterpret_cast<void *>(offsetof(VertShared, meshIndex)));
    glEnableVertexAttribArray(meshIndexLocation);
    graphics::GL::catchErrors();

    GLuint materialIndexLocation = vao.prepareProgramAttribute("MATERIAL_INDEX_LOCATION", 1);
    glVertexAttribIPointer(materialIndexLocation, 1, GL_UNSIGNED_INT, sizeof(VertShared), reinterpret_cast<void *>(offsetof(VertShared, materialIndex)));
    glEnableVertexAttribArray(materialIndexLocation);
    graphics::GL::catchErrors();

    GLuint renderFlagsLocation = vao.prepareProgramAttribute("RENDER_FLAGS_LOCATION", 1);
    glVertexAttribIPointer(renderFlagsLocation, 1, GL_UNSIGNED_INT, sizeof(VertShared), reinterpret_cast<void *>(offsetof(VertShared, renderFlags)));
    glEnableVertexAttribArray(renderFlagsLocation);
    graphics::GL::catchErrors();

    vao.prepareDefine("RENDER_FLAGS_SELECTED_BIT", renderFlagSelectedBit);

//    GLuint colorLocation = vao.prepareProgramAttribute("COLOR_LOCATION", 1);
//    glVertexAttribPointer(colorLocation, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertShared), reinterpret_cast<void *>(offsetof(VertShared, color)));
//    glEnableVertexAttribArray(colorLocation);
//    graphics::GL::catchErrors();
}

}
