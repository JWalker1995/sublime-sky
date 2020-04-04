#pragma once

#include "graphics/glm.h"
#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>

#include "spatial/uintcoord.h"

namespace world {

class RayDrawer {
public:
    RayDrawer(glm::vec3 origin, glm::vec3 dir)
        : coord(spatial::UintCoord::fromPoint(origin))
    {
        dir /= glm::length(dir);
        invDir = 1.0f / dir;

        auto calcStep = [](float originX, float dirX) {
            float t = std::floor(originX) - originX;
            if (dirX >= 0.0f) {
                t += 1.0f;
            }
            t /= dirX;
            assert(t >= 0.0f);
            return t;
        };

        timeToPlane = glm::vec3(calcStep(origin.x, dir.x), calcStep(origin.y, dir.y), calcStep(origin.z, dir.z));
    }

    spatial::UintCoord getCurCoord() const {
        return coord;
    }

    float getDistanceAtNextStep() const {
        if (timeToPlane.x < timeToPlane.y) {
            if (timeToPlane.x < timeToPlane.z) {
                return timeToPlane.x;
            } else {
                return timeToPlane.z;
            }
        } else {
            if (timeToPlane.y < timeToPlane.z) {
                return timeToPlane.y;
            } else {
                return timeToPlane.z;
            }
        }
    }

    template <spatial::UintCoord::AxisType chunkSize>
    bool step() {
        // Returns true if this step is over a chunk boundary

        if (timeToPlane.x < timeToPlane.y) {
            if (timeToPlane.x < timeToPlane.z) {
                if (invDir.x >= 0.0f) {
                    coord.x++;
                    timeToPlane.x += invDir.x;
                    return coord.x % chunkSize == 0;
                } else {
                    coord.x--;
                    timeToPlane.x -= invDir.x;
                    return coord.x % chunkSize == chunkSize - 1;
                }
            } else {
                if (invDir.z >= 0.0f) {
                    coord.z++;
                    timeToPlane.z += invDir.z;
                    return coord.z % chunkSize == 0;
                } else {
                    coord.z--;
                    timeToPlane.z -= invDir.z;
                    return coord.z % chunkSize == chunkSize - 1;
                }
            }
        } else {
            if (timeToPlane.y < timeToPlane.z) {
                if (invDir.y >= 0.0f) {
                    coord.y++;
                    timeToPlane.y += invDir.y;
                    return coord.y % chunkSize == 0;
                } else {
                    coord.y--;
                    timeToPlane.y -= invDir.y;
                    return coord.y % chunkSize == chunkSize - 1;
                }
            } else {
                if (invDir.z >= 0.0f) {
                    coord.z++;
                    timeToPlane.z += invDir.z;
                    return coord.z % chunkSize == 0;
                } else {
                    coord.z--;
                    timeToPlane.z -= invDir.z;
                    return coord.z % chunkSize == chunkSize - 1;
                }
            }
        }
    }

private:
    spatial::UintCoord coord;
    glm::vec3 invDir;
    glm::vec3 timeToPlane;
};

}
