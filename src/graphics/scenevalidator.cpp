#include "scenevalidator.h"

namespace graphics {

SceneValidator::SceneValidator(const SceneManager &sceneManager)
    : sceneManager(sceneManager)
{}

bool SceneValidator::isFaceActive(unsigned int index) const {
    return destroyedFaces.find(index) == destroyedFaces.end();
}
bool SceneValidator::isVertActive(unsigned int index) const {
    return destroyedVerts.find(index) == destroyedVerts.end();
}

void SceneValidator::validateActiveFace(SceneManager::FaceBuffer::Viewer face) {
    SceneManager::VertBuffer::Viewer vert0 = sceneManager.getVertBuffer().view(face.shared.verts[0]);
    SceneManager::VertBuffer::Viewer vert1 = sceneManager.getVertBuffer().view(face.shared.verts[1]);
    SceneManager::VertBuffer::Viewer vert2 = sceneManager.getVertBuffer().view(face.shared.verts[2]);

    // Verts are active
    assert(isVertActive(vert0.index));
    assert(isVertActive(vert1.index));
    assert(isVertActive(vert2.index));

    // Verts are distinct
    assert(vert0.index != vert1.index);
    assert(vert0.index != vert2.index);
    assert(vert1.index != vert2.index);

    // Verts are part of the same mesh
    assert(vert0.shared.meshIndex == vert1.shared.meshIndex);
    assert(vert0.shared.meshIndex == vert2.shared.meshIndex);
    assert(vert1.shared.meshIndex == vert2.shared.meshIndex);

    /*
    // Vert normals are kind of in the right direction
    glm::vec3 point0 = vert0.shared.getPoint();
    glm::vec3 point1 = vert1.shared.getPoint();
    glm::vec3 point2 = vert2.shared.getPoint();
    glm::vec3 faceNormal = glm::cross(point2 - point0, point1 - point0);
    glm::vec3 normal0 = vert0.shared.getNormal();
    glm::vec3 normal1 = vert1.shared.getNormal();
    glm::vec3 normal2 = vert2.shared.getNormal();
    assert(glm::dot(faceNormal, normal0) > 0.0f);
    assert(glm::dot(faceNormal, normal1) > 0.0f);
    assert(glm::dot(faceNormal, normal2) > 0.0f);
    */
}

void SceneValidator::validateDestroyedFace(SceneManager::FaceBuffer::Viewer face) {
    // First two verts are degenerate
    assert(face.shared.verts[0] == face.shared.verts[1]);
}

void SceneValidator::validateActiveVert(SceneManager::VertBuffer::Viewer vert) {
    // The normal has a length close to 1
    float normalLength = glm::length(vert.shared.getNormal());
    assert(normalLength > 0.99f);
    assert(normalLength < 1.01f);
}

void SceneValidator::validateDestroyedVert(SceneManager::VertBuffer::Viewer vert) {
    (void) vert;

    // Nothing to do here
}

}
