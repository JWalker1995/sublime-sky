#include "connectionmanager.h"

#include <string>

#include "schemas/config_client_generated.h"
#include "network/websocketclient.h"
#include "util/pool.h"
#include "util/refset.h"

namespace network {

ConnectionManager::ConnectionManager(game::GameContext &context, const SsProtocol::Config::Network *config)
    : TickableBase(context)
{
    const flatbuffers::Vector<flatbuffers::Offset<SsProtocol::Config::ServerConnectionSpec>> *servers = config->servers();
    for (std::size_t i = 0; i < servers->size(); i++) {
        const SsProtocol::Config::ServerConnectionSpec *scs = servers->Get(i);
        switch (scs->protocol()) {
            case SsProtocol::Config::ConnectionProtocol_WebSocket: {
                std::string uri = "ws://" + scs->hostname()->str();
                if (scs->port()) {
                    uri += ":" + std::to_string(scs->port());
                }
                context.get<util::Pool<WebSocketClient::Connection>>().alloc(context, uri);
            } break;
        }
    }
}

void ConnectionManager::tick(game::TickerContext &tickerContext) {
    (void) tickerContext;

    util::RefSet<BaseConnection>::Invoker<>::call<&BaseConnection::tick>(context.get<util::RefSet<BaseConnection>>());
}

}
