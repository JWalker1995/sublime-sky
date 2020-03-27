#pragma once

#include "network/connectionpoolbase.h"
#include "network/baseconnection.h"
#include "schemas/message_generated.h"

namespace game { class GameContext; }

namespace network {

template <SsProtocol::Capabilities requiredCapabilities>
class ConnectionPoolSpecialized : public ConnectionPoolBase {
public:
    static constexpr std::uint64_t capabilityMask = requiredCapabilities | SsProtocol::Capabilities_Connected;

    ConnectionPoolSpecialized(game::GameContext &context)
        : ConnectionPoolBase(context)
    {
        updateAllConnections();
    }

    void updateConnection(BaseConnection *connection) {
        bool matches = (capabilityMask & ~connection->getCapabilities()) == 0;
        if (matches) {
            insert(connection);
        } else {
            remove(connection);
        }
    }
};

}
