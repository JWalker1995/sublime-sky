#pragma once

#include <assert.h>

namespace spatial {

class CubeRotation {
public:
    static constexpr unsigned int numRotations = 48;

    static int buildLookupTables();

    static CubeRotation identity() {
        CubeRotation res;
        res.value = identityLookup;
        return res;
    }

    unsigned int rotate(unsigned int point) {
        assert(point < 8);
        return applicationLookupTable[value][point];
    }

    CubeRotation rotate(CubeRotation child) {
        CubeRotation res;
        res.value = compositionLookupTable[value][child.value];
        return res;
    }

    CubeRotation inverse() const {
        CubeRotation res;
        res.value = inversionLookupTable[value];
        return res;
    }

private:
    unsigned char value;

    static unsigned int identityLookup;
    static unsigned int applicationLookupTable[numRotations][8];
    static unsigned int compositionLookupTable[numRotations][numRotations];
    static unsigned int inversionLookupTable[numRotations];
};

}
