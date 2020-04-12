#include "meshcolorprogram.h"

#include "graphics/imgui.h"
#include "render/camera.h"

namespace render {

MeshColorProgram::MeshColorProgram(game::GameContext &context)
    : MeshProgram(context)
{}

void MeshColorProgram::insertDefines(Defines &defines) {
    MeshProgram::insertDefines(defines);

    defines.set("EMPTY_FRAGMENT_SHADER", false);

    // context.get<FaceFragCounter>().insertDefines(defines);
}

void MeshColorProgram::linkProgram() {
    MeshProgram::linkProgram();

    showTrianglesLocation = glGetUniformLocation(getProgramId(), "showTriangles");
    assert(showTrianglesLocation != -1);
    graphics::GL::catchErrors();

    eyePosLocation = glGetUniformLocation(getProgramId(), "eyePos");
    assert(eyePosLocation != -1);
    graphics::GL::catchErrors();
}

void MeshColorProgram::draw() {
    MeshProgram::bind();

    if (ImGui::Begin("Debug")) {
        if (ImGui::Button("Toggle Triangles")) {
            showTrianglesValue = !showTrianglesValue;
            showTrianglesDirty = true;
        }
    }
    ImGui::End();

    if (showTrianglesDirty) {
        glUniform1i(showTrianglesLocation, showTrianglesValue);
        graphics::GL::catchErrors();

        showTrianglesDirty = false;
    }

    glm::vec3 eyePos = context.get<render::Camera>().getEyePos();
    glUniform3f(eyePosLocation, eyePos.x, eyePos.y, eyePos.z);

    SceneManager &sceneManager = context.get<SceneManager>();
    glDrawRangeElements(GL_TRIANGLES, 0, sceneManager.getVertBuffer().getExtentSize(), sceneManager.getFaceBuffer().getExtentSize() * 3, GL_UNSIGNED_INT, reinterpret_cast<char *>(0));
    graphics::GL::catchErrors();
}

}
