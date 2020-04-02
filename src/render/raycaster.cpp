#include "raycaster.h"

#include "graphics/glm.h"
#include <glm/gtx/norm.hpp>

#include "graphics/imgui.h"
#include "render/imguirenderer.h"
#include "render/camera.h"
#include "render/meshupdater.h"
#include "world/hashtreeworld.h"
#include "world/timemanager.h"

namespace render {

RayCaster::RayCaster(game::GameContext &context)
    : TickableBase(context)
{}

void RayCaster::tick(game::TickerContext &tickerContext) {
    tickerContext.get<render::ImguiRenderer::Ticker>();

    truncateRetries();
    castRetries();
    castNew();
}

void RayCaster::truncateRetries() {
    static constexpr unsigned int retryLimit = 0;

    if (retryLimit > 0) {
        while (retryRays.size() > retryLimit) {
            unsigned int remove = rand() % retryRays.size();
            retryRays[remove] = retryRays.back();
            retryRays.pop_back();
        }
    } else {
        retryRays.clear();
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
    if (ImGui::Begin("Debug")) {
        ImGui::Text("Raycast count:");
        ImGui::SameLine();
        ImGui::RadioButton("0", &numRays, 0);
        ImGui::SameLine();
        ImGui::RadioButton("1", &numRays, 1);
        ImGui::SameLine();
        ImGui::RadioButton("4", &numRays, 4);
        ImGui::SameLine();
        ImGui::RadioButton("16", &numRays, 16);
    }
    ImGui::End();

    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    glm::vec3 origin = context.get<render::Camera>().getEyePos();
    for (unsigned int i = 0; i < numRays; i++) {
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

    if (res.materialIndex == static_cast<world::VoronoiCell::MaterialIndex>(-1)) {
        retryRays.emplace_back(origin + dir * (res.pointDistance - 1.0f), dir);
    } else {
        context.get<world::TimeManager>().incTimeAround(res.hitCoord, 1.0f);
        context.get<render::MeshUpdater>().enqueueCellUpdate(res.hitCoord, false);
    }
}


}
