#include "websocketclient.h"

#include "schemas/message_generated.h"
#include "spdlog/logger.h"

namespace network {

WebSocketClient::WebSocketClient(game::GameContext &context)
    : TickableBase(context)
{
    m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
    m_endpoint.clear_error_channels(websocketpp::log::elevel::all);

    m_endpoint.init_asio();
}

WebSocketClient::~WebSocketClient() {
    // Tick one more time to send close packets
    m_endpoint.poll();
}

void WebSocketClient::tick(game::TickerContext &tickerContext) {
    (void) tickerContext;

    try {
        std::size_t numEvents = m_endpoint.poll();
        if (numEvents > 100) {
            context.get<spdlog::logger>().warn("Whoa! Asio poll handled more than 100 events in a single tick! It actually handled {}. This might mean a connected server is sending too much data.", numEvents);
        }
    } catch (const Connection::ConnectionException &ex) {
        context.get<spdlog::logger>().error("Asio poll failed: {}", ex.what());
    }
}

WebSocketClient::Connection::Connection(game::GameContext &context, const std::string &uri)
    : BaseConnection(context, uri)
{}

WebSocketClient::Connection::~Connection() {
    close();
}

void WebSocketClient::Connection::initializeDependencies(game::GameContext &context) {
    BaseConnection::initializeDependencies(context);
    context.get<WebSocketClient>();
}

void WebSocketClient::Connection::connect(const std::string &uri) {
    WebSocketClient &wsClient = context.get<WebSocketClient>();

    websocketpp::lib::error_code ec;
    Client::connection_ptr conn = wsClient.m_endpoint.get_connection(uri, ec);
    if (ec) {
        throw ConnectionException("Could not connect to " + uri + ": " + ec.message());
    }

    conn->set_open_handler([this](websocketpp::connection_hdl hdl) {
        if (hdl.lock() != handle.lock()) { return; }

        setCapabilities(getCapabilities() | SsProtocol::Capabilities_Connected);
        sendInitRequest();

        Client::connection_ptr con = this->context.get<WebSocketClient>().m_endpoint.get_con_from_hdl(hdl);
//        con->get_response_header("Server");
    });
    conn->set_fail_handler([this, uri](websocketpp::connection_hdl hdl) {
        if (hdl.lock() != handle.lock()) { return; }

        assert((getCapabilities() & SsProtocol::Capabilities_Connected) == 0);

        Client::connection_ptr con = this->context.get<WebSocketClient>().m_endpoint.get_con_from_hdl(hdl);
        throw ConnectionException("Could not connect to " + uri + ": " + con->get_ec().message());
    });

    conn->set_message_handler([this](websocketpp::connection_hdl hdl, WsppClientConfig::message_type::ptr message) {
        if (hdl.lock() != handle.lock()) { return; }

        if (message->get_opcode() == websocketpp::frame::opcode::binary) {
            recv(reinterpret_cast<const std::uint8_t *>(message->get_payload().data()), message->get_payload().size());
        }
    });

    conn->set_close_handler([this](websocketpp::connection_hdl hdl) {
        if (hdl.lock() != handle.lock()) { return; }

        setCapabilities(getCapabilities() & ~SsProtocol::Capabilities_Connected);
    });

    wsClient.m_endpoint.connect(conn);

    handle = conn->get_handle();
}

void WebSocketClient::Connection::send(const std::uint8_t *data, std::size_t size) {
    websocketpp::lib::error_code ec;
    context.get<WebSocketClient>().m_endpoint.send(handle, data, size, websocketpp::frame::opcode::binary, ec);
    if (ec) {
        throw SendException("Could not send message: " + ec.message());
    }
}

void WebSocketClient::Connection::close() {
    websocketpp::lib::error_code ec;
    context.get<WebSocketClient>().m_endpoint.close(handle, websocketpp::close::status::normal, "Closing sublime sky game client", ec);
    if (ec) {
        context.get<spdlog::logger>().warn("Error closing websocketpp connection: {}", ec.message());
    }
}

}
