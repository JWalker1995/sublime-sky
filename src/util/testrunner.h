#pragma once

#include <vector>

namespace game { class GameContext; }

namespace util {

class TestRunner {
public:
    static TestRunner getInstance() {
        static TestRunner instance;
        return instance;
    }

    void run();

    int registerGameTest(void funcPtr(game::GameContext &)) {
        gameTests.push_back(funcPtr);
        return 0;
    }

private:
    std::vector<void (game::GameContext &)> gameTests;
};

}
