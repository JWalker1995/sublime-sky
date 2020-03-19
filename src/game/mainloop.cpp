#include "mainloop.h"

#include <unistd.h>

#include "client_generated.h"
#include "game/gamecontext.h"
#include "game/tickercontext.h"
#include "application/callqueue.h"
#include "application/syncpoint.h"
#include "application/signalhandler.h"
#include "application/window.h"
#include "network/network.h"
#include "render/raycaster.h"
#include "pointgen/cubiclatticegenerator.h"
#include "worldgen/simplegenerator.h"

namespace game {

MainLoop::MainLoop(GameContext &context)
    : context(context)
{}

void MainLoop::load() {
    const config::Client &config = context.get<const config::Client>();

    context.get<application::CallQueue>();
    context.get<application::SyncPoint>();

    if (config.loadSignalHandler()) {
        context.get<application::SignalHandler>();
    }
    if (config.loadWindow()) {
        context.get<application::Window>();
    }

    context.get<render::RayCaster>();
    context.construct<pointgen::PointGenerator, pointgen::CubicLatticeGenerator>();
    context.construct<worldgen::WorldGenerator, worldgen::SimpleGenerator>();
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