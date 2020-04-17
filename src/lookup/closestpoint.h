#pragma once

#include "jw_util/bitset.h"
#include "spatial/uintcoord.h"

#define INTRACELL_POINT_RESOLUTION_BITS 3
#define CLOSEST_POINT_LOOKUP_TABLE_RESOLUTION_BITS 3

#define CLOSEST_POINT_LOOKUP_TABLE_MAX_QUESTIONS 64

namespace lookup {

class ClosestPoint {
private:
    static constexpr unsigned int tableResolutionBits = CLOSEST_POINT_LOOKUP_TABLE_RESOLUTION_BITS;
    static constexpr unsigned int tableMask = (1 << tableResolutionBits) - 1;

    class Node {
    public:
        enum class Type {
            Question,
            ResultSingle,
            ResultMultiple,
        };
        Type type;

        union {
            // For Type::Question:
            glm::tvec3<signed int> questionOffset;

            // For Type::ResultSingle:
            glm::tvec3<signed int> resultOffset;

            // For Type::ResultMultiple:
            jw_util::Bitset<5 * 5 * 5> resultCandidateCells;
        };

        static constexpr unsigned int numChildren = 1 << (tableResolutionBits * 3);
        Node *children[numChildren];
    };

public:
    class Query {
        friend class ClosestPoint;

    public:
#ifndef NDEBUG
        Query()
            : node(0)
            , nextAnswerIndex(static_cast<unsigned int>(-1))
        {}
#endif

    private:
        const Node *node;
        unsigned int nextAnswerIndex;
        std::uint32_t answers[CLOSEST_POINT_LOOKUP_TABLE_MAX_QUESTIONS];
        static_assert(INTRACELL_POINT_RESOLUTION_BITS <= 10, "Intracell point positions do not fit into 32-bit integers");
    };

    static void run() {
        /*
        spatial::UintCoord coord;

        ClosestPoint &cp = ClosestPoint::getInstance();
        Query query;

        cp.initQuery(query);
        cp.answerQuestion(query, getPointAt(coord));
        while (cp.hasQuestion(query)) {
            spatial::UintCoord neighbor = coord + cp.getQuestionOffset(query);
            cp.answerQuestion(query, getPointAt(neighbor));
        }

        coord += cp.getResultOffset(query);
        */
    }

    ClosestPoint();

    static ClosestPoint &getInstance() {
        static ClosestPoint inst;
        return inst;
    }

    void initQuery(Query &query) const {
        query.node = &root;
        query.nextAnswerIndex = 0;

        assert(getQuestionOffset(query) == glm::tvec3<signed int>(0));
    }

    bool hasQuestion(Query &query) const {
        return query.node->type == Node::Type::Question;
    }

    glm::tvec3<signed int> getQuestionOffset(Query &query) const {
        assert(hasQuestion(query));
        return query.node->questionOffset;
    }

    void answerQuestion(Query &query, unsigned int pointPosition) const {
        assert(hasQuestion(query));

        query.answers[query.nextAnswerIndex++] = pointPosition;

        static_assert(INTRACELL_POINT_RESOLUTION_BITS >= tableResolutionBits, "Lookup table resolution is larger than the cell resolution!");
        static constexpr unsigned int extraBits = INTRACELL_POINT_RESOLUTION_BITS - tableResolutionBits;

        if (extraBits) {
            pointPosition = 0
                    | ((pointPosition >> (extraBits * 1)) & (tableMask << (0 * tableResolutionBits)))
                    | ((pointPosition >> (extraBits * 2)) & (tableMask << (1 * tableResolutionBits)))
                    | ((pointPosition >> (extraBits * 3)) & (tableMask << (2 * tableResolutionBits)));
        }

        assert(pointPosition < Node::numChildren);
        query.node = query.node->children[pointPosition];
    }

    glm::tvec3<signed int> getResultOffset(Query &query) {
        assert(!hasQuestion(query));

        if (query.node->type == Node::Type::ResultSingle) {
            return query.answers[query.node->resultIndex];
        }

        enum class Type {
            Question,
            ResultSingle,
            ResultMultiple,
        };
        Type type;

        union {
            // For Type::Question:
            glm::tvec3<signed int> cellOffset;

            // For Type::ResultSingle:
            unsigned int resultIndex;

            // For Type::ResultMultiple:
            jw_util::Bitset<CLOSEST_POINT_LOOKUP_TABLE_MAX_QUESTIONS> resultCandidates;
        };
    }

private:
    Node root;
};

}
