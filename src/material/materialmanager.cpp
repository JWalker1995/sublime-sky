#include "materialmanager.h"

#include "schemas/message_generated.h"
#include "render/scenemanager.h"

namespace material {

MaterialManager::MaterialManager(game::GameContext &context)
    : TickableBase(context)
{}

void MaterialManager::tick(game::TickerContext &tickerContext) {
    (void) tickerContext;
}

unsigned int MaterialManager::registerMaterials(const flatbuffers::Vector<flatbuffers::Offset<SsProtocol::Material>> *newMaterials) {
    render::SceneManager &sceneManager = context.get<render::SceneManager>();

    unsigned int firstIndex;
    unsigned int nextIndex;

    auto it = newMaterials->cbegin();
    while (it != newMaterials->cend()) {
        render::SceneManager::MaterialMutator material = sceneManager.createMaterial();
        if (it == newMaterials->cbegin()) {
            firstIndex = material.index;
            nextIndex = material.index;
        }

        assert(material.index == nextIndex);
        nextIndex++;

        material.local.phase = [](SsProtocol::Phase phase) {
            switch (phase) {
                case SsProtocol::Phase_Solid: return graphics::MaterialLocal::Phase::Solid;
                case SsProtocol::Phase_Liquid: return graphics::MaterialLocal::Phase::Liquid;
                case SsProtocol::Phase_Gas: return graphics::MaterialLocal::Phase::Gas;
            }
        }(it->phase());
        material.shared.renderModel = [](SsProtocol::MaterialRenderModel model) {
            switch (model) {
                case SsProtocol::MaterialRenderModel_Invisible: return graphics::MaterialShared::RenderModel::Invisible;
                case SsProtocol::MaterialRenderModel_Phong: return graphics::MaterialShared::RenderModel::Phong;
                case SsProtocol::MaterialRenderModel_Blinn: return graphics::MaterialShared::RenderModel::Blinn;
            }
        }(it->render_model());

        material.local.name = it->name()->str();
        material.local.mass = it->mass();

        material.shared.colorDiffuse[0] = it->color_diffuse()->x();
        material.shared.colorDiffuse[1] = it->color_diffuse()->y();
        material.shared.colorDiffuse[2] = it->color_diffuse()->z();
        material.shared.colorDiffuse[3] = 1.0f;

        material.shared.colorSpecular[0] = it->color_specular()->x();
        material.shared.colorSpecular[1] = it->color_specular()->y();
        material.shared.colorSpecular[2] = it->color_specular()->z();
        material.shared.colorSpecular[3] = 1.0f;

        material.shared.shininess = it->shininess();
    }

    return firstIndex;
}

}
