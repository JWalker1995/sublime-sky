#include "meshprogram.h"

#include "spdlog/logger.h"

#include "graphics/glbufferbase.h"
#include "render/shaders.h"
#include "schemas/config_client_generated.h"

namespace render {

MeshProgram::MeshProgram(game::GameContext &context)
    : Program(context)
    , meshBufferNewVboListener(
          context.get<SceneManager>().getMeshBuffer().onNewVbo,
          jw_util::MethodCallback<SceneManager::MeshBuffer &>::create<MeshProgram, &MeshProgram::onNewMeshBufferVbo>(this))
    , materialBufferNewVboListener(
          context.get<SceneManager>().getMaterialBuffer().onNewVbo,
          jw_util::MethodCallback<SceneManager::MaterialBuffer &>::create<MeshProgram, &MeshProgram::onNewMaterialBufferVbo>(this))
{}

void MeshProgram::insertDefines(Defines &defines) {
    Program::insertDefines(defines);

    const SsProtocol::Config::Render *render = context.get<const SsProtocol::Config::Client>().render();
    if (!render) {
        throw NoRenderConfigException("Trying to create a DrawProgram but there's no render config!");
    }

    defines.set("USE_LOG_DEPTH_BUFFER", render->use_log_depth_buffer());

    context.get<Vao>().insertDefines(defines);

    /*
    defines.set("CURRENT_TRIGGER_BINDING", current_trigger_buffer_binding);
    defines.set("TRIGGERS_BINDING", triggers_buffer_binding);
    defines.set("OVERLAY_DATA_BINDING", overlay_data_binding);
    defines.set("NUM_FRAGMENTS_BINDING", num_fragments_binding);
    defines.set("DEBUG_BINDING", debug_binding);
    */
}

void MeshProgram::setupProgram(const Defines &defines) {
    Program::setupProgram(defines);

    context.get<spdlog::logger>().debug("Compiling main vertex shader");
    std::string vertShaderStr = std::string(Shaders::mainVert);
    attachShader(GL_VERTEX_SHADER, std::move(vertShaderStr), defines);

    context.get<spdlog::logger>().debug("Compiling main fragment shader");
    std::string fragShaderStr = std::string(Shaders::mainFrag);
    attachShader(GL_FRAGMENT_SHADER, std::move(fragShaderStr), defines);
}

void MeshProgram::linkProgram() {
    Program::linkProgram();
}

void MeshProgram::bind() {
    assertLinked();

    Vao &vao = context.get<Vao>();
    vao.bind();

    SceneManager &sceneManager = context.get<SceneManager>();
    sceneManager.getMeshBuffer().sync(vao);
    sceneManager.getVertBuffer().sync(vao);
    sceneManager.getFaceBuffer().sync(vao);
    sceneManager.getMaterialBuffer().sync(vao);
}

void MeshProgram::unbind() {
    Vao &vao = context.get<Vao>();
    vao.unbind();
}

void MeshProgram::onNewMeshBufferVbo(SceneManager::MeshBuffer &meshBuffer) {
    assertLinked();

    meshBuffer.getGlBuffer().bind();
    bindUniformBlock("MeshesBlock", meshBuffer.getGlBuffer().get_base_binding());
}

void MeshProgram::onNewMaterialBufferVbo(SceneManager::MaterialBuffer &materialBuffer) {
    assertLinked();

    materialBuffer.getGlBuffer().bind();
    bindUniformBlock("MaterialsBlock", materialBuffer.getGlBuffer().get_base_binding());
}

}
