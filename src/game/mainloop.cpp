#include "mainloop.h"

#include <unistd.h>

#include "spdlog/logger.h"
#include "schemas/config_client_generated.h"
#include "schemas/config_game_generated.h"
#include "game/gamecontext.h"
#include "game/tickercontext.h"
#include "util/testrunner.h"
#include "application/callqueue.h"
#include "application/syncpoint.h"
#include "application/signalhandler.h"
#include "application/window.h"
#include "render/scenerenderer.h"
#include "render/meshupdater.h"
#include "game/cameraflycontroller.h"
#include "network/connectionmanager.h"
#include "render/raycaster.h"
#include "pointgen/cubiclatticegenerator.h"
#include "pointgen/rsquaredlatticegenerator.h"
#include "worldgen/simplegenerator.h"
#include "worldgen/externalgenerator.h"
#include "network/baseconnection.h"
#include "game/digger.h"
#include "material/materialeditor.h"
#include "world/hashtreeworld.h"
#include "util/refset.h"
#include "render/depthbufferprocessor.h"

namespace game {

MainLoop::MainLoop(GameContext &context)
    : context(context)
{}

void MainLoop::load() {
    const SsProtocol::Config::Client &clientConfig = context.get<const SsProtocol::Config::Client>();
    const SsProtocol::Config::Game &gameConfig = context.get<const SsProtocol::Config::Game>();

    context.get<application::CallQueue>();
    context.get<application::SyncPoint>();

    if (clientConfig.test_runner()) {
        util::TestRunner::getInstance().run(clientConfig.test_runner());
    }
    if (clientConfig.signal_handler()) {
        context.construct<application::SignalHandler>(clientConfig.signal_handler());
    }
    if (clientConfig.window()) {
        context.construct<application::Window>(clientConfig.window());
    }
    if (clientConfig.render()) {
        context.construct<render::SceneRenderer>(clientConfig.render());
    }
    if (clientConfig.mesh_generator()) {
        context.construct<render::MeshUpdater>(clientConfig.mesh_generator());
    }
    if (clientConfig.depth_buffer_processor()) {
        context.construct<render::DepthBufferProcessor>(clientConfig.depth_buffer_processor());
    }
    if (clientConfig.hash_tree_world()) {
        context.construct<world::HashTreeWorld>(clientConfig.hash_tree_world());
    }
    if (clientConfig.network()) {
        context.construct<network::ConnectionManager>(clientConfig.network());
    }

    switch (gameConfig.lattice_generator_type()) {
        case SsProtocol::Config::LatticeGenerator_NONE:
            context.get<spdlog::logger>().warn("No lattice_generator property in game config. You will probably get a fatal error later on.");
            break;
        case SsProtocol::Config::LatticeGenerator_CubicLatticeGenerator:
            context.construct<pointgen::PointGenerator, pointgen::CubicLatticeGenerator>(gameConfig.lattice_generator_as_CubicLatticeGenerator());
            break;
        case SsProtocol::Config::LatticeGenerator_RSquaredLatticeGenerator:
            context.construct<pointgen::PointGenerator, pointgen::RSquaredLatticeGenerator>(gameConfig.lattice_generator_as_RSquaredLatticeGenerator());
            break;
    }

    switch (gameConfig.world_generator_type()) {
        case SsProtocol::Config::WorldGenerator_NONE:
            context.get<spdlog::logger>().warn("No world_generator property in game config. You will probably get a fatal error later on.");
            break;
        case SsProtocol::Config::WorldGenerator_SimpleWorldGenerator:
            context.construct<worldgen::WorldGenerator, worldgen::SimpleGenerator>(gameConfig.world_generator_as_SimpleWorldGenerator());
            break;
        case SsProtocol::Config::WorldGenerator_ExternalWorldGenerator:
            context.construct<worldgen::WorldGenerator, worldgen::ExternalGenerator>(gameConfig.world_generator_as_ExternalWorldGenerator());
            break;
    }

    context.get<game::CameraFlyController>();
    context.get<render::RayCaster>();
    context.get<game::Digger>();
    context.get<material::MaterialEditor>();
}

void MainLoop::run() {
//    case  0: context.get<render::ImguiRenderer>().tickOpen(); break;
//    case  1: game::Debug::getInstance().tick(); break;
//    case  2: context.get<network::GameSynchronizer>().tick(); break;
//    case  3: context.get<physics::Physics>().advance(ticks * 0.01f); break;
//    case  4: context.get<simulation::BlobTicker>().tick(); break;
//    case  5: /* context.get<spheretree::SphereTree>().advanceTo(time); */ break;
//    case  6: context.get<render::CameraFlyController>().tick(); break;
//    case  7: context.get<render::Camera>().tick(); break;
//    case 10: context.get<render::Menu>().tick(); break;
//#if WORLDGEN_SIMPLE_ENABLED
//    case 11: context.get<worldgen::SimpleWorldgenDriver>().tick(); break;
//#else
//    case 12: context.get<skygrid::SkyGrid>().tick(); break;
//#endif
//    case 13: context.get<CallQueue>().run(); break;

//    case 14: context.get<game::Digger>().tick(); break;
//    case 20: context.get<game::WorldBuilder>().tick(); break;
//    case 21: context.get<render::VisibilityManager>().tick(1.0f); break;
//    case 22: context.get<game::ObjectManager>().tick(); break;

//    case 23: if (ticks % 256 == 0) {context.get<render::SceneValidator>().validate();} break;

//    case 24: context.get<util::RefList<render::IcosphereMesh>>().callEach<&render::IcosphereMesh::tick>(); break;

//    case 25: context.get<render::SphereTreeRenderer>().startRender(); break;
//    case 26: context.get<render::SphereTreeRenderer>().finishRender(); break;
//    case 27: context.get<inventory::Inventory>().render(); break;
//    case 28: context.get<inventory::ObjectPlacer>().tick(); break;
//    case 29: context.get<wfc::SampleRecorder>().tick(); break;
//#if !WORLDGEN_SIMPLE_ENABLED
//    case 30: context.get<skygrid::SkyGridRenderer>().processFeedback(); break;
//    case 31: context.get<skygrid::SkyGridRenderer>().render(); break;
//#endif
//    case 32: context.get<render::ImguiRenderer>().tickClose(); break;
//    case 33: context.get<Window>().updateFrame(); break;
//    case 34: /* context.get<GameRecorder>().takeSnapshot(); */ break;
//    case 40: context.get<Window>().pollEvents(); break;

    load();

    while (true) {
        context.get<game::TickerContext>().tick();
//        usleep(100000);
        curTime++;
    }
}

}
