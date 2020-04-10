#pragma once

#include <vector>

#include "jw_util/baseexception.h"
#include "game/tickercontext.h"

namespace SsProtocol {
namespace Config { struct Network; }
}

namespace network {

class ConnectionManager : public game::TickerContext::TickableBase<ConnectionManager> {
private:
public:
    class NoNetworkConfigException : public jw_util::BaseException {
        friend class ConnectionManager;

    private:
        NoNetworkConfigException(const std::string &msg)
            : BaseException(msg)
        {}
    };

    ConnectionManager(game::GameContext &context, const SsProtocol::Config::Network *config);

    void tick(game::TickerContext &tickerContext);
};

}
