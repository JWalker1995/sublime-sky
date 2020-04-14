#include "pointclouddepthprogram.h"

namespace render {

PointCloudDepthProgram::PointCloudDepthProgram(game::GameContext &context)
    : PointCloudProgram(context)
{}

void PointCloudDepthProgram::insertDefines(Defines &defines) {
    PointCloudProgram::insertDefines(defines);

    defines.set("ENABLE_SHADING", false);
}

}
