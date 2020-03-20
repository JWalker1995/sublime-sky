#include "hashtreevertindex.h"

#include "game/gamecontext.h"
#include "render/meshupdater.h"
#include "query/allquery.h"

namespace render {

HashTreeVertIndex::HashTreeVertIndex(game::GameContext &context)
    : vertsVecManager(context.get<util::SmallVectorManager<unsigned int>>())
    , meshHandle(context.get<MeshUpdater>().getMeshHandle())
{}

HashTreeVertIndex::~HashTreeVertIndex() {
    Iterator<query::AllQuery, false> it(*this, query::AllQuery());
    it.init(spatial::CellKey::makeRoot());
    while (it.has()) {
        it.get()->second.verts.release(vertsVecManager);
        it.advance();
    }
}

}
