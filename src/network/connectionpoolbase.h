#pragma once

#include <vector>

namespace game { class GameContext; }

namespace network {

class BaseConnection;

class ConnectionPoolBase {
public:
    ConnectionPoolBase(game::GameContext &context);
    virtual ~ConnectionPoolBase();

    void send(const std::uint8_t *data, std::size_t size);

    virtual void updateConnection(BaseConnection *connection) = 0;

protected:
    game::GameContext &context;

    void updateAllConnections();

    void insert(BaseConnection *connection);
    void remove(BaseConnection *connection);

private:
    std::vector<BaseConnection *> connections;
    unsigned int nextSendConnIndex = 0;

    std::vector<std::string> pendingSends;
};

}
