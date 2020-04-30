#include "eyepostracker.h"

#include "game/tickercontext.h"
#include "render/camera.h"

namespace {

class EyePosListener : public game::TickerContext::TickableBase<EyePosListener> {
public:
    EyePosListener(game::GameContext &context)
        : TickableBase(context)
    {}

    ~EyePosListener() {
        assert(eyeRefs.empty());
    }

    void tick(game::TickerContext &tickerContext) {
        (void) tickerContext;

        glm::vec3 eyePos = context.get<render::Camera>().getEyePos();
        for (glm::vec3 *ref : eyeRefs) {
            *ref = eyePos;
        }
    }

    std::vector<glm::vec3 *> eyeRefs;
};

}

namespace spatial {

EyePosTracker::EyePosTracker(game::GameContext &context)
    : context(context)
{
    context.get<EyePosListener>().eyeRefs.push_back(&eyePos);
}

EyePosTracker::~EyePosTracker() {
    std::vector<glm::vec3 *> &eyeRefs = context.get<EyePosListener>().eyeRefs;
    std::vector<glm::vec3 *>::iterator found = std::find(eyeRefs.begin(), eyeRefs.end(), &eyePos);
    assert(found != eyeRefs.end());
    *found = eyeRefs.back();
    eyeRefs.pop_back();
}

}
