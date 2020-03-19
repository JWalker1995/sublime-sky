#include "raycaster.h"

#include <glm/gtx/norm.hpp>

#include "render/camera.h"
#include "world/hashtreeworld.h"

namespace render {

RayCaster::RayCaster(game::GameContext &context)
    : TickableBase(context)
{}

void RayCaster::tick(game::TickerContext &tickerContext) {
    (void) tickerContext;

    truncateRetries();
    castRetries();
    castNew();
}

void RayCaster::truncateRetries() {
    static constexpr unsigned int retryLimit = 128;

    while (retryRays.size() > retryLimit) {
        unsigned int remove = rand() % retryRays.size();
        retryRays[remove] = retryRays.back();
        retryRays.pop_back();
    }
}

void RayCaster::castRetries() {
    static thread_local std::vector<std::pair<glm::vec3, glm::vec3>> retryingRays;
    retryingRays = std::move(retryRays);
    retryRays.clear();

    std::vector<std::pair<glm::vec3, glm::vec3>>::const_iterator i = retryingRays.cbegin();
    while (i != retryingRays.cend()) {
        castRay(i->first, i->second);
        i++;
    }
}

void RayCaster::castNew() {
    static constexpr unsigned int count = 1;

    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    glm::vec3 origin = context.get<render::Camera>().getEyePos();
    for (unsigned int i = 0; i < count; i++) {
        glm::vec3 dir;
        do {
            dir.x = dist(rngGen);
            dir.y = dist(rngGen);
            dir.z = dist(rngGen);
        } while (glm::length2(dir) > 1.0f);

        castRay(origin, dir);
    }
}

void RayCaster::castRay(glm::vec3 origin, glm::vec3 dir) {
    world::HashTreeWorld::RaytestResult res = context.get<world::HashTreeWorld>().testRay(origin, dir, 100.0f);

    if (res.state == world::SpaceState::Generating) {
        retryRays.emplace_back(origin + dir * (res.pointDistance - 1.0f), dir);
    }
}


}
