#include "materialeditor.h"

#include "graphics/imgui.h"
#include "render/imguirenderer.h"
#include "render/scenemanager.h"

namespace material {

MaterialEditor::MaterialEditor(game::GameContext &context)
    : TickableBase(context)
{}

void MaterialEditor::tick(game::TickerContext &tickerContext) {
    tickerContext.get<render::ImguiRenderer::Ticker>();

    render::SceneManager::MaterialBuffer &materials = context.get<render::SceneManager>().getMaterialBuffer();
    assert(materials.getActiveSize() == materials.getExtentSize());

    if (ImGui::Begin("Materials")) {
        for (unsigned int i = 0; i < materials.getExtentSize(); i++) {
            render::SceneManager::MaterialMutator mat = materials.mutate(i);
            ImGui::Text("%s", mat.local.name.data());
            ImGui::ColorEdit4("My color", mat.shared.colorDiffuse);
            // ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel
        }
//        HelpMarker("With the ImGuiColorEditFlags_NoInputs flag you can hide all the slider/text inputs.\nWith the ImGuiColorEditFlags_NoLabel flag you can pass a non-empty label which will only be used for the tooltip and picker popup.");

        ImGui::End();
    }
}

}
