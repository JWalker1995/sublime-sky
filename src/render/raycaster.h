#pragma once

#include <random>

#include "graphics/glm.h"
#include <glm/vec3.hpp>

#include "game/tickercontext.h"

namespace render {

class RayCaster : public game::TickerContext::TickableBase<RayCaster> {
public:
    RayCaster(game::GameContext &context);

    void tick(game::TickerContext &tickerContext);

private:
    std::mt19937 rngGen;
    int numRays = 16;

    std::vector<std::pair<glm::vec3, glm::vec3>> retryRays;

    void truncateRetries();
    void castRetries();
    void castNew();

    void castRay(glm::vec3 origin, glm::vec3 dir);
};

}
