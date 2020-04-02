#include "indexconnector.h"

#include "graphics/imgui.h"

#include "game/gamecontext.h"
#include "render/imguirenderer.h"

namespace util {

IndexConnector::IndexConnector(game::GameContext &context)
    : TickableBase(context)
{}

void IndexConnector::tick(game::TickerContext &tickerContext) {
    tickerContext.get<render::ImguiRenderer::Ticker>();

    if (ImGui::Begin("Debug")) {
        unsigned int ids = registry.size();
        unsigned int waiting = 0;
        unsigned int pending = 0;

        std::unordered_map<Key, Value, Key::Hasher>::const_iterator i = registry.cbegin();
        while (i != registry.cend()) {
            assert((!i->second.data) ^ (!!i->second.callbacks.empty()));
            waiting += !i->second.callbacks.empty();
            pending += i->second.callbacks.size();
            i++;
        }

        ImGui::Text("IndexConnector: ids=%d, waiting=%d, pending=%d", ids, waiting, pending);
    }
    ImGui::End();
}

}
