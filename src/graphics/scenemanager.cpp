#include "scenemanager.h"

namespace graphics {

SceneManager::SceneManager()
    : meshBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW)
    , vertBuffer(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW)
    , faceBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW)
    , materialBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW)
{}

}
