#include "signalhandler.h"

#include <csignal>
#include <unistd.h>

#include "application/quitexception.h"

namespace {

static volatile std::sig_atomic_t stopFlag = false;

void handleSignal(int signal) {
    if (signal != SIGINT) {
        return;
    }

    if (stopFlag) {
        const char *msg = "Caught a second SIGINT; exiting...\n";
        write(2, msg, strlen(msg));

        std::_Exit(5);
    } else {
        const char *msg = "Caught SIGINT; setting stop flag...\n";
        write(2, msg, strlen(msg));

        stopFlag = true;
    }
}

}

namespace application {

SignalHandler::SignalHandler(game::GameContext &context, const SsProtocol::Config::SignalHandler *config)
    : TickableBase(context)
{
    (void) config;

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
