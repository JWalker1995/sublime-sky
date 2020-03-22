#pragma once

#include <deque>

#include "game/tickercontext.h"
#include "particle/particle.h"

namespace game { class GameContext; }

namespace particle {

class ParticleManager : public game::TickerContext::TickableBase<ParticleManager> {
public:
    ParticleManager(game::GameContext &context);

    Particle &createParticle();
    void addBond(Particle &particle, glm::vec3 point);

    void tick(game::TickerContext &tickerContext);

private:
    std::deque<Particle> particles;
    std::vector<std::pair<Particle *, glm::vec3>> bonds;
};

}
