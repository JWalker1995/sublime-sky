#include "baseconnection.h"

#include "game/gamecontext.h"
#include "network/connectionpoolbase.h"
#include "util/refset.h"
#include "schemas/message_generated.h"
#include "network/messagebuilder.h"
#include "worldgen/externalgenerator.h"

namespace network {

BaseConnection::BaseConnection(game::GameContext &context, const std::string &uri)
    : context(context)
    , uri(uri)
{
    context.get<util::RefSet<BaseConnection>>().add(this);
    setCapabilities(0);
}

BaseConnection::~BaseConnection() {
    context.get<util::RefSet<BaseConnection>>().remove(this);
}

void BaseConnection::initializeDependencies(game::GameContext &context) {
    context.get<util::RefSet<BaseConnection>>();
}

void BaseConnection::setCapabilities(std::uint64_t newCapabilities) {
    capabilities = newCapabilities;
    util::RefSet<ConnectionPoolBase>::Invoker<BaseConnection *>::call<&ConnectionPoolBase::updateConnection>(context.get<util::RefSet<ConnectionPoolBase>>(), this);
}

void BaseConnection::recv(const std::uint8_t *data, std::size_t size) {
    flatbuffers::Verifier verifier(data, size);
    bool valid = SsProtocol::VerifyMessageBuffer(verifier);
    if (!valid) {
        context.log(game::GameContext::LogLevel::Warning, "Received invalid message with size=" + std::to_string(size));
        return;
    }

    const SsProtocol::Message *message = SsProtocol::GetMessage(data);
    context.log(game::GameContext::LogLevel::Trace, "Received message with size=" + std::to_string(size) + ", type=" + SsProtocol::EnumNameMessageUnion(message->message_type()));

    switch (message->message_type()) {
        case SsProtocol::MessageUnion_InitResponse: handleInitResponse(message->message_as_InitResponse()); break;
        case SsProtocol::MessageUnion_Chunk: context.get<worldgen::ExternalGenerator>().handleResponse(message->message_as_Chunk()); break;
        default: context.log(game::GameContext::LogLevel::Warning, "Received message with unexpected type: " + std::to_string(message->message_type()));
    }
}

void BaseConnection::sendInitRequest() {
    network::MessageBuilder::Lock lock(context);

    auto initRequest = SsProtocol::CreateInitRequest(lock.getBuilder());
    auto message = SsProtocol::CreateMessage(lock.getBuilder(), SsProtocol::MessageUnion_InitRequest, initRequest.Union());
    lock.getBuilder().Finish(message);

    send(lock.getBuilder().GetBufferPointer(), lock.getBuilder().GetSize());
}

void BaseConnection::handleInitResponse(const SsProtocol::InitResponse *response) {
    setCapabilities((response->capabilities() & ~SsProtocol::Capabilities_Connected) | (getCapabilities() & SsProtocol::Capabilities_Connected));
}

}
