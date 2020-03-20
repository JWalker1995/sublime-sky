#pragma once

#include "game/gamecontext.h"

#include "render/program/program.h"
#include "render/scenemanager.h"

namespace render {

class DrawProgram : public Program {
public:
    DrawProgram(game::GameContext &context);

    virtual void insertDefines(Defines &defines);
    virtual void setupProgram(const Defines &defines);
    virtual void linkProgram();

private:
    jw_util::Signal<SceneManager::MeshBuffer &>::Listener meshBufferNewVboListener;

    void onNewMeshBufferVbo(SceneManager::MeshBuffer &meshBuffer);
};

}
