#pragma once

#include "flatbuffers/flatbuffers.h"

namespace game { class GameContext; }

namespace network {

class MessageBuilder {
public:
    class Lock {
    public:
        Lock(game::GameContext &context);

        Lock(MessageBuilder *messageBuilder)
            : messageBuilder(messageBuilder)
        {
            assert(!messageBuilder->locked);
            messageBuilder->locked = true;
        }

        Lock(Lock&& original)
           : messageBuilder(original.messageBuilder)
        {
            original.messageBuilder = nullptr;
        }

        ~Lock() {
            if (messageBuilder) {
                assert(messageBuilder->locked);
                messageBuilder->builder.Clear();
                messageBuilder->locked = false;
            }
        }

        Lock& operator=(Lock&& other) {
            std::swap(messageBuilder, other.messageBuilder);
            return *this;
        }

        flatbuffers::FlatBufferBuilder &getBuilder() {
            return messageBuilder->builder;
        }

    private:
        MessageBuilder *messageBuilder = nullptr;
    };

    MessageBuilder(game::GameContext &context);

private:
    game::GameContext &context;
    flatbuffers::FlatBufferBuilder builder;
    bool locked = false;
};

}
