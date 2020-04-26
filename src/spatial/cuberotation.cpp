#include "cuberotation.h"

#include <iostream>

#include "graphics/glm.h"
#include <glm/mat3x3.hpp>

#include "jw_util/fastmath.h"

namespace spatial {

static signed int determinant(const glm::tmat3x3<signed int> &mat) {
    signed int d1 = (mat[1][1] * mat[2][2]) - (mat[2][1] * mat[1][2]);
    signed int d2 = (mat[1][0] * mat[2][2]) - (mat[2][0] * mat[1][2]);
    signed int d3 = (mat[1][0] * mat[2][1]) - (mat[2][0] * mat[1][1]);

    return (mat[0][0] * d1) - (mat[0][1] * d2) + (mat[0][2] * d3);
}

static glm::tmat3x3<signed int> transpose(const glm::tmat3x3<signed int> &mat) {
    return glm::tmat3x3<signed int>(
                mat[0][0], mat[1][0], mat[2][0],
                mat[0][1], mat[1][1], mat[2][1],
                mat[0][2], mat[1][2], mat[2][2]
            );
}

static bool isRotationMatrix(const glm::tmat3x3<signed int> &mat) {
    signed int det = determinant(mat);
    return (det == -1 || det == 1) && mat * transpose(mat) == glm::tmat3x3<signed int>(1);
}

static void printMatrix(const glm::tmat3x3<signed int> &mat) {
    std::cout << mat[0][0] << "," << mat[0][1] << "," << mat[0][2] << std::endl;
    std::cout << mat[1][0] << "," << mat[1][1] << "," << mat[1][2] << std::endl;
    std::cout << mat[2][0] << "," << mat[2][1] << "," << mat[2][2] << std::endl;
    std::cout << std::endl;
}

int CubeRotation::buildLookupTables() {
    glm::tmat3x3<signed int> rotationMatrices[numRotations];
    unsigned int nextRotMatIndex = 0;

    glm::tmat3x3<signed int> mat;
#define LOOP_3(x, y) for (mat[x][y] = -1; mat[x][y] <= 1; mat[x][y]++)
    LOOP_3(0, 0) LOOP_3(0, 1) LOOP_3(0, 2) LOOP_3(1, 0) LOOP_3(1, 1) LOOP_3(1, 2) LOOP_3(2, 0) LOOP_3(2, 1) LOOP_3(2, 2) {
#undef LOOP_3
        if (isRotationMatrix(mat)) {
            // Make sure there's no duplicates
            assert(std::find(rotationMatrices, rotationMatrices + numRotations, mat) == rotationMatrices + numRotations);

            assert(nextRotMatIndex < numRotations);
            rotationMatrices[nextRotMatIndex++] = mat;
        }
    }
    assert(nextRotMatIndex == numRotations);

    unsigned int k = std::find(rotationMatrices, rotationMatrices + numRotations, glm::tmat3x3<signed int>(1)) - rotationMatrices;
    assert(k < numRotations);
    identityLookup = k;

    for (unsigned int i = 0; i < numRotations; i++) {
        // Build applicationLookupTable
        for (unsigned int j = 0; j < 8; j++) {
            glm::tvec3<signed int> pt(j & 4 ? 1 : -1, j & 2 ? 1 : -1, j & 1 ? 1 : -1);
            pt = rotationMatrices[i] * pt;
            assert(pt[0] == -1 || pt[0] == 1);
            assert(pt[1] == -1 || pt[1] == 1);
            assert(pt[2] == -1 || pt[2] == 1);
            applicationLookupTable[i][j] = (pt[0] == 1 ? 4 : 0) | (pt[1] == 1 ? 2 : 0) | (pt[2] == 1 ? 1 : 0);
        }

        // Build compositionLookupTable
        for (unsigned int j = 0; j < numRotations; j++) {
            glm::tmat3x3<signed int> comp = rotationMatrices[i] * rotationMatrices[j];
            unsigned int k = std::find(rotationMatrices, rotationMatrices + numRotations, comp) - rotationMatrices;
            assert(k < numRotations);
            compositionLookupTable[i][j] = k;
        }

        // Build inversionLookupTable
        unsigned int k = std::find(rotationMatrices, rotationMatrices + numRotations, transpose(rotationMatrices[i])) - rotationMatrices;
        assert(k < numRotations);
        inversionLookupTable[i] = k;
    }

    return 0;
}

unsigned int CubeRotation::identityLookup;
unsigned int CubeRotation::applicationLookupTable[numRotations][8];
unsigned int CubeRotation::compositionLookupTable[numRotations][numRotations];
unsigned int CubeRotation::inversionLookupTable[numRotations];

static int _ = CubeRotation::buildLookupTables();

}
