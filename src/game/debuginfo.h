#pragma once

#include "game/tickercontext.h"

namespace game { class GameContext; }

namespace game {

class DebugInfo : public TickerContext::TickableBase<DebugInfo> {
public:
    DebugInfo(game::GameContext &context);

    void tick(TickerContext &tickerContext);

    unsigned int counter = 0;
};

}
