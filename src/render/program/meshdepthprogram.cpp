#include "meshdepthprogram.h"

namespace render {

MeshDepthProgram::MeshDepthProgram(game::GameContext &context)
    : MeshProgram(context)
{}

void MeshDepthProgram::insertDefines(Defines &defines) {
    MeshProgram::insertDefines(defines);

    defines.set("EMPTY_FRAGMENT_SHADER", true);
}

}
