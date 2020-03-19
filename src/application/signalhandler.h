#pragma once

#include "game/tickercontext.h"

namespace game { class GameContext; }

namespace application {

class SignalHandler : public game::TickerContext::TickableBase<SignalHandler> {
public:
    SignalHandler(game::GameContext &context);
    ~SignalHandler();

    void tick(game::TickerContext &tickerContext);
};

}
