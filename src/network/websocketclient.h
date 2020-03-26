#pragma once

#include "websocketpp/client.hpp"
#include "websocketpp/config/asio_no_tls_client.hpp"

#include "jw_util/baseexception.h"

#include "game/gamecontext.h"
#include "network/baseconnection.h"

namespace network {

class WebSocketClient {
public:
    typedef websocketpp::client<websocketpp::config::asio_client> Client;

    class Connection : public BaseConnection {
    public:
        Connection(game::GameContext &context, const std::string &uri)
            : BaseConnection(context)
        {
            WebSocketClient &wsClient = context.get<WebSocketClient>();

            websocketpp::lib::error_code ec;
            Client::connection_ptr conn = wsClient.m_endpoint.get_connection(uri, ec);
            if (ec) {
                throw ConnectionException("Could not connect to " + uri + ": " + ec.message());
            }

            conn->set_open_handler([this](websocketpp::connection_hdl hdl) {
                setReady(true);

                Client::connection_ptr con = this->context.get<WebSocketClient>().m_endpoint.get_con_from_hdl(hdl);
//                con->get_response_header("Server");
            });
            conn->set_fail_handler([this, uri](websocketpp::connection_hdl hdl) {
                Client::connection_ptr con = this->context.get<WebSocketClient>().m_endpoint.get_con_from_hdl(hdl);
                throw ConnectionException("Could not connect to " + uri + ": " + con->get_ec().message());
            });

            conn->set_close_handler([this](websocketpp::connection_hdl hdl) {
                setReady(false);
            });

            wsClient.m_endpoint.connect(conn);

            handle = conn->get_handle();
        }

        void send(const char *data, std::size_t size) {
            websocketpp::lib::error_code ec;
            context.get<WebSocketClient>().m_endpoint.send(handle, data, size, websocketpp::frame::opcode::binary, ec);
            if (ec) {
                throw SendException("Could not send message: " + ec.message());
            }
        }

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


    WebSocketClient();

private:
    Client m_endpoint;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;
};

}
