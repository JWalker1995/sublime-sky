#pragma once

#include "spatial/uintcoord.h"

namespace game { class GameContext; }

namespace world {

class TimeManager {
public:
    TimeManager(game::GameContext &context);

    void incTimeAround(spatial::UintCoord coord, float time);
};

}
