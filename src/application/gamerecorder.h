#pragma once

#include <string>

namespace game { class GameContext; }

namespace application {

class GameRecorder {
public:
    GameRecorder(game::GameContext &context);
    ~GameRecorder();

    void loadScreenDims();
    void chooseDir();
    void initDir();
    void takeSnapshot();

private:
    bool enabled = true;
    unsigned int width = 0;
    unsigned int height = 0;
    std::string dir;
    unsigned int nextFileIndex = 0;
};

}
