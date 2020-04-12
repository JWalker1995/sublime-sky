#pragma once

#include "game/gamecontext.h"

#include "render/program/program.h"
#include "render/scenemanager.h"

namespace render {

class MeshProgram : public Program {
public:
    class NoRenderConfigException : public jw_util::BaseException {
        friend class MeshProgram;

    private:
        NoRenderConfigException(const std::string &msg)
            : BaseException(msg)
        {}
    };

    MeshProgram(game::GameContext &context);

    virtual void insertDefines(Defines &defines);
    virtual void setupProgram(const Defines &defines);
    virtual void linkProgram();

    void bind();
    void unbind();

private:
    class Vao : public graphics::GlVao {
    public:
        Vao(game::GameContext &context) {
            bind();

            SceneManager &sceneManager = context.get<SceneManager>();
            sceneManager.getMeshBuffer().setupVao(*this);
            sceneManager.getVertBuffer().setupVao(*this);
            sceneManager.getFaceBuffer().setupVao(*this);
            sceneManager.getMaterialBuffer().setupVao(*this);

            unbind();
        }
    };

    jw_util::Signal<SceneManager::MeshBuffer &>::Listener meshBufferNewVboListener;
    jw_util::Signal<SceneManager::MaterialBuffer &>::Listener materialBufferNewVboListener;

    void onNewMeshBufferVbo(SceneManager::MeshBuffer &meshBuffer);
    void onNewMaterialBufferVbo(SceneManager::MaterialBuffer &materialBuffer);
};

}
