#include "camera.h"

#include "render/imguirenderer.h"
#include "graphics/imgui.h"

namespace render {

Camera::Camera(game::GameContext &context)
    : TickableBase(context)
{}

void Camera::tick(game::TickerContext &tickerContext) {
    tickerContext.get<ImguiRenderer::Ticker>();

    if (ImGui::Begin("Debug")) {
        ImGui::Text("Eye position = (%f, %f, %f)", getEyePos().x, getEyePos().y, getEyePos().z);
        ImGui::Text("Eye direction = (%f, %f, %f)", getEyeDir().x, getEyeDir().y, getEyeDir().z);
        ImGui::End();
    }
}

}
