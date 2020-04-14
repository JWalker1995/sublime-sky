#pragma once

#include "game/gamecontext.h"

#include "render/program/program.h"
#include "render/scenemanager.h"

namespace render {

class PointCloudProgram : public Program {
public:
    class NoRenderConfigException : public jw_util::BaseException {
        friend class PointCloudProgram;

    private:
        NoRenderConfigException(const std::string &msg)
            : BaseException(msg)
        {}
    };

    PointCloudProgram(game::GameContext &context);

    virtual void insertDefines(Defines &defines);
    virtual void setupProgram(const Defines &defines);
    virtual void linkProgram();

    virtual void draw();

private:
    GLint eyePosLocation;
    GLint eyeDirLocation;

    class Vao : public graphics::GlVao {
    public:
        Vao(game::GameContext &context) {
            bind();

            SceneManager &sceneManager = context.get<SceneManager>();
            sceneManager.getMeshBuffer().setupVao(*this);
            sceneManager.getMaterialBuffer().setupVao(*this);
            sceneManager.getPointBuffer().setupVao(*this);

            unbind();
        }
    };

    jw_util::Signal<SceneManager::MeshBuffer &>::Listener meshBufferNewVboListener;
    jw_util::Signal<SceneManager::MaterialBuffer &>::Listener materialBufferNewVboListener;

    void onNewMeshBufferVbo(SceneManager::MeshBuffer &meshBuffer);
    void onNewMaterialBufferVbo(SceneManager::MaterialBuffer &materialBuffer);
};

}
