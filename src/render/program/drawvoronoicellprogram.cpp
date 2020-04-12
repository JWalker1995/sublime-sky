#include "drawvoronoicellprogram.h"

#include "spdlog/logger.h"

#include "graphics/glbufferbase.h"
#include "render/shaders.h"
#include "schemas/config_client_generated.h"
#include "graphics/imgui.h"
#include "spatial/uintcoord.h"
#include "render/camera.h"

namespace render {

DrawVoronoiCellProgram::DrawVoronoiCellProgram(game::GameContext &context)
    : Program(context)
    , meshBufferNewVboListener(
          context.get<SceneManager>().getMeshBuffer().onNewVbo,
          jw_util::MethodCallback<SceneManager::MeshBuffer &>::create<DrawVoronoiCellProgram, &DrawVoronoiCellProgram::onNewMeshBufferVbo>(this))
    , materialBufferNewVboListener(
          context.get<SceneManager>().getMaterialBuffer().onNewVbo,
          jw_util::MethodCallback<SceneManager::MaterialBuffer &>::create<DrawVoronoiCellProgram, &DrawVoronoiCellProgram::onNewMaterialBufferVbo>(this))
{}

void DrawVoronoiCellProgram::insertDefines(Defines &defines) {
    Program::insertDefines(defines);

    const SsProtocol::Config::Render *render = context.get<const SsProtocol::Config::Client>().render();
    if (!render) {
        throw NoRenderConfigException("Trying to create a DrawProgram but there's no render config!");
    }

    defines.set("USE_LOG_DEPTH_BUFFER", render->use_log_depth_buffer());
    defines.set("UINT_COORD_OFFSET", spatial::UintCoord::center);

    float pointSizeRange[2];
    glGetFloatv(GL_POINT_SIZE_RANGE, pointSizeRange);
    defines.set("POINT_SIZE_MAX", pointSizeRange[1]);

    context.get<Vao>().insertDefines(defines);
}

void DrawVoronoiCellProgram::setupProgram(const Defines &defines) {
    Program::setupProgram(defines);

    context.get<spdlog::logger>().debug("Compiling voronoi cell vertex shader");
    std::string vertShaderStr = std::string(Shaders::voronoiCellVert);
    attachShader(GL_VERTEX_SHADER, std::move(vertShaderStr), defines);

    context.get<spdlog::logger>().debug("Compiling voronoi cell fragment shader");
    std::string fragShaderStr = std::string(Shaders::voronoiCellFrag);
    attachShader(GL_FRAGMENT_SHADER, std::move(fragShaderStr), defines);
}

void DrawVoronoiCellProgram::linkProgram() {
    Program::linkProgram();

//    showTrianglesLocation = glGetUniformLocation(getProgramId(), "showTriangles");
//    graphics::GL::catchErrors();

    int a;
    glGetProgramiv(getProgramId(), GL_DELETE_STATUS, &a);
    glGetProgramiv(getProgramId(), GL_LINK_STATUS, &a);
    glGetProgramiv(getProgramId(), GL_VALIDATE_STATUS, &a);
    glGetProgramiv(getProgramId(), GL_INFO_LOG_LENGTH, &a);
    glGetProgramiv(getProgramId(), GL_ATTACHED_SHADERS, &a);
    glGetProgramiv(getProgramId(), GL_ACTIVE_ATTRIBUTES, &a);
    glGetProgramiv(getProgramId(), GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &a);
    glGetProgramiv(getProgramId(), GL_ACTIVE_UNIFORMS, &a);
    glGetProgramiv(getProgramId(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &a);


    GLint max_length = 0;
    glGetProgramiv(getProgramId(), GL_INFO_LOG_LENGTH, &max_length);

    GLchar *error_log = new GLchar[static_cast<std::size_t>(max_length)];
    glGetProgramInfoLog(getProgramId(), max_length, &max_length, error_log);

    context.get<spdlog::logger>().error("Could not compile GLSL shader:");
    context.get<spdlog::logger>().error(std::string(error_log, static_cast<std::size_t>(max_length)));


    eyePosLocation = glGetUniformLocation(getProgramId(), "eyePos");
    assert(eyePosLocation != -1);
    graphics::GL::catchErrors();
}

void DrawVoronoiCellProgram::draw() {
    assertLinked();

    Vao &vao = context.get<Vao>();
    vao.bind();

    SceneManager &sceneManager = context.get<SceneManager>();
    sceneManager.getMeshBuffer().sync(vao);
    sceneManager.getMaterialBuffer().sync(vao);
    sceneManager.getVoronoiCellBuffer().sync(vao);

    if (ImGui::Begin("Debug")) {
//        if (ImGui::Button("Toggle Triangles")) {
//            showTrianglesValue = !showTrianglesValue;
//            showTrianglesDirty = true;
//        }
    }
    ImGui::End();

//    if (showTrianglesDirty) {
//        glUniform1i(showTrianglesLocation, showTrianglesValue);
//        graphics::GL::catchErrors();

//        showTrianglesDirty = false;
//    }

    glm::vec3 eyePos = context.get<render::Camera>().getEyePos();
    glUniform3f(eyePosLocation, eyePos.x, eyePos.y, eyePos.z);

    glDrawArrays(GL_POINTS, 0, sceneManager.getVoronoiCellBuffer().getExtentSize());
    graphics::GL::catchErrors();

    vao.unbind();
}

void DrawVoronoiCellProgram::onNewMeshBufferVbo(SceneManager::MeshBuffer &meshBuffer) {
    assertLinked();

    meshBuffer.getGlBuffer().bind();
    bindUniformBlock("MeshesBlock", meshBuffer.getGlBuffer().get_base_binding());
}

void DrawVoronoiCellProgram::onNewMaterialBufferVbo(SceneManager::MaterialBuffer &materialBuffer) {
    assertLinked();

    materialBuffer.getGlBuffer().bind();
    bindUniformBlock("MaterialsBlock", materialBuffer.getGlBuffer().get_base_binding());
}

}
