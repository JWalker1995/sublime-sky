#include "debuginfo.h"

#include "graphics/imgui.h"

#include "game/gamecontext.h"
#include "render/imguirenderer.h"

namespace game {

DebugInfo::DebugInfo(game::GameContext &context)
    : TickableBase(context)
{}

void DebugInfo::tick(TickerContext &tickerContext) {
    tickerContext.get<render::ImguiRenderer::Ticker>();

    if (ImGui::Begin("Debug")) {
        ImGui::Text("Counter: %d", counter);
    }
    ImGui::End();
}

}
