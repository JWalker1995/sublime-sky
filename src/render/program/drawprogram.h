#pragma once

#include "game/gamecontext.h"

#include "render/program/program.h"
#include "render/scenemanager.h"

namespace render {

class DrawProgram : public Program {
public:
    class NoRenderConfigException : public jw_util::BaseException {
        friend class DrawProgram;

    private:
        NoRenderConfigException(const std::string &msg)
            : BaseException(msg)
        {}
    };

    DrawProgram(game::GameContext &context);

    virtual void insertDefines(Defines &defines);
    virtual void setupProgram(const Defines &defines);
    virtual void linkProgram();

private:
    jw_util::Signal<SceneManager::MeshBuffer &>::Listener meshBufferNewVboListener;
    jw_util::Signal<SceneManager::MaterialBuffer &>::Listener materialBufferNewVboListener;

    void onNewMeshBufferVbo(SceneManager::MeshBuffer &meshBuffer);
    void onNewMaterialBufferVbo(SceneManager::MaterialBuffer &materialBuffer);
};

}
