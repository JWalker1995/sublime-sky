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

    unsigned int firstIndex = static_cast<unsigned int>(-1);
    unsigned int nextIndex;

    for (unsigned int i = 0; i < newMaterials->size(); i++) {
        const SsProtocol::Material *inMat = newMaterials->Get(i);

        render::SceneManager::MaterialMutator material = sceneManager.createMaterial();
        if (i == 0) {
            firstIndex = material.index;
            nextIndex = material.index;
        }

        assert(material.index == nextIndex);
        nextIndex++;

        material.local.phase = [](SsProtocol::MaterialPhase phase) {
            switch (phase) {
                case SsProtocol::MaterialPhase_Solid: return graphics::MaterialLocal::Phase::Solid;
                case SsProtocol::MaterialPhase_Liquid: return graphics::MaterialLocal::Phase::Liquid;
                case SsProtocol::MaterialPhase_Gas: return graphics::MaterialLocal::Phase::Gas;
            }
        }(inMat->phase());
        material.shared.renderModel = [](SsProtocol::MaterialRenderModel model) {
            switch (model) {
                case SsProtocol::MaterialRenderModel_Invisible: return graphics::MaterialShared::RenderModel::Invisible;
                case SsProtocol::MaterialRenderModel_Phong: return graphics::MaterialShared::RenderModel::Phong;
                case SsProtocol::MaterialRenderModel_Blinn: return graphics::MaterialShared::RenderModel::Blinn;
            }
        }(inMat->render_model());

        material.local.name = inMat->name()->str();
        material.local.originalIndex = i;
        material.local.mass = inMat->mass();

        material.shared.colorDiffuse[0] = inMat->color_diffuse()->x();
        material.shared.colorDiffuse[1] = inMat->color_diffuse()->y();
        material.shared.colorDiffuse[2] = inMat->color_diffuse()->z();
        material.shared.colorDiffuse[3] = inMat->color_diffuse()->w();

        material.shared.colorSpecular[0] = inMat->color_specular()->x();
        material.shared.colorSpecular[1] = inMat->color_specular()->y();
        material.shared.colorSpecular[2] = inMat->color_specular()->z();
        material.shared.colorSpecular[3] = inMat->color_specular()->w();

        material.shared.shininess = inMat->shininess();
    }

    return firstIndex;
}

}
