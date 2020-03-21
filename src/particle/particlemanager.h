#pragma once

#include "game/tickercontext.h"
#include "particle/particle.h"

namespace game { class GameContext; }

namespace particle {

class ParticleManager : public game::TickerContext::TickableBase<ParticleManager> {
public:
    ParticleManager(game::GameContext &context);

    void addParticle(const Particle &particle);

    void tick(game::TickerContext &tickerContext);

private:
    std::vector<Particle> particles;
};

}
