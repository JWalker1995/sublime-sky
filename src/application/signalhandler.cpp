#include "signalhandler.h"

#include <csignal>

#include "application/quitexception.h"

namespace {

static volatile std::sig_atomic_t stopFlag = false;

void handleSignal(int signal) {
    stopFlag = true;
}

}

namespace application {

SignalHandler::SignalHandler(game::GameContext &context)
    : TickableBase(context)
{
    std::signal(SIGINT, handleSignal);
}

SignalHandler::~SignalHandler() {
    std::signal(SIGINT, SIG_DFL);
}

void SignalHandler::tick(game::TickerContext &tickerContext) {
    (void) tickerContext;

    if (stopFlag) {
        throw QuitException();
    }
}

}
