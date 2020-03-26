#include "connectionmanager.h"

#include <string>

#include "uriparser/include/uriparser/Uri.h"

#include "network/websocketclient.h"
#include "util/pool.h"

namespace network {

ConnectionManager::ConnectionManager(game::GameContext &context)
    : TickableBase(context)
{}

void ConnectionManager::tick(game::TickerContext &tickerContext) {

}

BaseConnection *ConnectionManager::addConnection(const std::string &uriStr) {
    UriUriA uri;
    const char *errorPos;
    if (uriParseSingleUriA(&uri, uriStr.data(), &errorPos) != URI_SUCCESS) {
        throw UriFormatException("Invalid URI " + uriStr + ": Parse error at character " + std::to_string(errorPos - uriStr.data()));
    }

    std::string scheme(uri.scheme.first, uri.scheme.afterLast);
    uriFreeUriMembersA(&uri);

    if (scheme == "ws") {
        return context.get<util::Pool<WebSocketClient::Connection>>().alloc(context, uriStr);
    } else {
        throw UnexpectedUriSchemaException("Unexpected uri scheme " + scheme);
    }
}

void ConnectionManager::send(const char *data, std::size_t size) {

}

}
