#pragma once

#include <cstdint>

namespace world {

class VoronoiCell;
class Particle;

class Bond {
public:
    Particle *particle;
    float weight;
};

class Particle {
public:
    // Target center is simply the weighted average of bonds
    // Should nudge velocity towards average
    Bond bonds[8];
    VoronoiCell *cell;
};

}
