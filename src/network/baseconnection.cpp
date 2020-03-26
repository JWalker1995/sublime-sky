#include "baseconnection.h"

#include "game/gamecontext.h"

namespace network {

BaseConnection::BaseConnection(game::GameContext &context)
    : context(context)
{}

void BaseConnection::recv(const char *data, std::size_t size) {
//    context.get<>
}

}
