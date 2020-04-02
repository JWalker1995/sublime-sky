#include "baseconnection.h"

#include "spdlog/spdlog.h"

#include "game/gamecontext.h"
#include "network/connectionpoolbase.h"
#include "util/refset.h"
#include "schemas/message_generated.h"
#include "schemas/config_game_generated.h"
#include "network/messagebuilder.h"
#include "worldgen/externalgenerator.h"
#include "material/materialmanager.h"

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

void BaseConnection::tick() {
    if (capabilities == 0) {
        if (waitTicks) {
            waitTicks--;
        } else {
            waitTicksBackoff *= 2;
            waitTicks = waitTicksBackoff;

            callConnect();
        }
    }
}

void BaseConnection::callConnect() {
    try {
        context.get<spdlog::logger>().info("Attempting connection to {}...", uri);
        connect(uri);
    } catch (const ConnectionException &ex) {
        context.get<spdlog::logger>().error("Failed connecting to {}: {}", uri, ex.what());
    }
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
        case SsProtocol::MessageUnion_TerrainChunk: handleTerrainChunk(message->message_as_TerrainChunk()); break;
        default: context.log(game::GameContext::LogLevel::Warning, "Received message with unexpected type: " + std::to_string(message->message_type()));
    }
}

void BaseConnection::sendInitRequest() {
    network::MessageBuilder::Lock lock(context);

    std::uint64_t seed = context.get<const SsProtocol::Config::Game>().seed();
    auto initRequest = SsProtocol::CreateInitRequest(lock.getBuilder(), seed);
    auto message = SsProtocol::CreateMessage(lock.getBuilder(), SsProtocol::MessageUnion_InitRequest, initRequest.Union());
    lock.getBuilder().Finish(message);

    send(lock.getBuilder().GetBufferPointer(), lock.getBuilder().GetSize());
}

void BaseConnection::handleInitResponse(const SsProtocol::InitResponse *msg) {
    setCapabilities((msg->capabilities() & ~SsProtocol::Capabilities_Connected) | (getCapabilities() & SsProtocol::Capabilities_Connected));

    if (msg->materials()) {
        materialOffset = context.get<material::MaterialManager>().registerMaterials(msg->materials());
        materialCount = msg->materials()->size();
    }
}

void BaseConnection::handleTerrainChunk(const SsProtocol::TerrainChunk *msg) {
    if (materialCount == 0) {
        context.log(game::GameContext::LogLevel::Warning, "Received TerrainChunk message but no materials yet; skipping message.");
        return;
    }

    auto it = msg->cell_materials()->cbegin();
    while (it != msg->cell_materials()->cend()) {
        if (*it >= materialCount) {
            context.log(game::GameContext::LogLevel::Warning, "Received TerrainChunk message with material index beyond InitResponse::materials array; skipping: " + std::to_string(*it));
            return;
        }
        it++;
    }

    context.get<worldgen::ExternalGenerator>().handleResponse(msg, materialOffset);
}

}
