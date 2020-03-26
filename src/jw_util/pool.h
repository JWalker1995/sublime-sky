#ifndef JWUTIL_POOL_H
#define JWUTIL_POOL_H

#include <vector>

namespace jw_util
{

template <typename Type, std::size_t blockSize = 1024 * 64>
class Pool {
private:
    static constexpr unsigned int floorLog2(unsigned long long x) {
        return sizeof(unsigned long long) * CHAR_BIT - __builtin_clzll(x) - 1;
    }

    static constexpr std::size_t tryNumItems = blockSize < sizeof(Type) ? 1 : blockSize / sizeof(Type);
    static constexpr unsigned int numItemsRoundLog2 = floorLog2(tryNumItems * 1.41421356237309504880168872420969808);
    static constexpr std::size_t numItems = static_cast<std::size_t>(1) << numItemsRoundLog2;

public:
    ~Pool() {
        std::sort(freed.begin(), freed.end());

        std::vector<void *>::const_iterator i = blocks.cbegin();
        while (i != blocks.cend()) {
            Type *b = static_cast<Type *>(*i);
            for (std::size_t j = 0; j < numItems; j++) {
                if (!std::binary_search(freed.cbegin(), freed.cend(), b + j)) {
                    b[j].Type::~Type();
                }
            }

            ::free(*i);

            i++;
        }
    }

    template <typename... ArgTypes>
    Type *alloc(ArgTypes &&... args) {
        if (freed.empty()) {
            std::size_t blockIndex = size++ / numItems;
            if (blockIndex == blocks.size()) {
                blocks.push_back(::malloc(numItems * sizeof(Type)));
            }
            return new(blocks[blockIndex]) Type(std::forward<ArgTypes>(args)...);
        } else {
            Type *res = freed.back();
            freed.pop_back();
            return new (res) Type(std::forward<ArgTypes>(args)...);
        }
    }

    void free(const Type *type) {
        type->Type::~Type();
        freed.push_back(const_cast<Type *>(type));
    }

    void cleanup() {
    }

private:
    std::vector<void *> blocks;
    std::size_t size;
    std::vector<Type *> freed;
};

}

#endif // JWUTIL_POOL_H
