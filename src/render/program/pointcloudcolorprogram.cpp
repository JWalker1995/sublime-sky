#include "pointcloudcolorprogram.h"

#include "graphics/imgui.h"
#include "render/camera.h"

namespace render {

PointCloudColorProgram::PointCloudColorProgram(game::GameContext &context)
    : PointCloudProgram(context)
{}

void PointCloudColorProgram::insertDefines(Defines &defines) {
    PointCloudProgram::insertDefines(defines);

    defines.set("ENABLE_SHADING", true);

    // context.get<FaceFragCounter>().insertDefines(defines);
}

}
