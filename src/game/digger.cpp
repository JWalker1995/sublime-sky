#include "digger.h"

#include "application/window.h"
#include "graphics/imgui.h"
#include "render/camera.h"
#include "world/hashtreeworld.h"
#include "render/meshupdater.h"

namespace game {

Digger::Digger(game::GameContext &context)
    : TickableBase(context)
{}

void Digger::tick(game::TickerContext &tickerContext) {
    application::Window &window = context.get<application::Window>();
    application::Window::Dimensions windowSize = window.dimensions;

    // Why does this make everything work???
    windowSize.width /= 2;
    windowSize.height /= 2;

    ImVec2 a(windowSize.width / 2.0f - 10.0f, windowSize.height / 2.0f);
    ImVec2 b(windowSize.width / 2.0f + 10.0f, windowSize.height / 2.0f);
    ImVec2 c(windowSize.width / 2.0f, windowSize.height / 2.0f - 10.0f);
    ImVec2 d(windowSize.width / 2.0f, windowSize.height / 2.0f + 10.0f);
    ImGui::GetForegroundDrawList()->AddLine(a, b, 0xFFFFFFFF, 2.0f);
    ImGui::GetForegroundDrawList()->AddLine(c, d, 0xFFFFFFFF, 2.0f);

    return;

    render::Camera &camera = context.get<render::Camera>();
    world::HashTreeWorld &hashTreeWorld = context.get<world::HashTreeWorld>();
    render::MeshUpdater &meshUpdater = context.get<render::MeshUpdater>();

    if (window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
        spatial::UintCoord selectedCell = hashTreeWorld.testRay(camera.getEyePos(), camera.getEyeDir(), 100.0f).hitCoord;
        hashTreeWorld.getSpaceStateMutable(selectedCell) = world::SpaceState::Air;

        spatial::UintCoord min = selectedCell - spatial::UintCoord(2);
        spatial::UintCoord max = selectedCell + spatial::UintCoord(2);
        spatial::UintCoord neighborCoord;
        for (neighborCoord.x = min.x; neighborCoord.x <= max.x; neighborCoord.x++) {
            for (neighborCoord.y = min.y; neighborCoord.y <= max.y; neighborCoord.y++) {
                for (neighborCoord.z = min.z; neighborCoord.z <= max.z; neighborCoord.z++) {
                    hashTreeWorld.getNeedsRegen(neighborCoord) = true;
                    meshUpdater.updateCell<true>(neighborCoord);
                }
            }
        }
    }
}

}
