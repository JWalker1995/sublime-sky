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

    bool *ins = new bool[sizeXYZ];
    std::fill_n(ins, sizeXYZ, false);

    const std::vector<glm::vec3> &inPoints = request->getInternalPoints();
    std::vector<glm::vec3>::const_iterator i = inPoints.cbegin();
    while (i != inPoints.cend()) {
        signed int x = std::floorf(i->x);
        x -= minX;
        signed int y = std::floorf(i->y);
        y -= minY;
        signed int z = std::floorf(i->z);
        z -= minZ;

        if (x >= 0 && x < sizeX && y >= 0 && y < sizeY && z >= 0 && z < sizeZ) {
            unsigned int index = x * sizeYZ + y * sizeZ + z;
            ins[index] = true;
        }

        i++;
    }

    for (unsigned int x = 0; x < sizeX; x++) {
        for (unsigned int y = 0; y < sizeY; y++) {
            for (unsigned int z = 0; z < sizeZ; z++) {
                unsigned int index = x * sizeYZ + y * sizeZ + z;
                bool ii = ins[index];

                if (x > 0) {
                    bool ix = ins[index - sizeYZ];
                    if (ii && !ix) {
//                        createFace(request, );
                    }
                }
            }
        }
    }




    /*

        virtual glm::vec3 getRequestAabbMin() const = 0;
        virtual glm::vec3 getRequestAabbMax() const = 0;
        virtual const std::vector<glm::vec3> &getInternalPoints() const = 0;
        virtual const std::vector<glm::vec3> &getExternalPoints() const = 0;
        virtual std::vector<glm::vec3> &getDstVerticesArray() const = 0;
        virtual std::vector<Face> &getDstFacesArray() const = 0;

    */
}

}
