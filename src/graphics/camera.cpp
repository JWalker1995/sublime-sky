#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace graphics {

Camera::Camera() {
    setProjection();
    setView(glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // This is probably overwritten by CameraFlyController
}

void Camera::setProjection() {
    projection = calcProjectionMatrix();
    updateTransform();
}

void Camera::setView(glm::vec3 origin, glm::vec3 dir, glm::vec3 up) {
    view = calcViewMatrix(origin, dir, up);
    eyePos = origin;
    eyeDir = dir;
    updateTransform();
}

glm::mat4x4 Camera::calcProjectionMatrix() {
    return glm::perspective(3.1415f * 0.5f, 640.0f / 480.0f, 0.1f, 1000.0f);
}

glm::mat4x4 Camera::calcViewMatrix(glm::vec3 origin, glm::vec3 dir, glm::vec3 up) {
    return glm::lookAt(origin, origin + dir, up);
}

void Camera::updateTransform() {
    transform = projection * view;
}

}
