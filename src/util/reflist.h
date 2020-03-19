#pragma once

#include <assert.h>
#include <vector>
#include <algorithm>

#include "game/gamecontext.h"

namespace util {

template <typename Type, typename ContainerType = std::vector<Type *>>
class RefList {
public:
    RefList(game::GameContext &context) {
        (void) context;
    }

    void add(Type *inst) {
        data.push_back(inst);
    }

    void remove(Type *inst) {
        typename ContainerType::const_iterator found = std::find(data.cbegin(), data.cend(), inst);
        assert(found != data.cend());
        data.erase(found);
    }

    template <void (Type::*Method)()>
    void callEach() {
        typename ContainerType::const_iterator i = data.cbegin();
        while (i != data.cend()) {
            ((*i)->*Method)();
            i++;
        }
    }

private:
    ContainerType data;
};

}
