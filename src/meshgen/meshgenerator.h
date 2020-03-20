#pragma once

#include <vector>

#include "graphics/glm.h"
#include <glm/vec3.hpp>

#include "pointgen/chunk.h"
#include "world/chunk.h"

namespace meshgen {

class MeshGenerator {
public:
    virtual ~MeshGenerator() {}

    class Request {
    public:
        struct Face {
            glm::vec3 vertPositions[3];
        };

        virtual ~Request() {}

        virtual glm::vec3 getRequestAabbMin() const = 0;
        virtual glm::vec3 getRequestAabbMax() const = 0;
        virtual const std::vector<glm::vec3> &getInternalPoints() const = 0;
        virtual const std::vector<glm::vec3> &getExternalPoints() const = 0;
        virtual std::vector<Face> &getDstFacesArray() = 0;

        virtual void onComplete() = 0;
    };

    virtual void generate(Request *request) = 0;
};

}
