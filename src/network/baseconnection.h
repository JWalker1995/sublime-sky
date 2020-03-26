#pragma once

#include <cstdlib>

#include "jw_util/baseexception.h"

namespace game { class GameContext; }

namespace network {

class BaseConnection {
public:
    class SendException : public jw_util::BaseException {
    public:
        SendException(const std::string &msg)
            : BaseException(msg)
        {}
    };

    BaseConnection(game::GameContext &context);
    virtual ~BaseConnection() {}

    virtual void send(const char *data, std::size_t size) = 0;

protected:
    game::GameContext &context;

    void setReady(bool ready);

    void recv(const char *data, std::size_t size);
};

}
