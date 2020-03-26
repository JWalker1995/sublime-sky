#include "baseconnection.h"

#include "game/gamecontext.h"
#include "schemas/message_generated.h"
#include "network/connectionmanager.h"
#include "worldgen/externalgenerator.h"

namespace network {

BaseConnection::BaseConnection(game::GameContext &context)
    : context(context)
{}

void BaseConnection::setReady(bool ready) {
    context.get<ConnectionManager>().setReady(this, ready);
}

void BaseConnection::recv(const std::uint8_t *data, std::size_t size) {
    flatbuffers::Verifier verifier(data, size);
    bool valid = SublimeSky::VerifyMessageBuffer(verifier);
    if (!valid) {
        context.log(game::GameContext::LogLevel::Warning, "Received invalid message with size=" + std::to_string(size));
        return;
    }

    const SublimeSky::Message *message = SublimeSky::GetMessage(data);

    context.log(game::GameContext::LogLevel::Trace, "Received message with size=" + std::to_string(size) + ", type=" + SublimeSky::EnumNameMessageUnion(message->message_type()));

    switch (message->message_type()) {
        case SublimeSky::MessageUnion_Chunk: context.get<worldgen::ExternalGenerator>().handleResponse(message->message_as_Chunk()); break;
        default: context.log(game::GameContext::LogLevel::Warning, "Received message with unhandled type: " + std::to_string(message->message_type()));
    }
}

}
