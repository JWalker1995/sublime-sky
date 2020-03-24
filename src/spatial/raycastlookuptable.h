#pragma once

#include "jw_util/baseexception.h"

#include "graphics/glm.h"
#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>

namespace game { class GameContext; }

namespace spatial {

class RaycastLookupTable {
public:
    static constexpr unsigned int originSplits = 2;
    static constexpr unsigned int dirSplits = 8;
    static constexpr unsigned int parallelDistSplits = 8;
    static constexpr unsigned int perpDistSplits = 8;

    static constexpr float minParallelDist = -1.0f;
    static constexpr float maxParallelDist = 2.0f;
    static constexpr float maxPerpDistSq = 1.0f;

    static constexpr float surfaceDistanceLowerBound = -1e-3f;
    static constexpr float surfaceDistanceUpperBound = 2.0f;

    static constexpr unsigned int sequenceLength = 256;

    class BuildException : public jw_util::BaseException {
        friend class RaycastLookupTable;

    private:
        BuildException(const std::string &msg)
            : BaseException(msg)
        {}
    };

    class SearchSequence {
    public:
        class Cell {
        public:
            class Hasher {
                std::size_t operator()(const Cell &cell) const {
                    // This hash function should only be used right before a prime modulus
                    // If right before a power of two modulus, need to do proper hashing here
                    std::size_t res = 0;
                    res ^= static_cast<std::size_t>(cell.offsetX) << 0;
                    res ^= static_cast<std::size_t>(cell.offsetY) << 8;
                    res ^= static_cast<std::size_t>(cell.offsetZ) << 16;
                    return res;
                }
            };

            Cell()
                : minSurfaceDistance(NAN)
            {}

            Cell(signed int offsetX, signed int offsetY, signed int offsetZ, float minSurfaceDistance)
                : offsetX(offsetX)
                , offsetY(offsetY)
                , offsetZ(offsetZ)
                , minSurfaceDistance(minSurfaceDistance)
            {
                assert(offsetX < 128 && offsetX >= -128);
                assert(offsetY < 128 && offsetY >= -128);
                assert(offsetZ < 128 && offsetZ >= -128);
            }

            std::int8_t offsetX;
            std::int8_t offsetY;
            std::int8_t offsetZ;
            std::int8_t _padding;
            float minSurfaceDistance;

            bool operator==(const Cell &other) const {
                return offsetX == other.offsetX && offsetY == other.offsetY && offsetZ == other.offsetZ;
            }
        };

        Cell cells[sequenceLength];

        bool isGenerated() const {
            return !std::isnan(cells[0].minSurfaceDistance);
        }
    };

    RaycastLookupTable(game::GameContext &context);

    bool loadSequences(const char *filename);
    void writeSequences(const char *filename);
    void generateSequences();

    const SearchSequence &lookup(glm::vec3 origin, glm::vec3 dir, float parallelDist, float perpDistSq);

    template <typename RealType>
    static RealType getSurfaceDistanceAlongRay(glm::tvec3<RealType> origin, glm::tvec3<RealType> dir, RealType parallelDist, RealType perpDistSq, glm::tvec3<RealType> cellPoint) {
        cellPoint -= origin;

        RealType cellDistParallel = glm::dot(cellPoint, dir);
        RealType cellDistPerpSq = glm::distance2(cellPoint, cellDistParallel * dir);
        RealType surfaceDist = static_cast<RealType>(0.5) * (parallelDist * parallelDist + perpDistSq - cellDistParallel * cellDistParallel - cellDistPerpSq) / (parallelDist - cellDistParallel);

        return surfaceDist;
    }

private:
    SearchSequence sequences[originSplits][originSplits][originSplits][dirSplits][dirSplits][parallelDistSplits][perpDistSplits];

    game::GameContext &context;


    SearchSequence &getSequence(glm::vec3 origin, glm::vec3 dir, float parallelDist, float perpDistSq);

    void generateSingleSequence(SearchSequence &searchSequence);

    struct Query {
        glm::dvec3 origin;
        glm::dvec3 dir;
        double parallelDist;
        double perpDistSq;
    };
    Query searchSequenceToLookupQuery(const SearchSequence &searchSequence, const double offs[7]);

    static constexpr unsigned int nloptArgCount = 10;
    struct NloptUserData {
        RaycastLookupTable *raycastLookupTable;
        const SearchSequence *searchSequence;
    };
    void setMinSurfaceDistance(SearchSequence::Cell &cell, const SearchSequence &searchSequence);
    static double nloptFunc(unsigned int n, const double *x, double *grad, void *userData);
    static double getSurfaceDistForParams(const double *x, NloptUserData *userData);
};

}
