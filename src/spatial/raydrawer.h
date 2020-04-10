#pragma once

#include "defs/CHUNK_SIZE_LOG2.h"

#include "spatial/cellkey.h"

namespace spatial {

template <typename VectorType>
class RayDrawer {
private:
    typedef typename VectorType::value_type RealType;

public:
    RayDrawer(VectorType origin, VectorType dir, spatial::CellKey initCellKey)
        : cellKey(initCellKey)
    {
        timeStepSize = static_cast<RealType>(1.0) / dir;

        static constexpr RealType zero = 0.0;
        VectorType corner = initCellKey.getCoord(timeStepSize.x >= zero, timeStepSize.y >= zero, timeStepSize.z >= zero).toPoint();
        for (unsigned int i = 0; i < 3; i++) {
            RealType t = (corner[i] - origin[i]) * timeStepSize[i];
            assert(t >= zero);
            timeToPlane[i] = t;

            timeStepSize[i] = std::ldexp(timeStepSize[i], initCellKey.sizeLog2);
        };
    }

    VectorType getCurCellEnterPosition() const {
        RealType lastStepTime = static_cast<RealType>(0.0);
        for (unsigned int i = 0; i < 3; i++) {
            RealType t = timeToPlane[i] - std::fabs(timeStepSize[i]);
            if (t > lastStepTime) {
                lastStepTime = t;
            }
        }

        VectorType res;
        for (unsigned int i = 0; i < 3; i++) {
            res[i] = (timeToPlane[i] - lastStepTime) / std::fabs(timeStepSize[i]);
            if (timeStepSize[i] >= static_cast<RealType>(0.0)) {
                res[i] = static_cast<RealType>(1.0) - res[i];
            }
            assert(res[i] > static_cast<RealType>(-0.001) && res[i] < static_cast<RealType>(1.001));
        }
        return res;
    }

    VectorType getCurCellExitPosition() const {
        RealType firstStepTime = std::numeric_limits<RealType>::infinity();
        for (unsigned int i = 0; i < 3; i++) {
            RealType t = timeToPlane[i];
            if (t < firstStepTime) {
                firstStepTime = t;
            }
        }

        VectorType res;
        for (unsigned int i = 0; i < 3; i++) {
            res[i] = (timeToPlane[i] - firstStepTime) / std::fabs(timeStepSize[i]);
            if (timeStepSize[i] >= static_cast<RealType>(0.0)) {
                res[i] = static_cast<RealType>(1.0) - res[i];
            }
            assert(res[i] > static_cast<RealType>(-0.001) && res[i] < static_cast<RealType>(1.001));
        }
        return res;
    }

    void enterChildCell() {
        RealType firstStepTime = std::numeric_limits<RealType>::infinity();
        for (unsigned int i = 0; i < 3; i++) {
            RealType t = timeToPlane[i];
            if (t < firstStepTime) {
                firstStepTime = t;
            }
        }

        bool childDir[3];
        for (unsigned int i = 0; i < 3; i++) {
            timeStepSize[i] *= static_cast<RealType>(0.5);

            bool farChild = (timeToPlane[i] - firstStepTime) > std::fabs(timeStepSize[i]);
            childDir[i] = farChild ^ (timeStepSize[i] >= static_cast<RealType>(0.0));
            if (farChild) {
                timeToPlane[i] -= std::fabs(timeStepSize[i]);
            }
        }

        cellKey = cellKey.grandChild<1>(childDir[0], childDir[1], childDir[2]);
    }

    void enterParentCell() {
        for (unsigned int i = 0; i < 3; i++) {
            if ((cellKey.cellCoord[i] & 1) != (timeStepSize[i] >= static_cast<RealType>(0.0))) {
                // If going towards positive, but we're in the low-bit child of the parent cell.
                // OR
                // If going towards negative, but we're in the high-bit child of the parent cell.
                // THEN
                // We need to add on the time it would take to cross that child cell
                timeToPlane[i] += std::fabs(timeStepSize[i]);
            }

            timeStepSize[i] *= static_cast<RealType>(2.0);
        }

        cellKey = cellKey.parent();
    }

    spatial::CellKey getCurCellKey() const {
        return cellKey;
    }

    RealType getDistanceAtNextStep() const {
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
        RealType distance;
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
    VectorType timeToPlane;
    VectorType timeStepSize;

    template <unsigned int stepAxis>
    StepResult stepWithDir() {
        static constexpr spatial::UintCoord::AxisType chunkSize = static_cast<spatial::UintCoord::AxisType>(1) << CHUNK_SIZE_LOG2;

        StepResult res;

//        res.planeImpactPoint.x = (timeToPlane[stepAxis] - timeToPlane[(stepAxis + 1) % 3]) / timeStepSize[(stepAxis + 1) % 3];
//        if (timeStepSize[(stepAxis + 1) % 3] >= static_cast<RealType>(0.0)) {
//            res.planeImpactPoint.x += static_cast<RealType>(1.0);
//        }
//        assert(res.planeImpactPoint.x > -0.001f && res.planeImpactPoint.x < 1.001f);

//        res.planeImpactPoint.y = (timeToPlane[stepAxis] - timeToPlane[(stepAxis + 2) % 3]) / timeStepSize[(stepAxis + 2) % 3];
//        if (timeStepSize[(stepAxis + 2) % 3] >= static_cast<RealType>(0.0)) {
//            res.planeImpactPoint.y += static_cast<RealType>(1.0);
//        }
//        assert(res.planeImpactPoint.y > -0.001f && res.planeImpactPoint.y < 1.001f);

        if (timeStepSize[stepAxis] >= static_cast<RealType>(0.0)) {
//            res.towardsPositive = true;
            res.movedChunk = cellKey.cellCoord[stepAxis] % chunkSize == chunkSize - 1;
            res.distance = timeToPlane[stepAxis];

            cellKey.cellCoord[stepAxis]++;
            timeToPlane[stepAxis] += timeStepSize[stepAxis];
        } else {
//            res.towardsPositive = false;
            res.movedChunk = cellKey.cellCoord[stepAxis] % chunkSize == 0;
            res.distance = timeToPlane[stepAxis];

            cellKey.cellCoord[stepAxis]--;
            timeToPlane[stepAxis] -= timeStepSize[stepAxis];
        }

//        res.stepAxis = stepAxis;
        return res;
    }
};

}
