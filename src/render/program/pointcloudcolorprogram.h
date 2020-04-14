#pragma once

#include "game/gamecontext.h"

#include "render/program/pointcloudprogram.h"

namespace render {

class PointCloudColorProgram : public PointCloudProgram {
public:
    PointCloudColorProgram(game::GameContext &context);

    virtual void insertDefines(Defines &defines);
};

}
