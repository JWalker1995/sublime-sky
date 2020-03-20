#include "drawdepthprogram.h"

namespace render {

DrawDepthProgram::DrawDepthProgram(game::GameContext &context)
    : DrawProgram(context)
{}

void DrawDepthProgram::insertDefines(Defines &defines) {
    DrawProgram::insertDefines(defines);

    defines.set("EMPTY_FRAGMENT_SHADER", true);
}

}
