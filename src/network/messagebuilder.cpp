#include "messagebuilder.h"

#include "game/gamecontext.h"

namespace network {

MessageBuilder::Lock::Lock(game::GameContext &context)
    : Lock(&context.get<MessageBuilder>())
{}

MessageBuilder::MessageBuilder(game::GameContext &context)
    : context(context)
{}

}
