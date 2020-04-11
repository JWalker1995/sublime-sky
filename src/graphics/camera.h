#pragma once

#include "graphics/glm.h"
#include <glm/glm.hpp>

namespace graphics {

class Camera
{
public:
    Camera();

    void setProjection();
    void setView(glm::vec3 origin, glm::vec3 dir, glm::vec3 up);

    const glm::mat4x4 &getProjection() const {
        return projection;
    }
    const glm::mat4x4 &getView() const {
        return view;
    }
    const glm::mat4x4 &getTransform() const {
        return transform;
    }

    glm::vec3 getEyePos() const {
        return eyePos;
    }
    glm::vec3 getEyeDir() const {
        return eyeDir;
    }

private:
    glm::mat4x4 projection;
    glm::mat4x4 view;
    glm::mat4x4 transform;

    glm::vec3 eyePos;
    glm::vec3 eyeDir;

    static glm::mat4x4 calcProjectionMatrix();
    static glm::mat4x4 calcViewMatrix(glm::vec3 origin, glm::vec3 dir, glm::vec3 up);

    void updateTransform();
};

}
