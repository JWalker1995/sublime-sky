#include "connectionmanager.h"

#include <string>

#include "network/websocketclient.h"
#include "util/pool.h"

namespace network {

ConnectionManager::ConnectionManager(game::GameContext &context)
    : TickableBase(context)
{}

void ConnectionManager::tick(game::TickerContext &tickerContext) {

}

BaseConnection *ConnectionManager::addConnection(const std::string &uriStr) {
    if (uriStr.substr(0, 3) == "ws:") {
        return context.get<util::Pool<WebSocketClient::Connection>>().alloc(context, uriStr);
    } else {
        throw UnexpectedUriSchemaException("Unexpected uri scheme in uri: " + uriStr);
    }
}

void ConnectionManager::setReady(BaseConnection *connection, bool ready) {
    if (ready) {
        assert(std::find(activeConnections.cbegin(), activeConnections.cend(), connection) == activeConnections.cend());
        activeConnections.push_back(connection);
    } else {
        std::vector<BaseConnection *>::iterator found = std::find(activeConnections.begin(), activeConnections.end(), connection);
        assert(found != activeConnections.end());
        *found = activeConnections.back();
        activeConnections.pop_back();
    }
}

void ConnectionManager::send(const std::uint8_t *data, std::size_t size) {
}

}
