#include "cubichoneycomb.h"

namespace meshgen {

CubicHoneycomb::CubicHoneycomb(game::GameContext &context) {
    (void) context;
}

void CubicHoneycomb::generate(Request *request) {
    glm::vec3 min = request->getRequestAabbMin();
    glm::vec3 max = request->getRequestAabbMax();

    signed int minX = std::ceilf(min.x);
    signed int minY = std::ceilf(min.y);
    signed int minZ = std::ceilf(min.z);

    signed int maxX = std::ceilf(max.x);
    signed int maxY = std::ceilf(max.y);
    signed int maxZ = std::ceilf(max.z);

    assert(minX <= maxX);
    assert(minY <= maxY);
    assert(minZ <= maxZ);

    unsigned int sizeX = maxX - minX;
    unsigned int sizeY = maxY - minY;
    unsigned int sizeZ = maxZ - minZ;
    unsigned int sizeYZ = sizeY * sizeZ;
    unsigned int sizeXYZ = sizeX * sizeYZ;

    unsigned int *internalIds = new unsigned int[sizeXYZ];
    std::fill_n(internalIds, sizeXYZ, static_cast<unsigned int>(-1));

    const std::vector<std::pair<unsigned int, glm::vec3>> &inPoints = request->getInternalPoints();
    std::vector<std::pair<unsigned int, glm::vec3>>::const_iterator i = inPoints.cbegin();
    while (i != inPoints.cend()) {
        signed int x = std::floorf(i->second.x);
        x -= minX;
        signed int y = std::floorf(i->second.y);
        y -= minY;
        signed int z = std::floorf(i->second.z);
        z -= minZ;

        if (x >= 0 && x < sizeX && y >= 0 && y < sizeY && z >= 0 && z < sizeZ) {
            unsigned int index = x * sizeYZ + y * sizeZ + z;
            internalIds[index] = i->first;
        }

        i++;
    }

    glm::vec3 pos;
    for (unsigned int x = 0; x < sizeX; x++) {
        pos.x = static_cast<signed int>(minX + x);
        for (unsigned int y = 0; y < sizeY; y++) {
            pos.y = static_cast<signed int>(minY + y);
            for (unsigned int z = 0; z < sizeZ; z++) {
                pos.z = static_cast<signed int>(minZ + z);

                unsigned int index = x * sizeYZ + y * sizeZ + z;
                unsigned int cur = internalIds[index];
                bool testCur = cur != static_cast<unsigned int>(-1);

                if (x > 0) {
                    unsigned int off = internalIds[index - sizeYZ];
                    bool testOff = off != static_cast<unsigned int>(-1);
                    if (testCur && !testOff) {
                        createFace(request, cur, pos, pos + glm::vec3(0.0f, 0.0f, 1.0f), pos + glm::vec3(0.0f, 1.0f, 1.0f), pos + glm::vec3(0.0f, 1.0f, 0.0f));
                    } else if (!testCur && testOff) {
                        createFace(request, off, pos, pos + glm::vec3(0.0f, 1.0f, 0.0f), pos + glm::vec3(0.0f, 1.0f, 1.0f), pos + glm::vec3(0.0f, 0.0f, 1.0f));
                    }
                }

                if (y > 0) {
                    unsigned int off = internalIds[index - sizeZ];
                    bool testOff = off != static_cast<unsigned int>(-1);
                    if (testCur && !testOff) {
                        createFace(request, cur, pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), pos + glm::vec3(1.0f, 0.0f, 1.0f), pos + glm::vec3(0.0f, 0.0f, 1.0f));
                    } else if (!testCur && testOff) {
                        createFace(request, off, pos, pos + glm::vec3(0.0f, 0.0f, 1.0f), pos + glm::vec3(1.0f, 0.0f, 1.0f), pos + glm::vec3(1.0f, 0.0f, 0.0f));
                    }
                }

                if (z > 0) {
                    unsigned int off = internalIds[index - 1];
                    bool testOff = off != static_cast<unsigned int>(-1);
                    if (testCur && !testOff) {
                        createFace(request, cur, pos, pos + glm::vec3(0.0f, 1.0f, 0.0f), pos + glm::vec3(1.0f, 1.0f, 0.0f), pos + glm::vec3(1.0f, 0.0f, 0.0f));
                    } else if (!testCur && testOff) {
                        createFace(request, off, pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), pos + glm::vec3(1.0f, 1.0f, 0.0f), pos + glm::vec3(0.0f, 1.0f, 0.0f));
                    }
                }
            }
        }
    }

    request->onComplete();
}

void CubicHoneycomb::createFace(Request *request, unsigned int internalPointId, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
    Request::Face face;
    face.internalPointId = internalPointId;

    face.vertPositions[0] = v0;
    face.vertPositions[1] = v1;
    face.vertPositions[2] = v2;
    request->getDstFacesArray().push_back(face);

    face.vertPositions[0] = v2;
    face.vertPositions[1] = v3;
    face.vertPositions[2] = v0;
    request->getDstFacesArray().push_back(face);
}

}
