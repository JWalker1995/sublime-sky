#pragma once

#include "graphics/glm.h"
#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>

#include "spatial/cellkey.h"

namespace world {

class RayDrawer {
public:
    RayDrawer(glm::vec3 origin, glm::vec3 dir, spatial::CellKey initCellKey)
        : cellKey(initCellKey)
    {
        timeStepSize = 1.0f / dir;

        glm::vec3 corner = initCellKey.getCoord(timeStepSize.x >= 0.0f, timeStepSize.y >= 0.0f, timeStepSize.z >= 0.0f).toPoint();
        for (unsigned int i = 0; i < 3; i++) {
            float t = (corner[i] - origin[i]) * timeStepSize[i];
            assert(t >= 0.0f);
            timeToPlane[i] = t;

            timeStepSize[i] = std::ldexp(timeStepSize[i], initCellKey.sizeLog2);
        };
    }

    void enterChildCell() {
        // This method should work, but will test some extra cells because we always enter the low-bit child

        // Go into the low-bit child
        cellKey = cellKey.child<0, 0, 0>();

        // Update time
        for (unsigned int i = 0; i < 3; i++) {
            timeStepSize[i] *= 0.5f;

            if (timeStepSize[i] >= 0.0f) {
                timeToPlane[i] -= timeStepSize[i];
            }
        }
    }

    void enterParentCell() {
        for (unsigned int i = 0; i < 3; i++) {
            if ((cellKey.cellCoord[i] & 1) != (timeStepSize[i] >= 0.0f)) {
                // If going towards positive, but we're in the low-bit child of the parent cell.
                // OR
                // If going towards negative, but we're in the high-bit child of the parent cell.
                // THEN
                // We need to add on the time it would take to cross that child cell
                timeToPlane[i] += std::abs(timeStepSize[i]);
            }

            timeStepSize[i] *= 2.0f;
        }

        cellKey = cellKey.parent();
    }

    spatial::CellKey getCurCellKey() const {
        return cellKey;
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
                if (timeStepSize.x >= 0.0f) {
                    cellKey.cellCoord.x++;
                    timeToPlane.x += timeStepSize.x;
                    return cellKey.cellCoord.x % chunkSize == 0;
                } else {
                    cellKey.cellCoord.x--;
                    timeToPlane.x -= timeStepSize.x;
                    return cellKey.cellCoord.x % chunkSize == chunkSize - 1;
                }
            } else {
                if (timeStepSize.z >= 0.0f) {
                    cellKey.cellCoord.z++;
                    timeToPlane.z += timeStepSize.z;
                    return cellKey.cellCoord.z % chunkSize == 0;
                } else {
                    cellKey.cellCoord.z--;
                    timeToPlane.z -= timeStepSize.z;
                    return cellKey.cellCoord.z % chunkSize == chunkSize - 1;
                }
            }
        } else {
            if (timeToPlane.y < timeToPlane.z) {
                if (timeStepSize.y >= 0.0f) {
                    cellKey.cellCoord.y++;
                    timeToPlane.y += timeStepSize.y;
                    return cellKey.cellCoord.y % chunkSize == 0;
                } else {
                    cellKey.cellCoord.y--;
                    timeToPlane.y -= timeStepSize.y;
                    return cellKey.cellCoord.y % chunkSize == chunkSize - 1;
                }
            } else {
                if (timeStepSize.z >= 0.0f) {
                    cellKey.cellCoord.z++;
                    timeToPlane.z += timeStepSize.z;
                    return cellKey.cellCoord.z % chunkSize == 0;
                } else {
                    cellKey.cellCoord.z--;
                    timeToPlane.z -= timeStepSize.z;
                    return cellKey.cellCoord.z % chunkSize == chunkSize - 1;
                }
            }
        }
    }

private:
    spatial::CellKey cellKey;
    glm::vec3 timeToPlane;
    glm::vec3 timeStepSize;
};

}
