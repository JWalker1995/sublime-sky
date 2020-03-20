#include "cameraflycontroller.h"

#include "graphics/glm.h"
#include <glm/gtx/norm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "game/gamecontext.h"
#include "application/window.h"
#include "render/camera.h"

namespace game {

CameraFlyController::CameraFlyController(game::GameContext &context)
    : TickableBase(context)
    , position(0.0f, 0.0f, 10.0f)
    , velocity(0.0f, 0.0f, 0.0f)
    , lookDir(0.0f, 1.0f, -10.0f)
{
    application::Window &window = context.get<application::Window>();
    application::Window::MousePosition mousePos = window.getMousePosition();
    prevMousePos = glm::dvec2(mousePos.x, mousePos.y);

    window.setMouseVisible(false);
}

void CameraFlyController::tick(game::TickerContext &tickerContext) {
    application::Window &window = context.get<application::Window>();
    application::Window::Dimensions windowSize = window.dimensions;

    static bool prevKeyEsc = false;
    bool keyEsc = window.isKeyPressed(GLFW_KEY_ESCAPE);
    if (keyEsc && !prevKeyEsc) {
        isEnabled = !isEnabled;
        window.setMouseVisible(!isEnabled);
    }
    prevKeyEsc = keyEsc;

    if (windowSize.width && windowSize.height) {
        application::Window::MousePosition mousePos = window.getMousePosition();
        glm::dvec2 newMousePos(mousePos.x / windowSize.width, mousePos.y / windowSize.height);

        glm::vec2 delta = (newMousePos - prevMousePos) * 8.0;
        prevMousePos = newMousePos;

        if (!isEnabled) {
            return;
        }

        lookDir = glm::rotate(lookDir, -delta.x, render::Camera::up);
        lookDir = glm::rotate(lookDir, delta.y, glm::cross(render::Camera::up, lookDir));
        lookDir = glm::normalize(lookDir);
    }

    if (window.isKeyPressed(GLFW_KEY_W)) {
        velocity += lookDir;
    }
    if (window.isKeyPressed(GLFW_KEY_S)) {
        velocity -= lookDir;
    }
    if (window.isKeyPressed(GLFW_KEY_A)) {
        velocity += glm::cross(render::Camera::up, lookDir);
    }
    if (window.isKeyPressed(GLFW_KEY_D)) {
        velocity -= glm::cross(render::Camera::up, lookDir);
    }

    if (window.isKeyPressed(GLFW_KEY_SPACE)) {
        if (window.isKeyPressed(GLFW_KEY_LEFT_SHIFT) || window.isKeyPressed(GLFW_KEY_RIGHT_SHIFT)) {
            velocity -= render::Camera::up;
        } else {
            velocity += render::Camera::up;
        }
    }

    if (window.isKeyPressed(GLFW_KEY_P)) {
        velocity = glm::vec3(0.0f);
    }

    velocity *= 0.97f;

    static bool prevKeyG = false;
    bool keyG = window.isKeyPressed(GLFW_KEY_G);
    if (keyG && !prevKeyG) {
        enableGravity = !enableGravity;
    }
    prevKeyG = keyG;

    if (enableGravity) {
        velocity -= render::Camera::up * 0.5f;
    }

    position += velocity * 0.01f;

    float floor = -4.0f;
    if (position.z < floor && velocity.z < 0.0f) {
        position.z = floor;
        velocity.z = 0.0f;
    }

    context.get<render::Camera>().setView(position, lookDir);
}

}
