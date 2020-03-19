#pragma once

#include <vector>
#include <algorithm>

#include "graphics/glm.h"
#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>

namespace spatial {

template <typename DataType, DataType nullValue>
class UniformPointIndexer {
public:
    struct Slot {
        DataType data = nullValue;
        glm::vec3 point;
    };

    class Iterator {
        friend class UniformPointIndexer;

    public:
        bool has() const {
            return layerIt != layerEnd;
        }

        const DataType &get() const {
            assert(has());
            return slot->data;
        }

        void advance() {
            assert(has);

            do {
                z++;
                if (z == ez) {
                    z = sz;
                    y++;
                    if (y == ey) {
                        y = sy;
                        x++;
                        if (x == ex) {
                            x = sx;
                            layerIt++;
                            if (layerIt == layerEnd) {
                                return;
                            }
                        }
                    }
                }

                updateSlot();
            } while (glm::distance2(center, slot->point) > radSq);
        }

    private:
        std::vector<Slot *>::const_iterator layerIt;
        std::vector<Slot *>::const_iterator layerEnd;
        const Slot *slot;
        signed int sx;
        signed int sy;
        signed int sz;
        signed int ex;
        signed int ey;
        signed int ez;
        signed int x;
        signed int y;
        signed int z;
        glm::vec3 center;
        float radSq;

        void updateSlot() {
            unsigned int index = x * indexer.sizeYZ + y * indexer.sizeZ + z;
            slot = *layerIt + index;
        }
    };

    UniformPointIndexer(glm::vec3 min, glm::vec3 max, float maxRadius)
        : min(min)
        , max(max)
        , maxRadius(maxRadius)
        , invMaxRadius(1.0f / maxRadius)
        , sizeX((max.x - min.x) * invMaxRadius)
        , sizeY((max.y - min.y) * invMaxRadius)
        , sizeZ((max.z - min.z) * invMaxRadius)
        , sizeYZ(sizeY * sizeZ)
        , sizeXYZ(sizeX * sizeYZ)
    {}

    ~UniformPointIndexer() {
        std::vector<Slot *>::const_iterator i = layers.cbegin();
        while (i != layers.cend()) {
            delete[] *i;
            i++;
        }
    }

    float getMaxRadius() const {
        return maxRadius;
    }

    void insert(Slot entry) {
        assert(entry.point.x >= min.x && entry.point.x < max.x);
        assert(entry.point.y >= min.y && entry.point.y < max.y);
        assert(entry.point.z >= min.z && entry.point.z < max.z);

        glm::vec3 cell = (entry.point - min) * invMaxRadius;
        unsigned int cx = std::floor(cell.x);
        unsigned int cy = std::floor(cell.y);
        unsigned int cz = std::floor(cell.z);
        unsigned int index = cx * sizeYZ + cy * sizeZ + cz;

        std::vector<Slot *>::const_iterator i = layers.cbegin();
        while (i != layers.cend()) {
            Slot *slot = *i + index;
            if (slot->data == nullValue) {
                *slot = entry;
                return;
            }
            i++;
        }

        layers.push_back(new Slot[sizeXYZ]);
        layers.back()[index] = entry;
    }

    Iterator lookup(geometry::Sphere sphere) const {
        assert(sphere.rad <= getMaxRadius());

        Iterator res;
        res.layerIt = layers.cbegin();
        res.layerEnd = layers.cend();

        glm::vec3 cell = (sphere.center - min) * invMaxRadius;
        res.sx = std::floor(cell.x - sphere.rad * invMaxRadius);
        res.sy = std::floor(cell.y - sphere.rad * invMaxRadius);
        res.sz = std::floor(cell.z - sphere.rad * invMaxRadius);
        res.ex = std::ceil(cell.x + sphere.rad * invMaxRadius);
        res.ey = std::ceil(cell.y + sphere.rad * invMaxRadius);
        res.ez = std::ceil(cell.z + sphere.rad * invMaxRadius);
        if (res.sx < 0) {res.sx = 0;}
        if (res.sy < 0) {res.sy = 0;}
        if (res.sz < 0) {res.sz = 0;}
        if (res.ex > sizeX) {res.ex = sizeX;}
        if (res.ey > sizeY) {res.ey = sizeY;}
        if (res.ez > sizeZ) {res.ez = sizeZ;}
        res.x = res.sx;
        res.y = res.sy;
        res.z = res.sz;

        res.center = sphere.center;
        res.radSq = sphere.rad * sphere.rad;

        res.updateSlot();

        return res;
    }

    static void test(unsigned int count = 1000) {
        glm::vec3 min(-6.0f, 5.0f, 23.0f);
        glm::vec3 max(17.0f, 8.0f, 56.0f);
        float maxRadius = 1.4f;

        UniformPointIndexer<unsigned int, static_cast<unsigned int>(-1)> indexer(min, max, maxRadius);

        std::vector<glm::vec3> points;
        points.resize(count);
        for (unsigned int i = 0; i < count; i++) {
            points[i].x = std::uniform_random_distribution<float>(min.x, max.x)(rngGen);
            points[i].y = std::uniform_random_distribution<float>(min.y, max.y)(rngGen);
            points[i].z = std::uniform_random_distribution<float>(min.z, max.z)(rngGen);

            indexer.insert(Slot{i, points[i]});
        }

        std::vector<unsigned int> found;
        for (unsigned int i = 0; i < count; i++) {
            geometry::Sphere sphere;
            sphere.center.x = std::uniform_random_distribution<float>(min.x, max.x)(rngGen);
            sphere.center.y = std::uniform_random_distribution<float>(min.y, max.y)(rngGen);
            sphere.center.z = std::uniform_random_distribution<float>(min.z, max.z)(rngGen);
            sphere.rad = std::uniform_random_distribution<float>(0.0f, maxRadius)(rngGen);

            assert(found.empty());
            UniformPointIndexer<unsigned int, static_cast<unsigned int>(-1)>::Iterator j = indexer.lookup(sphere);
            while (j.has()) {
                found.push_back(j.get());
                j.advance();
            }

            std::sort(found.begin(), found.end());

            for (signed int k = count - 1; k >= 0; k--) {
                bool hit = sphere.contains(points[k]);
                if (found.back() == k) {
                    assert(hit);
                    found.pop_back();
                } else {
                    assert(!hit);
                }
            }

            assert(found.empty());
        }
    }

private:
    std::vector<Slot *> layers;
    glm::vec3 min;
    glm::vec3 max;
    float maxRadius;
    float invMaxRadius;
    unsigned int sizeX;
    unsigned int sizeY;
    unsigned int sizeZ;
    unsigned int sizeYZ;
    unsigned int sizeXYZ;
};

}
