#pragma once

#define ASIO_STANDALONE
#include "websocketpp/client.hpp"
#include "websocketpp/config/asio_no_tls_client.hpp"
#include "websocketpp/concurrency/none.hpp"

#include "jw_util/baseexception.h"

#include "game/tickercontext.h"
#include "network/baseconnection.h"

namespace network {

class WebSocketClient : game::TickerContext::TickableBase<WebSocketClient> {
public:
    struct WsppClientConfig : public websocketpp::config::asio_client {
        typedef websocketpp::concurrency::none concurrency_type;

        static bool const enable_multithreading = false;

        struct transport_config : public websocketpp::config::asio_client::transport_config {
            static bool const enable_multithreading = false;
        };

        typedef websocketpp::transport::asio::endpoint<transport_config> transport_type;
    };
    typedef websocketpp::client<WsppClientConfig> Client;

    class Connection : public BaseConnection {
    public:
        Connection(game::GameContext &context, const std::string &uri);
        ~Connection();

        static void initializeDependencies(game::GameContext &context);

        void send(const std::uint8_t *data, std::size_t size);

        void close();

    private:
        websocketpp::connection_hdl handle;
    };

    class ConnectionException : public jw_util::BaseException {
        friend class WebSocketClient::Connection;

    private:
        ConnectionException(const std::string &msg)
            : BaseException(msg)
        {}
    };

    WebSocketClient(game::GameContext &context);
    ~WebSocketClient();

    void tick(game::TickerContext &tickerContext);

private:
    Client m_endpoint;
};

}
