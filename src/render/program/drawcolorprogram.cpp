#include "drawcolorprogram.h"

#include "imgui/imgui.h"
#include "render/camera.h"

namespace render {

DrawColorProgram::DrawColorProgram(game::GameContext &context)
    : DrawProgram(context)
{}

void DrawColorProgram::insertDefines(Defines &defines) {
    DrawProgram::insertDefines(defines);

    defines.set("EMPTY_FRAGMENT_SHADER", false);

    // context.get<FaceFragCounter>().insertDefines(defines);
}

void DrawColorProgram::linkProgram() {
    DrawProgram::linkProgram();

    showTrianglesLocation = glGetUniformLocation(getProgramId(), "showTriangles");
    graphics::GL::catchErrors();

    eyePosLocation = glGetUniformLocation(getProgramId(), "eyePos");
    graphics::GL::catchErrors();
}

void DrawColorProgram::bind() {
    DrawProgram::bind();

    if (ImGui::Begin("Debug")) {
        if (ImGui::Button("Toggle Triangles")) {
            showTrianglesValue = !showTrianglesValue;
            showTrianglesDirty = true;
        }
        ImGui::End();
    }

    if (showTrianglesDirty) {
        glUniform1i(showTrianglesLocation, showTrianglesValue);
        graphics::GL::catchErrors();

        showTrianglesDirty = false;
    }

    glm::vec3 eyePos = context.get<render::Camera>().getEyePos();
    glUniform3f(eyePosLocation, eyePos.x, eyePos.y, eyePos.z);
}

}
