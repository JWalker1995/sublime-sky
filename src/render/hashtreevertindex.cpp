#include "hashtreevertindex.h"

#include "game/gamecontext.h"
#include "render/meshupdater.h"

namespace render {

HashTreeVertIndex::HashTreeVertIndex(game::GameContext &context)
    : vertsVecManager(context.get<util::SmallVectorManager<unsigned int>>())
    , meshHandle(context.get<MeshUpdater>().getMeshHandle())
{}

}
