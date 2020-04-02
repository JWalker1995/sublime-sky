#pragma once

#include "game/tickercontext.h"
#include "graphics/type/material.h"

namespace flatbuffers { template <typename _> class Vector; }
namespace flatbuffers { template <typename _> struct Offset; }
namespace SsProtocol { struct Material; }

namespace material {

class MaterialManager : public game::TickerContext::TickableBase<MaterialManager> {
public:
    MaterialManager(game::GameContext &context);

    void tick(game::TickerContext &tickerContext);

    unsigned int registerMaterials(const flatbuffers::Vector<flatbuffers::Offset<SsProtocol::Material>> *materials);
};

}
