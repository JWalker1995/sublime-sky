#pragma once

#include "graphics/glm.h"
#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>

#include "spatial/uintcoord.h"

namespace game { class GameContext; }

namespace spatial {

class RaycastLookupTable {
public:
    static constexpr unsigned int originSplits = 4;
    static constexpr unsigned int dirSplits = 8;
    static constexpr unsigned int parallelDistSplits = 8;
    static constexpr unsigned int perpDistSplits = 8;

    static constexpr float minParallelDist = -1.0f;
    static constexpr float maxParallelDist = 2.0f;
    static constexpr float maxPerpDistSq = 1.0f;

    static constexpr unsigned int sequenceLength = 64;

    class SearchSequence {
    public:
        class Cell {
        public:
            float minSurfaceDistance;
            std::int8_t offsetX;
            std::int8_t offsetY;
            std::int8_t offsetZ;

            spatial::UintCoord apply(spatial::UintCoord base) const {
                base.x += offsetX;
                base.y += offsetY;
                base.z += offsetZ;
                return base;
            }
        };

        Cell cells[sequenceLength];
    };

    RaycastLookupTable(game::GameContext &context);

    const SearchSequence &lookup(glm::vec3 origin, glm::vec3 dir, float parallelDist, float perpDistSq) const;

private:
    SearchSequence sequences[originSplits][originSplits][originSplits][dirSplits][dirSplits][parallelDistSplits][perpDistSplits];

    struct Query {
        glm::vec3 origin;
        glm::vec3 dir;
        float parallelDist;
        float perpDistSq;
    };
    Query searchSequenceToLookupQuery(const SearchSequence &searchSequence, float offs[7]) const;

    float getSurfaceDistanceAlongRay(glm::vec3 origin, glm::vec3 dir, float parallelDist, float perpDistSq, glm::vec3 cellPoint);

    float findSmallestSurfaceDistance(const SearchSequence &searchSequence, signed int offsetX, signed int offsetY, signed int offsetZ);
};

}
