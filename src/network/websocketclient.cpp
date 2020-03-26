#include "websocketclient.h"

namespace network {

WebSocketClient::WebSocketClient() {
    m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
    m_endpoint.clear_error_channels(websocketpp::log::elevel::all);

    m_endpoint.init_asio();
    m_endpoint.start_perpetual();

    m_thread.reset(new websocketpp::lib::thread(&Client::run, &m_endpoint));
}

}
