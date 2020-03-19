#pragma once

#include "graphics/glm.h"
#include <glm/vec3.hpp>

#include "THST/RTree.h"

namespace render {

class VertSpatialIndex {
public:
private:
    class Point {
    private:
        glm::vec3 data;
    };

    spatial::RTree<int, Point, 3> rtree;
};

}
