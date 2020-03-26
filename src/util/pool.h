#pragma once

#include "jw_util/pool.h"

namespace game { class GameContext; }

namespace util {

template <typename Type>
class Pool : public jw_util::Pool<Type> {
public:
    Pool(game::GameContext &context) {
        (void) context;
    }
};

}
