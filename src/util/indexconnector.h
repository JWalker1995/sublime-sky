#pragma once

#include <cstdint>
#include <functional>
#include <deque>
#include <unordered_map>

#include "game/tickercontext.h"

namespace game { class GameContext; }

namespace util {

class IndexConnector : public game::TickerContext::TickableBase<IndexConnector> {
private:
    static constexpr std::size_t callbackMemorySize = sizeof(std::function<void(int &)>);
    struct CallbackMemory {
        char mem[callbackMemorySize];
    };

    template <typename T>
    struct CallbackType {
        typedef std::function<void(typename std::remove_cv<T>::type &)> type;
    };

    struct Key {
        Key(std::uint64_t index, std::type_index type)
            : index(index)
            , type(type)
        {}

        std::uint64_t index;
        std::type_index type;

        bool operator==(const Key &other) {
            return index == other.index && type == other.type;
        }

        struct Hasher {
            std::size_t operator()(const Key &key) const {
                return 0x9e3779b97f4a7c15u + (std::hash<std::uint64_t>{}(key.index) << 16) + std::hash<std::type_index>{}(key.type);
            }
        };
    };

    struct Value {
        void *data = 0;
        std::deque<CallbackMemory> callbacks;
    };

public:
    IndexConnector(game::GameContext &context);

    void tick(game::TickerContext &tickerContext);

    template <typename DataType>
    void provide(std::uint64_t index, DataType &data) {
        static_assert(sizeof(CallbackType<DataType>::type) == callbackMemorySize, "Sizeof different std::functions are different!");

        Value &val = registry[Key(index, std::type_index(typeid(DataType)))];
        assert(!val.data);
        val.data = static_cast<void *>(&data);

        std::deque<CallbackMemory>::const_iterator i = val.callbacks.cbegin();
        while (i != val.callbacks.cend()) {
            (*reinterpret_cast<typename CallbackType<DataType>::type *>(i->mem))(data);
            i++;
        }
    }

    template <typename DataType>
    void request(std::uint64_t index, typename CallbackType<DataType>::type callback) {
        static_assert(sizeof(CallbackType<DataType>::type) == callbackMemorySize, "Sizeof different std::functions are different!");

        Value &val = registry[Key(index, std::type_index(typeid(DataType)))];
        if (val.data) {
            callback(*static_cast<DataType *>(val.data));
        } else {
            val.callbacks.emplace_back();
            *reinterpret_cast<typename CallbackType<DataType>::type *>(val.callbacks.back().mem) = std::move(callback);
        }
    }

private:
    std::unordered_map<Key, Value, Key::Hasher> registry;
};

}
