#include "computeprogram.h"

#include "spdlog/logger.h"

#include "render/shaders.h"

namespace render {

ComputeProgram::ComputeProgram(game::GameContext &context)
    : Program(context)
{}

void ComputeProgram::insertDefines(Defines &defines) {
    Program::insertDefines(defines);

    // context.get<FaceFragCounter>().insertDefines(defines);

    /*
    defines.set("CURRENT_TRIGGER_BINDING", current_trigger_buffer_binding);
    defines.set("TRIGGERS_BINDING", triggers_buffer_binding);
    defines.set("OVERLAY_DATA_BINDING", overlay_data_binding);
    defines.set("NUM_FRAGMENTS_BINDING", num_fragments_binding);
    defines.set("DEBUG_BINDING", debug_binding);
    */
}

void ComputeProgram::setupProgram(const Defines &defines) {
    Program::setupProgram(defines);

    context.get<spdlog::logger>().debug("Compiling compute shader");
    std::string computeShaderStr = std::string(Shaders::fragCounterCompute);
    attachShader(GL_COMPUTE_SHADER, std::move(computeShaderStr), defines);
}

}
