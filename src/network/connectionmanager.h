#pragma once

#include <vector>

#include "jw_util/baseexception.h"
#include "game/tickercontext.h"

namespace network {

class BaseConnection;

class ConnectionManager : public game::TickerContext::TickableBase<ConnectionManager> {
public:
    class UriFormatException : public jw_util::BaseException {
        friend class ConnectionManager;

    private:
        UriFormatException(const std::string &msg)
            : BaseException(msg)
        {}
    };

    class UnexpectedUriSchemaException : public jw_util::BaseException {
        friend class ConnectionManager;

    private:
        UnexpectedUriSchemaException(const std::string &msg)
            : BaseException(msg)
        {}
    };


    ConnectionManager(game::GameContext &context);

    void tick(game::TickerContext &tickerContext);

    BaseConnection *addConnection(const std::string &uriStr);

    void setReady(BaseConnection *connection, bool ready);

    void send(const std::uint8_t *data, std::size_t size);

private:
    std::vector<BaseConnection *> connections;
    std::vector<BaseConnection *> activeConnections;

    unsigned int nextSendIndex = 0;
};

}
