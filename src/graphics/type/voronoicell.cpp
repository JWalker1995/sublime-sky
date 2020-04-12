#include "voronoicell.h"

#include "graphics/glvao.h"

namespace graphics {

void VoronoiCellShared::setupVao(GlVao &vao) {
    GLuint positionLocation = vao.prepareProgramAttribute("CELL_POSITION_LOCATION", 1);
    glVertexAttribIPointer(positionLocation, 3, GL_UNSIGNED_INT, sizeof(VoronoiCellShared), reinterpret_cast<void *>(offsetof(VoronoiCellShared, cellPosition)));
    glEnableVertexAttribArray(positionLocation);
    graphics::GL::catchErrors();

    static constexpr unsigned int numAttributes = neighborCellCount / neighborCellsPerUvec4;
    static_assert (neighborCellCount % neighborCellsPerUvec4 == 0, "Not divisible into uvec4's");
    for (unsigned int i = 0; i < numAttributes; i++) {
        GLuint neighborCellLocation = vao.prepareProgramAttribute("NEIGHBOR_CELL_LOCATION_" + std::to_string(i), 1);
        glVertexAttribIPointer(neighborCellLocation, 4, GL_UNSIGNED_INT, sizeof(VoronoiCellShared), reinterpret_cast<void *>(offsetof(VoronoiCellShared, neighborCells[i * neighborCellsPerUvec4])));
        glEnableVertexAttribArray(neighborCellLocation);
        graphics::GL::catchErrors();
    }

    GLuint meshIndexLocation = vao.prepareProgramAttribute("MESH_INDEX_LOCATION", 1);
    glVertexAttribIPointer(meshIndexLocation, 1, GL_UNSIGNED_INT, sizeof(VoronoiCellShared), reinterpret_cast<void *>(offsetof(VoronoiCellShared, meshIndex)));
    glEnableVertexAttribArray(meshIndexLocation);
    graphics::GL::catchErrors();

    GLuint materialIndexLocation = vao.prepareProgramAttribute("MATERIAL_INDEX_LOCATION", 1);
    glVertexAttribIPointer(materialIndexLocation, 1, GL_UNSIGNED_INT, sizeof(VoronoiCellShared), reinterpret_cast<void *>(offsetof(VoronoiCellShared, materialIndex)));
    glEnableVertexAttribArray(materialIndexLocation);
    graphics::GL::catchErrors();
}

}
