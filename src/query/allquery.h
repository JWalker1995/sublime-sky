#pragma once

#include "spatial/cellkey.h"

namespace query {

class AllQuery {
public:
    bool testCellKey(spatial::CellKey cellKey) {
        (void) cellKey;
        return true;
    }

};

}
