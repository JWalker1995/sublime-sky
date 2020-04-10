#pragma once

#include "game/tickercontext.h"

namespace SsProtocol {
namespace Config { struct SignalHandler; }
}

namespace game { class GameContext; }

namespace application {

class SignalHandler : public game::TickerContext::TickableBase<SignalHandler> {
public:
    SignalHandler(game::GameContext &context, const SsProtocol::Config::SignalHandler *config);
    ~SignalHandler();

    void tick(game::TickerContext &tickerContext);
};

}
