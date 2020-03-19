#pragma once

#include "jw_util/pool.h"

namespace game { class GameContext; }

namespace util {

template <typename Type, bool shrink = false, typename ContainerType = std::deque<Type>>
class Pool : public jw_util::Pool<Type, shrink, ContainerType> {
public:
    Pool(game::GameContext &context) {
        (void) context;
    }
};

}
