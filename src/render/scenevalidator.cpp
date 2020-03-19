#include "scenevalidator.h"

#include "render/scenemanager.h"

namespace render {

SceneValidator::SceneValidator(game::GameContext &context)
    : graphics::SceneValidator(context.get<SceneManager>())
    , context(context)
{}

void SceneValidator::validate() {
    destroyedFaces.insert(sceneManager.getFaceBuffer().getAvailableIndices().cbegin(), sceneManager.getFaceBuffer().getAvailableIndices().cend());
    unsigned int totalFaces = sceneManager.getFaceBuffer().getExtentSize();
    for (unsigned int i = 0; i < totalFaces; i++) {
        SceneManager::FaceBuffer::Viewer face = sceneManager.getFaceBuffer().view(i);
        SceneManager::VertBuffer::Viewer vert = sceneManager.getVertBuffer().view(face.shared.verts[rand() % 3]);
        if (sceneManager.getMeshBuffer().view(vert.shared.meshIndex).local.shouldValidate) {
            if (isFaceActive(face.index)) {
                validateActiveFace(face);
            } else {
                validateDestroyedFace(face);
            }
        }
    }
    destroyedFaces.clear();

    destroyedVerts.insert(sceneManager.getVertBuffer().getAvailableIndices().cbegin(), sceneManager.getVertBuffer().getAvailableIndices().cend());
    unsigned int totalVerts = sceneManager.getVertBuffer().getExtentSize();
    for (unsigned int i = 0; i < totalVerts; i++) {
        SceneManager::VertBuffer::Viewer vert = sceneManager.getVertBuffer().view(i);
        if (sceneManager.getMeshBuffer().view(vert.shared.meshIndex).local.shouldValidate) {
            if (isVertActive(vert.index)) {
                validateActiveVert(vert);
            } else {
                validateDestroyedVert(vert);
            }
        }
    }
    destroyedVerts.clear();
}

}
