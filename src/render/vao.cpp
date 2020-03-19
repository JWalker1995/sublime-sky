#include "vao.h"

#include "render/scenemanager.h"

namespace render {

Vao::Vao(game::GameContext &context) {
    bind();

    SceneManager &sceneManager = context.get<SceneManager>();
    sceneManager.getMeshBuffer().setupVao(*this);
    sceneManager.getVertBuffer().setupVao(*this);
    sceneManager.getFaceBuffer().setupVao(*this);
}

}
