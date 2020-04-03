#include "scenemanager.h"

namespace render {

SceneManager::SceneManager(game::GameContext &context) {
    (void) context;

    createNullMaterial();
}

void SceneManager::sync(graphics::GlVao &vao) {
    getMeshBuffer().sync(vao);
    getVertBuffer().sync(vao);
    getFaceBuffer().sync(vao);
    getMaterialBuffer().sync(vao);
}

void SceneManager::createNullMaterial() {
    // Allocate dummy material that will be set on vertices that aren't provoking for any face.
    // This material should never be seen.
    MaterialMutator nullMaterial = createMaterial();

    // This should match the default value for graphics::VertShared::materialIndex
    assert(nullMaterial.index == 0);

    // Let's make sure we can see them (so we know if we have a bug)
    nullMaterial.shared.renderModel = graphics::MaterialShared::RenderModel::Blinn;
    nullMaterial.shared.colorAmbient[0] = 1.0f;
    nullMaterial.shared.colorAmbient[1] = 0.0f;
    nullMaterial.shared.colorAmbient[2] = 0.0f;
    nullMaterial.shared.colorAmbient[3] = 1.0f;
    nullMaterial.shared.colorDiffuse[0] = 1.0f;
    nullMaterial.shared.colorDiffuse[1] = 0.0f;
    nullMaterial.shared.colorDiffuse[2] = 0.0f;
    nullMaterial.shared.colorDiffuse[3] = 1.0f;
    nullMaterial.shared.colorSpecular[0] = 1.0f;
    nullMaterial.shared.colorSpecular[1] = 0.0f;
    nullMaterial.shared.colorSpecular[2] = 0.0f;
    nullMaterial.shared.colorSpecular[3] = 1.0f;
    nullMaterial.shared.shininess = 1.0f;

    nullMaterial.local.name = "Null";
    nullMaterial.local.phase = graphics::MaterialLocal::Phase::Solid;
    nullMaterial.local.mass = 1.0f;
}

}
