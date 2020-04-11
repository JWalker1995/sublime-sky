#include "cameraflycontroller.h"

#include "graphics/glm.h"
#include <glm/gtx/norm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "graphics/imgui.h"
#include "game/gamecontext.h"
#include "application/window.h"
#include "render/camera.h"
#include "render/imguirenderer.h"

namespace game {

CameraFlyController::CameraFlyController(game::GameContext &context)
    : TickableBase(context)
    , position(1200.0f, 0.0f, 0.0f)
    , velocity(0.0f, 0.0f, 0.0f)
    , lookDir(-1.0f, 0.0f, 0.0f)
    , upDir(0.0f, 0.0f, 1.0f)
{
    application::Window &window = context.get<application::Window>();
    application::Window::MousePosition mousePos = window.getMousePosition();
    prevMousePos = glm::dvec2(mousePos.x, mousePos.y);

    window.setMouseVisible(false);
}

void CameraFlyController::tick(game::TickerContext &tickerContext) {
    tickerContext.get<render::ImguiRenderer::Ticker>();

    if (ImGui::Begin("Debug")) {
        ImGui::Text("Game mode:");
        ImGui::SameLine();
        ImGui::RadioButton("creative", &gameMode, static_cast<int>(GameMode::Creative));
        ImGui::SameLine();
        ImGui::RadioButton("spectator", &gameMode, static_cast<int>(GameMode::Spectator));
        ImGui::SameLine();
        ImGui::RadioButton("orbital", &gameMode, static_cast<int>(GameMode::Orbital));
    }
    ImGui::End();

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

        lookDir = glm::rotate(lookDir, -delta.x, upDir);
        lookDir = glm::rotate(lookDir, delta.y, glm::cross(upDir, lookDir));
        lookDir = glm::normalize(lookDir);
    }

    switch (gameMode) {
        case static_cast<int>(GameMode::Creative): tickModeCreative(); break;
        case static_cast<int>(GameMode::Spectator): tickModeSpectator(); break;
        case static_cast<int>(GameMode::Orbital): tickModeOrbital(); break;
    }

    lookDir = glm::normalize(lookDir);

    context.get<render::Camera>().setView(position, lookDir, upDir);
}

void CameraFlyController::tickModeCreative() {
    velocity -= upDir * 0.5f;
    tickPhysicalCamera();
    upDir = glm::normalize(position);
}

void CameraFlyController::tickModeSpectator() {
    tickPhysicalCamera();
    upDir = glm::normalize(position);
}

void CameraFlyController::tickModeOrbital() {
    static constexpr float rotationSpeed = 16.0f;

    velocity = glm::vec3(0.0f);
    upDir = glm::vec3(0.0f, 0.0f, 1.0f);

    application::Window &window = context.get<application::Window>();

    if (window.isKeyPressed(GLFW_KEY_W)) {
        position += upDir * rotationSpeed;
    }
    if (window.isKeyPressed(GLFW_KEY_S)) {
        position -= upDir * rotationSpeed;
    }
    if (window.isKeyPressed(GLFW_KEY_A)) {
        position += glm::cross(upDir, lookDir) * rotationSpeed;
    }
    if (window.isKeyPressed(GLFW_KEY_D)) {
        position -= glm::cross(upDir, lookDir) * rotationSpeed;
    }

    if (window.isKeyPressed(GLFW_KEY_SPACE)) {
        position *= 1.01f;
    } else if (window.isKeyPressed(GLFW_KEY_LEFT_SHIFT) || window.isKeyPressed(GLFW_KEY_RIGHT_SHIFT)) {
        position /= 1.01f;
    }

    lookDir = -glm::normalize(position);
}

void CameraFlyController::tickPhysicalCamera() {
    application::Window &window = context.get<application::Window>();

    if (window.isKeyPressed(GLFW_KEY_W)) {
        velocity += lookDir;
    }
    if (window.isKeyPressed(GLFW_KEY_S)) {
        velocity -= lookDir;
    }
    if (window.isKeyPressed(GLFW_KEY_A)) {
        velocity += glm::cross(upDir, lookDir);
    }
    if (window.isKeyPressed(GLFW_KEY_D)) {
        velocity -= glm::cross(upDir, lookDir);
    }

    if (window.isKeyPressed(GLFW_KEY_SPACE)) {
        velocity += upDir;
    } else if (window.isKeyPressed(GLFW_KEY_LEFT_SHIFT) || window.isKeyPressed(GLFW_KEY_RIGHT_SHIFT)) {
        velocity -= upDir;
    }

    if (window.isKeyPressed(GLFW_KEY_P)) {
        velocity = glm::vec3(0.0f);
    }

    velocity *= 0.97f;

    position += velocity * 0.1f;
}

}
