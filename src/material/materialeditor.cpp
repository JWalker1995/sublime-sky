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
        for (std::size_t i = 0; i < materials.getExtentSize(); i++) {
            render::SceneManager::MaterialMutator mat = materials.mutate(i);

            ImGui::ColorEdit4((mat.local.name + " ambient##" + std::to_string(i)).data(), mat.shared.colorAmbient, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
            ImGui::SameLine(35);
            ImGui::ColorEdit4((mat.local.name + " diffuse##" + std::to_string(i)).data(), mat.shared.colorDiffuse, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
            ImGui::SameLine(60);
            ImGui::ColorEdit4((mat.local.name + " specular##" + std::to_string(i)).data(), mat.shared.colorSpecular, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
            ImGui::SameLine(85);
            ImGui::PushItemWidth(100);
            ImGui::InputFloat(("##shininess_" + std::to_string(i)).data(), &mat.shared.shininess, 0.1f, 1.0f);
            ImGui::SameLine(190);
            if (mat.local.originalIndex == static_cast<unsigned int>(-1)) {
                ImGui::Text("%s", mat.local.name.data());
            } else {
                ImGui::Text("%s (%d)", mat.local.name.data(), mat.local.originalIndex);
            }
        }
    }
    ImGui::End();
}

}
