#pragma once

#include "game/tickercontext.h"

namespace SsProtocol {
namespace Config { struct JumpFlooder; }
}

namespace render {

class JumpFlooder : public game::TickerContext::TickableBase<JumpFlooder> {
public:
    JumpFlooder(game::GameContext &context, const SsProtocol::Config::JumpFlooder *config);

    void tick(game::TickerContext &tickerContext);

    void flood();
};

}
