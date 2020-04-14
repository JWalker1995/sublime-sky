#include "pixelmatchfetcherprogram.h"

#include "spdlog/logger.h"

#include "graphics/glbufferbase.h"
#include "render/shaders.h"
#include "schemas/config_client_generated.h"
#include "graphics/imgui.h"
#include "spatial/uintcoord.h"
#include "render/camera.h"
#include "application/window.h"

namespace render {

PixelMatchFetcherProgram::PixelMatchFetcherProgram(game::GameContext &context)
    : Program(context)
{}

void PixelMatchFetcherProgram::insertDefines(Defines &defines) {
    Program::insertDefines(defines);

    context.get<Vao>().insertDefines(defines);
}

void PixelMatchFetcherProgram::setupProgram(const Defines &defines) {
    Program::setupProgram(defines);

    context.get<spdlog::logger>().debug("Compiling pixel match fetcher vertex shader");
    std::string vertShaderStr = std::string(Shaders::pixelMatchFetcherVert);
    attachShader(GL_VERTEX_SHADER, std::move(vertShaderStr), defines);

    context.get<spdlog::logger>().debug("Compiling pixel match fetcher fragment shader");
    std::string fragShaderStr = std::string(Shaders::pixelMatchFetcherFrag);
    attachShader(GL_FRAGMENT_SHADER, std::move(fragShaderStr), defines);
}

void PixelMatchFetcherProgram::linkProgram() {
    Program::linkProgram();
}

void PixelMatchFetcherProgram::draw() {
    Program::bind();

    assertLinked();

    Vao &vao = context.get<Vao>();
    vao.bind();

    application::Window::Dimensions dims = context.get<application::Window>().dimensions;

    unsigned int size = dims.width * dims.height;
    vao.ensureTransferBufferSize(size);
    glReadPixels(0, 0, dims.width, dims.height, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
    graphics::GL::catchErrors();

    glPointSize(1.0);
    graphics::GL::catchErrors();

    glDepthMask(GL_FALSE);
    glDepthFunc(GL_ALWAYS);
    glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
    graphics::GL::catchErrors();

    glDrawArrays(GL_POINTS, 0, size / 16);
    graphics::GL::catchErrors();

    vao.unbind();
}

}
