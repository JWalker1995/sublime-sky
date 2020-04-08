#pragma once

#include "defs/CHUNK_SIZE_LOG2.h"

#include "graphics/glm.h"
#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>

#include "spatial/cellkey.h"

// TODO: Move to spatial namespace
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

    glm::vec3 getCurCellEnterPosition() const {
        float lastStepTime = 0.0f;
        for (unsigned int i = 0; i < 3; i++) {
            float t = timeToPlane[i] - std::fabs(timeStepSize[i]);
            if (t > lastStepTime) {
                lastStepTime = t;
            }
        }

        glm::vec3 res;
        for (unsigned int i = 0; i < 3; i++) {
            res[i] = (timeToPlane[i] - lastStepTime) / std::fabs(timeStepSize[i]);
            if (timeStepSize[i] >= 0.0f) {
                res[i] = 1.0f - res[i];
            }
            assert(res[i] > -0.001 && res[i] < 1.001f);
        }
        return res;
    }

    glm::vec3 getCurCellExitPosition() const {
        float firstStepTime = std::numeric_limits<float>::infinity();
        for (unsigned int i = 0; i < 3; i++) {
            float t = timeToPlane[i];
            if (t < firstStepTime) {
                firstStepTime = t;
            }
        }

        glm::vec3 res;
        for (unsigned int i = 0; i < 3; i++) {
            res[i] = (timeToPlane[i] - firstStepTime) / std::fabs(timeStepSize[i]);
            if (timeStepSize[i] >= 0.0f) {
                res[i] = 1.0f - res[i];
            }
            assert(res[i] > -0.001 && res[i] < 1.001f);
        }
        return res;
    }

    void enterChildCell() {
        float lastStepTime = 0.0f;
        for (unsigned int i = 0; i < 3; i++) {
            float t = timeToPlane[i] - std::fabs(timeStepSize[i]);
            if (t > lastStepTime) {
                lastStepTime = t;
            }
        }

        bool childDir[3];
        for (unsigned int i = 0; i < 3; i++) {
            timeStepSize[i] *= 0.5f;

            bool farCell = (timeToPlane[i] - lastStepTime) > std::fabs(timeStepSize[i]);
            childDir[i] = farCell ^ (timeStepSize[i] >= 0.0f);
            if (farCell) {
                timeToPlane[i] -= std::fabs(timeStepSize[i]);
            }
        }

        cellKey = cellKey.grandChild<1>(childDir[0], childDir[1], childDir[2]);
    }

    void enterParentCell() {
        for (unsigned int i = 0; i < 3; i++) {
            if ((cellKey.cellCoord[i] & 1) != (timeStepSize[i] >= 0.0f)) {
                // If going towards positive, but we're in the low-bit child of the parent cell.
                // OR
                // If going towards negative, but we're in the high-bit child of the parent cell.
                // THEN
                // We need to add on the time it would take to cross that child cell
                timeToPlane[i] += std::fabs(timeStepSize[i]);
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

//    template <spatial::UintCoord::AxisType chunkSize>
//    bool step() {
        // Returns true if this step is over a chunk boundary

    struct StepResult {
//        glm::vec2 planeImpactPoint;
//        bool towardsPositive;
//        unsigned char stepAxis;
        bool movedChunk;
    };

    StepResult step() {
        if (timeToPlane.x < timeToPlane.y) {
            if (timeToPlane.x < timeToPlane.z) {
                return stepWithDir<0>();
            } else {
                return stepWithDir<2>();
            }
        } else {
            if (timeToPlane.y < timeToPlane.z) {
                return stepWithDir<1>();
            } else {
                return stepWithDir<2>();
            }
        }
    }

private:
    spatial::CellKey cellKey;
    glm::vec3 timeToPlane;
    glm::vec3 timeStepSize;

    template <unsigned int stepAxis>
    StepResult stepWithDir() {
        static constexpr spatial::UintCoord::AxisType chunkSize = static_cast<spatial::UintCoord::AxisType>(1) << CHUNK_SIZE_LOG2;

        StepResult res;

//        res.planeImpactPoint.x = (timeToPlane[stepAxis] - timeToPlane[(stepAxis + 1) % 3]) / timeStepSize[(stepAxis + 1) % 3];
//        if (timeStepSize[(stepAxis + 1) % 3] >= 0.0f) {
//            res.planeImpactPoint.x += 1.0f;
//        }
//        assert(res.planeImpactPoint.x > -0.001f && res.planeImpactPoint.x < 1.001f);

//        res.planeImpactPoint.y = (timeToPlane[stepAxis] - timeToPlane[(stepAxis + 2) % 3]) / timeStepSize[(stepAxis + 2) % 3];
//        if (timeStepSize[(stepAxis + 2) % 3] >= 0.0f) {
//            res.planeImpactPoint.y += 1.0f;
//        }
//        assert(res.planeImpactPoint.y > -0.001f && res.planeImpactPoint.y < 1.001f);

        if (timeStepSize[stepAxis] >= 0.0f) {
//            res.towardsPositive = true;
            cellKey.cellCoord[stepAxis]++;
            timeToPlane[stepAxis] += timeStepSize[stepAxis];
            res.movedChunk = cellKey.cellCoord.z % chunkSize == 0;
        } else {
//            res.towardsPositive = false;
            cellKey.cellCoord[stepAxis]--;
            timeToPlane[stepAxis] -= timeStepSize[stepAxis];
            res.movedChunk = cellKey.cellCoord.z % chunkSize == chunkSize - 1;
        }

//        res.stepAxis = stepAxis;
        return res;
    }
};

}
