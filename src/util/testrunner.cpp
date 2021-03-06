#include "testrunner.h"

#include "game/gamecontext.h"

namespace util {

void TestRunner::run(const SsProtocol::Config::TestRunner *config) {
    (void) config;

    // Lets create a new test context
    game::GameContext testContext;

    for (std::size_t i = 0; i < gameTests.size(); i++) {
        gameTests[i](testContext);
    }

    gameTests.clear();
}

}
