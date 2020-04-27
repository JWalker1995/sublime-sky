#include "node.h"

#include <array>

#include "game/gamecontext.h"
#include "util/pool.h"

namespace world {

void Node::setBranch(game::GameContext &context) {
    assert(isLeaf);

    isLeaf = false;
    children = context.get<util::Pool<std::array<Node, 8>>>().alloc()->data();

    for (unsigned int i = 0; i < 8; i++) {
        children[i].parent = this;
    }
}

}
