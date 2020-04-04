#pragma once

#include <cstdint>

#include "defs/CELL_MATERIAL_INDEX_TYPE.h"

namespace world {

enum class MaterialIndex : CELL_MATERIAL_INDEX_TYPE {
    Null = 0,
    Generating = 1,
};

}
