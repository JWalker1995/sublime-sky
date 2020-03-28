#include "connectionmanager.h"

#include <string>

#include "network/websocketclient.h"
#include "util/pool.h"
#include "util/refset.h"

namespace network {

ConnectionManager::ConnectionManager(game::GameContext &context)
    : TickableBase(context)
{}

void ConnectionManager::tick(game::TickerContext &tickerContext) {
    (void) tickerContext;

    util::RefSet<BaseConnection>::Invoker<>::call<&BaseConnection::tick>(context.get<util::RefSet<BaseConnection>>());
}

BaseConnection *ConnectionManager::createConnection(const std::string &uriStr) {
    if (uriStr.substr(0, 3) == "ws:") {
        return context.get<util::Pool<WebSocketClient::Connection>>().alloc(context, uriStr);
    } else {
        throw UnexpectedUriSchemaException("Unexpected uri scheme in uri: " + uriStr);
    }
}

}
