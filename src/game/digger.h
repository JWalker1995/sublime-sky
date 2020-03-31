#pragma once

#include "game/tickercontext.h"

namespace game {

class Digger : public TickerContext::TickableBase<Digger> {
public:
    Digger(game::GameContext &context);

    void tick(game::TickerContext &tickerContext);

private:
};

}
