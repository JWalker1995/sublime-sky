#include "shaders.h"

namespace render {

const char Shaders::mainVert[] = {
    #include "shaders/main.vert.glsl.h"
};

const char Shaders::mainFrag[] = {
    #include "shaders/main.frag.glsl.h"
};

const char Shaders::voronoiCellVert[] = {
    #include "shaders/voronoi_cell.vert.glsl.h"
};

const char Shaders::voronoiCellFrag[] = {
    #include "shaders/voronoi_cell.frag.glsl.h"
};

}
