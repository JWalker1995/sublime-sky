#include "meshupdater.h"

#include "util/pool.h"

namespace render {

MeshUpdater::MeshUpdater(game::GameContext &context)
    : TickableBase(context)
{}

void MeshUpdater::tick(game::TickerContext &tickerContext) {
    (void) tickerContext;
}

void MeshUpdater::update(glm::vec3 aabbMin, glm::vec3 aabbMax, const std::vector<glm::vec3> &internalPoints, const std::vector<glm::vec3> &externalPoints) {
    MeshGenRequest *request = context.get<util::Pool<MeshGenRequest>>().alloc(*this, aabbMin, aabbMax, internalPoints, externalPoints);
    context.get<meshgen::MeshGenerator>().generate(request);
}

void MeshUpdater::finishMeshGen(MeshGenRequest *meshGenRequest) {
    assert(false);
}

}
