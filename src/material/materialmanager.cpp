#include "materialmanager.h"

#include "schemas/message_generated.h"
#include "render/scenemanager.h"
#include "world/materialindex.h"

namespace material {

MaterialManager::MaterialManager(game::GameContext &context)
    : TickableBase(context)
{
    createGeneratingMaterial();
}

void MaterialManager::tick(game::TickerContext &tickerContext) {
    (void) tickerContext;
}

std::vector<unsigned int> MaterialManager::registerMaterials(const flatbuffers::Vector<flatbuffers::Offset<SsProtocol::Material>> *newMaterials) {
    render::SceneManager &sceneManager = context.get<render::SceneManager>();

    std::vector<unsigned int> mapping;
    for (unsigned int i = 0; i < newMaterials->size(); i++) {
        const SsProtocol::Material *inMat = newMaterials->Get(i);

        if (inMat->flags() & SsProtocol::MaterialFlags::MaterialFlags_Regenerate) {
            mapping.push_back(static_cast<unsigned int>(world::MaterialIndex::Null));
            continue;
        }

        render::SceneManager::MaterialMutator material = sceneManager.createMaterial();
        mapping.push_back(material.index);

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

        material.shared.colorAmbient[0] = inMat->color_ambient()->x();
        material.shared.colorAmbient[1] = inMat->color_ambient()->y();
        material.shared.colorAmbient[2] = inMat->color_ambient()->z();
        material.shared.colorAmbient[3] = inMat->color_ambient()->w();

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

    assert(mapping.size() == newMaterials->size());
    return mapping;
}

void MaterialManager::createGeneratingMaterial() {
    // Allocate dummy material that shouldn't be set on any vertices, but will signify that a cell is generating.
    // This material should never be seen.
    render::SceneManager::MaterialMutator generatingMaterial = context.get<render::SceneManager>().createMaterial();

    // It should match world::MaterialIndex::Generating
    assert(generatingMaterial.index == static_cast<unsigned int>(world::MaterialIndex::Generating));

    // Let's make sure we can see them (so we know if we have a bug)
    generatingMaterial.shared.renderModel = graphics::MaterialShared::RenderModel::Blinn;
    generatingMaterial.shared.colorAmbient[0] = 1.0f;
    generatingMaterial.shared.colorAmbient[1] = 0.0f;
    generatingMaterial.shared.colorAmbient[2] = 0.0f;
    generatingMaterial.shared.colorAmbient[3] = 1.0f;
    generatingMaterial.shared.colorDiffuse[0] = 1.0f;
    generatingMaterial.shared.colorDiffuse[1] = 0.0f;
    generatingMaterial.shared.colorDiffuse[2] = 0.0f;
    generatingMaterial.shared.colorDiffuse[3] = 1.0f;
    generatingMaterial.shared.colorSpecular[0] = 1.0f;
    generatingMaterial.shared.colorSpecular[1] = 0.0f;
    generatingMaterial.shared.colorSpecular[2] = 0.0f;
    generatingMaterial.shared.colorSpecular[3] = 1.0f;
    generatingMaterial.shared.shininess = 1.0f;

    generatingMaterial.local.name = "_ss_generating";
    generatingMaterial.local.phase = graphics::MaterialLocal::Phase::Unknown;
    generatingMaterial.local.mass = 1.0f;
}

}
