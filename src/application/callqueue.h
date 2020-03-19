#pragma once

#include <queue>
#include <functional>

#include "game/gamecontext.h"
#include "game/tickercontext.h"

namespace application {

class CallQueue : public game::TickerContext::TickableBase<CallQueue> {
public:
    CallQueue(game::GameContext &context)
        : TickableBase(context)
    {}

    void push(std::function<void(game::GameContext &)> callback) {
        queue.push(callback);
    }

    void tick(game::TickerContext &tickerContext) {
        (void) tickerContext;
        run();
    }

    void run() {
        while (!queue.empty()) {
            queue.front()(context);
            queue.pop();
        }
    }

private:
    std::queue<std::function<void(game::GameContext &)>> queue;
};

}
