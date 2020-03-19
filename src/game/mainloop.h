#pragma once

#include <cstdint>

namespace game {

class GameContext;

class MainLoop {
public:
    MainLoop(GameContext &context);

    void load();
    void run();

    std::uint64_t getCurrentTime() const {
        return curTime;
    }

private:
    GameContext &context;

    std::uint64_t curTime = 0;
};

}
