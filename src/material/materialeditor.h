#pragma once

#include "game/tickercontext.h"

namespace material {

class MaterialEditor : public game::TickerContext::TickableBase<MaterialEditor> {
public:
    MaterialEditor(game::GameContext &context);

    void tick(game::TickerContext &tickerContext);
};

}
