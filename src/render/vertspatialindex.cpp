#include "vertspatialindex.h"

#if 0

#include <CGAL/Fuzzy_iso_box.h>

#include "render/meshupdater.h"

namespace render {

VertSpatialIndex::VertSpatialIndex(game::GameContext &context)
    : searchTraits(context.get<render::MeshUpdater>().getMeshHandle())
    , tree(Splitter(), searchTraits)
{}

void VertSpatialIndex::getVertsInside(glm::vec3 min, glm::vec3 max, std::vector<unsigned int> &dstVertIndices) {
    unsigned int a = 0;
    CGAL::Fuzzy_iso_box<SearchTraits> query(min, max, 0.0f, searchTraits);
    tree.search(std::back_inserter(dstVertIndices), query);
}

}

#endif
