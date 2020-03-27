#pragma once

#include <cstdlib>

#include "jw_util/baseexception.h"

namespace SsProtocol { struct InitResponse; }

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

    BaseConnection(game::GameContext &context, const std::string &uri);
    virtual ~BaseConnection();

    static void initializeDependencies(game::GameContext &context);

//    virtual void connect(const std::string &uri) = 0;

    virtual void send(const std::uint8_t *data, std::size_t size) = 0;

    std::uint64_t getCapabilities() const {
        return capabilities;
    }

protected:
    game::GameContext &context;

    void sendInitRequest();
    void setCapabilities(std::uint64_t capabilities);

    void recv(const std::uint8_t *data, std::size_t size);

private:
    const std::string &uri;

    std::uint64_t capabilities = 0;

    void handleInitResponse(const SsProtocol::InitResponse *response);
};

}
