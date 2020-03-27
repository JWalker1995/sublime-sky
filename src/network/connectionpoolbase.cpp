#include "connectionpoolbase.h"

#include "game/gamecontext.h"
#include "network/connectionmanager.h"
#include "network/baseconnection.h"
#include "util/refset.h"

namespace network {

ConnectionPoolBase::ConnectionPoolBase(game::GameContext &context)
    : context(context)
{
    context.get<util::RefSet<ConnectionPoolBase>>().add(this);
}

ConnectionPoolBase::~ConnectionPoolBase() {
    context.get<util::RefSet<ConnectionPoolBase>>().remove(this);
}

void ConnectionPoolBase::send(const std::uint8_t *data, std::size_t size) {
    if (connections.empty()) {
        pendingSends.emplace_back(reinterpret_cast<const char *>(data), size);
    } else {
        if (nextSendConnIndex >= connections.size()) {
            nextSendConnIndex = 0;
        }
        connections[nextSendConnIndex++]->send(data, size);
    }
}

void ConnectionPoolBase::updateAllConnections() {
    util::RefSet<BaseConnection> &connectionSet = context.get<util::RefSet<BaseConnection>>();
    util::RefSet<BaseConnection>::iterator i = connectionSet.cbegin();
    while (i != connectionSet.cend()) {
        updateConnection(*i);
        i++;
    }
}

void ConnectionPoolBase::insert(BaseConnection *connection) {
    std::vector<BaseConnection *>::const_iterator found = std::find(connections.cbegin(), connections.cend(), connection);
    if (found == connections.cend()) {
        connections.push_back(connection);

        for (const std::string &str : pendingSends) {
            send(reinterpret_cast<const std::uint8_t *>(str.data()), str.size());
        }
        pendingSends.clear();
    }
}

void ConnectionPoolBase::remove(BaseConnection *connection) {
    std::vector<BaseConnection *>::iterator found = std::find(connections.begin(), connections.end(), connection);
    if (found != connections.end()) {
        *found = connections.back();
        connections.pop_back();
    }
}

}
