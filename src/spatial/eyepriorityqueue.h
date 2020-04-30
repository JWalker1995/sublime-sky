#pragma once

#include <queue>

#include "spatial/eyepostracker.h"

#include "graphics/glm.h"
#include <glm/gtx/norm.hpp>

namespace game { class GameContext; }

namespace spatial {

template <typename DataType>
class EyePriorityQueue : private EyePosTracker {
private:
    struct Entry {
        float distanceSq;
        DataType data;

        bool operator<(const Entry &other) const {
            // Sort by least to greatest
            return distanceSq > other.distanceSq;
        }
    };

    std::priority_queue<Entry> queue;

public:
    EyePriorityQueue(game::GameContext &context)
        : EyePosTracker(context)
    {}

    auto top() const {
        return queue.top().data;
    }

    bool empty() const {
        return queue.empty();
    }

    std::size_t size() const {
        return queue.size();
    }

    void push(const DataType &data) {
        Entry entry;
        entry.distanceSq = glm::distance2(data.constrainPointInside(getEyePos()), getEyePos());
        entry.data = data;
        queue.push(entry);
    }

    void pop() {
        queue.pop();
    }
};

}
