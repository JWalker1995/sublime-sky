#pragma once

#include "game/gamecontext.h"

#include "render/program/pointcloudprogram.h"

namespace render {

class PointCloudDepthProgram : public PointCloudProgram {
public:
    PointCloudDepthProgram(game::GameContext &context);

    virtual void insertDefines(Defines &defines);
};

}
