#pragma once

#include "pointgen/jitterer.h"

namespace pointgen {

class RSquaredGenerator : private Jitterer {
public:
    // From http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/

    /*
    TODO: Implement rotation
    def genSeq(g, off, angle, count):
      a1 = 1.0/g
      a2 = 1.0/(g*g)
      xs = (off + a1 * np.arange(0, count)) % 1 - 0.5
      ys = (off + a2 * np.arange(0, count)) % 1 - 0.5
      xs, ys = xs * np.cos(angle) - ys * np.sin(angle), xs * np.sin(angle) + ys * np.cos(angle)
      return xs % 1, ys % 1
    */

    RSquaredGenerator(glm::vec3 size, unsigned int numPoints, float jitterFactor = 0.38f, unsigned int seed = 6487)
        : Jitterer(size, numPoints, jitterFactor, seed)
        , size(size)
        , cur(0.0f)
        , inc(
            size.x / (1.22074408460575947536f),
            size.y / (1.22074408460575947536f * 1.22074408460575947536f),
            size.z / (1.22074408460575947536f * 1.22074408460575947536f * 1.22074408460575947536f)
            ) // x = 1.2207... is the unique positive root of x^(d+1) = x + 1 where d = 3
    {
        advance();
    }

    glm::vec3 get() {
        assert(res.x >= 0.0f);
        assert(res.y >= 0.0f);
        assert(res.z >= 0.0f);
        assert(res.x < size.x);
        assert(res.y < size.y);
        assert(res.z < size.z);

        return res;
    }

    void advance() {
        cur += inc;
        res = cur + getJitter();

        if (res.x < 0.0f) {res.x += size.x;}
        if (res.y < 0.0f) {res.y += size.y;}
        if (res.z < 0.0f) {res.z += size.z;}

        while (res.x >= size.x) {res.x -= size.x; cur.x -= size.x;}
        while (res.y >= size.y) {res.y -= size.y; cur.y -= size.y;}
        while (res.z >= size.z) {res.z -= size.z; cur.z -= size.z;}
    }

    void advanceSteps(unsigned int steps) {
        cur += inc * static_cast<float>(steps);
        cur.x = std::fmod(cur.x, size.x);
        cur.y = std::fmod(cur.y, size.y);
        cur.z = std::fmod(cur.z, size.z);

        res = cur + getJitter();

        if (res.x < 0.0f) {res.x += size.x;}
        if (res.y < 0.0f) {res.y += size.y;}
        if (res.z < 0.0f) {res.z += size.z;}

        if (res.x >= size.x) {res.x -= size.x;}
        if (res.y >= size.y) {res.y -= size.y;}
        if (res.z >= size.z) {res.z -= size.z;}
    }

private:
    glm::vec3 size;
    glm::vec3 cur;
    glm::vec3 inc;
    glm::vec3 res;
};

}
