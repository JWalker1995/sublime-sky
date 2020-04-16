#pragma once

#include "jw_util/context.h"
#include "jw_util/contextbuilder.h"

namespace game {

class GameContext : public jw_util::Context<GameContext> {
public:
    GameContext()
        : Context(0.1f)
    {}

    /*
    template <typename InterfaceType>
    InterfaceType &get() {
        gets++;
        return jw_util::Context::get<InterfaceType>();
    }
    */

    void log(LogLevel level, const std::string &msg);

private:
    unsigned int gets = 0;
};

}
