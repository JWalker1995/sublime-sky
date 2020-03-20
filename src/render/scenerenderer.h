#pragma once

#include <deque>

#include "game/tickercontext.h"
#include "render/scenemanager.h"
#include "render/camera.h"

namespace render {

class SceneRenderer : public game::TickerContext::TickableBase<SceneRenderer, game::TickerContext::ScopedCaller<SceneRenderer>> {
public:
    SceneRenderer(game::GameContext &context);

    void tickOpen(game::TickerContext &tickerContext);
    void tickClose(game::TickerContext &tickerContext);

    void setMeshTransform(SceneManager::MeshMutator meshMutator, const geometry::Transform &transform) {
        meshMutator.shared.transform = context.get<Camera>().getTransform() * transform.toMat4x4();
    }
};

}
