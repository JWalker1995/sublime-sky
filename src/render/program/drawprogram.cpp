#include "drawprogram.h"

#include <spdlog/logger.h>

#include "graphics/glbufferbase.h"
#include "render/shaders.h"
#include "render/vao.h"

namespace render {

DrawProgram::DrawProgram(game::GameContext &context)
    : Program(context)
    , meshBufferNewVboListener(
          context.get<SceneManager>().getMeshBuffer().onNewVbo,
          jw_util::MethodCallback<SceneManager::MeshBuffer &>::create<DrawProgram, &DrawProgram::onNewMeshBufferVbo>(this))
{}

void DrawProgram::insertDefines(Defines &defines) {
    Program::insertDefines(defines);

    defines.set("USE_LOG_DEPTH_BUFFER", 1);

    context.get<Vao>().insertDefines(defines);

    /*
    defines.set("CURRENT_TRIGGER_BINDING", current_trigger_buffer_binding);
    defines.set("TRIGGERS_BINDING", triggers_buffer_binding);
    defines.set("OVERLAY_DATA_BINDING", overlay_data_binding);
    defines.set("NUM_FRAGMENTS_BINDING", num_fragments_binding);
    defines.set("DEBUG_BINDING", debug_binding);
    */
}

void DrawProgram::linkProgram() {
    Program::linkProgram();
}

void DrawProgram::setupProgram(const Defines &defines) {
    Program::setupProgram(defines);

    context.get<spdlog::logger>().debug("Compiling vertex shader");
    std::string vertShaderStr = std::string(Shaders::mainVert);
    attachShader(GL_VERTEX_SHADER, std::move(vertShaderStr), defines);

    context.get<spdlog::logger>().debug("Compiling fragment shader");
    std::string fragShaderStr = std::string(Shaders::mainFrag);
    attachShader(GL_FRAGMENT_SHADER, std::move(fragShaderStr), defines);
}

void DrawProgram::onNewMeshBufferVbo(SceneManager::MeshBuffer &meshBuffer) {
    assertLinked();

    meshBuffer.getGlBuffer().bind_base();
    bindUniformBlock("meshes", meshBuffer.getGlBuffer().get_base_binding());
}

}
