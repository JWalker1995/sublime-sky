#pragma once

#include "graphics/glm.h"
#include <glm/vec3.hpp>
#include <glm/mat3x3.hpp>

namespace geometry {

class Transform {
public:
    glm::vec3 translate;
    glm::mat3x3 rotate;

    glm::mat4x4 toMat4x4() const {
        glm::mat4x4 res;
        res[0] = glm::vec4(rotate[0], 0.0f);
        res[1] = glm::vec4(rotate[1], 0.0f);
        res[2] = glm::vec4(rotate[2], 0.0f);
        res[3] = glm::vec4(translate, 1.0f);
        return res;
    }

    glm::vec3 apply(const glm::vec3 &pos) const {
        return translate + rotate * pos;
    }

    static Transform mul(const Transform &a, const Transform &b) {
        // As a 4x4 matrix:
        //                      [b.rot, b.trans]
        //                      [0    , 1      ]
        //
        // [a.rot, a.trans]     [a.rot * b.rot, a.rot * b.trans + a.trans]
        // [0    , 1      ]     [0            , 1                        ]

        Transform res;
        res.translate = a.rotate * b.translate + a.translate;
        res.rotate = a.rotate * b.rotate;
        return res;
    }
};

}
