#include "meshupdater.h"

#include "spdlog/logger.h"

#include "voro++-0.4.6/src/cell.hh"

#include "schemas/config_client_generated.h"
#include "util/pool.h"
#include "render/hashtreevertindex.h"
#include "render/camera.h"
#include "spatial/raycastlookuptable.h"
#include "graphics/imgui.h"
#include "render/imguirenderer.h"

static constexpr unsigned int delayCellUpdateQueueSentinelSizeLog2 = 100;

namespace render {

MeshUpdater::MeshUpdater(game::GameContext &context, const SsProtocol::Config::MeshGenerator *config)
    : TickableBase(context)
    , facesVecManager(context.get<util::SmallVectorManager<unsigned int>>())
    , meshHandle(context.get<render::SceneManager>().createMesh())
    , cellUpdatesPerTick(config->cell_updates_per_tick())
    , cellUpdateQueue(cellKeyComparator)
{
    // Add sentinel values
    if (!config->ungenerated_retry_delay()) {
        context.get<spdlog::logger>().warn("Client config mesh_generator.ungenerated_retry_delay is zero; this is likely to cause performance problems. Set it to 16 or so.");
    }
    for (unsigned int i = 0; i < config->ungenerated_retry_delay(); i++) {
        delayedCellUpdateQueue.emplace();
        delayedCellUpdateQueue.back().cellCoord = spatial::UintCoord(0);
        delayedCellUpdateQueue.back().sizeLog2 = delayCellUpdateQueueSentinelSizeLog2;
    }
}

MeshUpdater::~MeshUpdater() {
    SceneManager::VertBuffer &vertBuffer = meshHandle.getSceneManager().getVertBuffer();
    for (std::size_t i = 0; i < vertBuffer.getExtentSize(); i++) {
        SceneManager::VertReader vert = vertBuffer.read(i);
        if (vert.shared.meshIndex == meshHandle.getMeshIndex()) {
            vert.local.facesVec.release(facesVecManager);
        }
    }
}

void MeshUpdater::tick(game::TickerContext &tickerContext) {
    tickerContext.get<render::ImguiRenderer::Ticker>();

    if (ImGui::Begin("Debug")) {
        ImGui::Text("Cell update / delay queue size = %zu / %zu", cellUpdateQueue.size(), delayedCellUpdateQueue.size());
    }
    ImGui::End();

    SceneManager::MeshMutator meshMutator = meshHandle.mutateMesh();
    meshMutator.shared.transform = context.get<render::Camera>().getTransform();

    cellKeyComparator.center = spatial::UintCoord::fromPoint(context.get<render::Camera>().getEyePos());

    unsigned int remainingCellUpdates = cellUpdatesPerTick;
//    unsigned int remainingCellUpdates = 0;
    while (remainingCellUpdates > 0) {
        remainingCellUpdates--;

        assert(!delayedCellUpdateQueue.empty());

        spatial::CellKey front = delayedCellUpdateQueue.front();
        delayedCellUpdateQueue.pop();

        if (front.sizeLog2 == delayCellUpdateQueueSentinelSizeLog2) {
            delayedCellUpdateQueue.push(front);
            break;
        } else {
            updateCell(front);
        }
    }

    while (remainingCellUpdates > 0) {
        remainingCellUpdates--;

        if (cellUpdateQueue.empty()) {
            break;
        }

        spatial::CellKey front = cellUpdateQueue.top();
        cellUpdateQueue.pop();
        updateCell(front);
    }
}

/*
void MeshUpdater::finishMeshGen(MeshGenRequest *meshGenRequest) {
    static thread_local std::unordered_map<glm::vec3, unsigned int> vertIndexMap;
    assert(vertIndexMap.empty());

    for (const MeshGenRequest::Face &faceReq : meshGenRequest->getDstFacesArray()) {
        vertIndexMap.emplace(faceReq.vertPositions[0], static_cast<unsigned int>(-1));
        vertIndexMap.emplace(faceReq.vertPositions[1], static_cast<unsigned int>(-1));
        vertIndexMap.emplace(faceReq.vertPositions[2], static_cast<unsigned int>(-1));
    }

    auto createFunc = [](SceneManager::VertMutator vert) {
        vert.shared.setNormal(glm::vec3(0.0f));
        vert.shared.setColor(188,143,143, 255);
    };

    auto destroyFunc = [this](SceneManager::VertReader vert) {
        unsigned int *faces = vert.local.facesVec.data(facesVecManager);
        unsigned int numFaces = vert.local.facesVec.size();

        for (unsigned int i = 0; i < numFaces; i++) {
            unsigned int f = faces[i];
            const unsigned int *verts = meshHandle.readFace(f).shared.verts;
            for (unsigned int j = 0; j < 3; j++) {
                unsigned int v = verts[j];
                if (v != vert.index) {
                    meshHandle.readVert(v).local.facesVec.remove(facesVecManager, f);
                }
            }
            meshHandle.destroyFace(f);
        }

        vert.local.facesVec.release(facesVecManager);
    };

    glm::vec3 min = meshGenRequest->getRequestAabbMin();
    glm::vec3 max = meshGenRequest->getRequestAabbMax();
    context.get<HashTreeVertIndex>().updateRegion(min, max, vertIndexMap, createFunc, destroyFunc);

    for (const MeshGenRequest::Face &faceReq : meshGenRequest->getDstFacesArray()) {
        SceneManager::FaceMutator face = meshHandle.createFace();
        for (unsigned int i = 0; i < 3; i++) {
            std::unordered_map<glm::vec3, unsigned int>::const_iterator found = vertIndexMap.find(faceReq.vertPositions[i]);
            assert(found != vertIndexMap.cend());
            face.shared.verts[i] = found->second;
            meshHandle.readVert(found->second).local.facesVec.push_back(facesVecManager, face.index);
        }
    }

    vertIndexMap.clear();
}
*/

void MeshUpdater::enqueueCellUpdate(spatial::CellKey cellKey) {
//    assert(false);
//    context.get<world::HashTreeWorld>().getNeedsRegen(coord) = true;

    cellUpdateQueue.push(cellKey);
}

void MeshUpdater::updateCell(spatial::CellKey cellKey) {
    cellKey.storeExtra(4);
}

}
