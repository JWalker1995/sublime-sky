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

const char Shaders::pointCloudVert[] = {
    #include "shaders/point_cloud.vert.glsl.h"
};

const char Shaders::pointCloudFrag[] = {
    #include "shaders/point_cloud.frag.glsl.h"
};

}
