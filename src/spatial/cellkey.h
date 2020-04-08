#pragma once

#include <cstdlib>

#include "graphics/glm.h"
#include <glm/vec3.hpp>

#include "spatial/uintcoord.h"

namespace spatial {

struct CellKey {
    // TODO: Remove, used for debugging
    CellKey() {}
    CellKey(UintCoord cellCoord, unsigned int sizeLog2)
        : cellCoord(cellCoord)
        , sizeLog2(sizeLog2)
    {}

    bool isNearby(CellKey other) const {
        assert(sizeLog2 == other.sizeLog2);
        return true
                && std::abs(static_cast<UintCoord::SignedAxisType>(cellCoord.x - other.cellCoord.x)) <= 1
                && std::abs(static_cast<UintCoord::SignedAxisType>(cellCoord.y - other.cellCoord.y)) <= 1
                && std::abs(static_cast<UintCoord::SignedAxisType>(cellCoord.z - other.cellCoord.z)) <= 1;
    }

    UintCoord cellCoord;
    unsigned int sizeLog2; // TODO: Try unsigned char


    static CellKey makeRoot() {
        CellKey root;
        root.cellCoord.x = 0;
        root.cellCoord.y = 0;
        root.cellCoord.z = 0;
        root.sizeLog2 = UintCoord::maxSizeLog2;
        return root;
    }

    static CellKey fromCoord(UintCoord coord, unsigned int sizeLog2) {
        assert(sizeLog2 <= UintCoord::maxSizeLog2);

        CellKey res;
        res.cellCoord.x = coord.x >> sizeLog2;
        res.cellCoord.y = coord.y >> sizeLog2;
        res.cellCoord.z = coord.z >> sizeLog2;
        res.sizeLog2 = sizeLog2;
        return res;
    }

    static CellKey fromCoords(UintCoord coord1, UintCoord coord2) {
        for (unsigned int i = 0; i <= UintCoord::maxSizeLog2; i++) {
            CellKey candidate = fromCoord(coord1, i);
            if (candidate.contains(coord2)) {
                return candidate;
            }
        }

        assert(false);
#ifdef NDEBUG
        __builtin_unreachable();
#endif

        /*
        if (coord1 == coord2) {
            CellKey res;
            res.cellCoord = coord1;
            res.sizeLog2 = 0;
            return res;
        }

        (coord1.x ^ coord2.x) | (coord1.y ^ coord2.y) | (coord1.z ^ coord2.z)
        */
    }

    static UintCoord maskCoord(UintCoord coord, unsigned int sizeLog2) {
        UintCoord::AxisType mask = ~((static_cast<UintCoord::AxisType>(1) << sizeLog2) - 1);
        UintCoord res;
        res.x = coord.x & mask;
        res.y = coord.y & mask;
        res.z = coord.z & mask;
        return res;
    }

    bool operator==(const CellKey &other) const {
        return cellCoord == other.cellCoord && sizeLog2 == other.sizeLog2;
    }
    bool operator!=(const CellKey &other) const {
        return cellCoord != other.cellCoord || sizeLog2 != other.sizeLog2;
    }

    bool operator<(const CellKey &other) const {
        if (cellCoord.x != other.cellCoord.x) {return cellCoord.x < other.cellCoord.x;}
        if (cellCoord.y != other.cellCoord.y) {return cellCoord.y < other.cellCoord.y;}
        if (cellCoord.z != other.cellCoord.z) {return cellCoord.z < other.cellCoord.z;}
        if (sizeLog2 != other.sizeLog2) {return sizeLog2 < other.sizeLog2;}
        return false;
    }

    CellKey parent() const {
        CellKey res;
        res.cellCoord.x = cellCoord.x >> 1;
        res.cellCoord.y = cellCoord.y >> 1;
        res.cellCoord.z = cellCoord.z >> 1;
        res.sizeLog2 = sizeLog2 + 1;
        assert(res.sizeLog2 <= UintCoord::maxSizeLog2);
        return res;
    }

    template <unsigned int levels>
    CellKey grandParent() const {
        CellKey res;
        res.cellCoord.x = cellCoord.x >> levels;
        res.cellCoord.y = cellCoord.y >> levels;
        res.cellCoord.z = cellCoord.z >> levels;
        res.sizeLog2 = sizeLog2 + levels;
        assert(res.sizeLog2 <= UintCoord::maxSizeLog2);
        return res;
    }

    template <bool dx, bool dy, bool dz>
    CellKey child() const {
        assert(sizeLog2 >= 1);
        CellKey res;
        res.cellCoord.x = (cellCoord.x << 1) | dx;
        res.cellCoord.y = (cellCoord.y << 1) | dy;
        res.cellCoord.z = (cellCoord.z << 1) | dz;
        res.sizeLog2 = sizeLog2 - 1;
        assert(res.sizeLog2 <= UintCoord::maxSizeLog2);
        return res;
    }

    template <unsigned int levels>
    CellKey grandChild(UintCoord::AxisType dx, UintCoord::AxisType dy, UintCoord::AxisType dz) const {
        assert(sizeLog2 >= levels);
        CellKey res;
        res.cellCoord.x = (cellCoord.x << levels) + dx;
        res.cellCoord.y = (cellCoord.y << levels) + dy;
        res.cellCoord.z = (cellCoord.z << levels) + dz;
        res.sizeLog2 = sizeLog2 - levels;
        assert(res.sizeLog2 <= UintCoord::maxSizeLog2);
        return res;
    }

    template <signed int dx, signed int dy, signed int dz>
    CellKey sibling() const {
        CellKey res;
        res.cellCoord.x = cellCoord.x + dx;
        res.cellCoord.y = cellCoord.y + dy;
        res.cellCoord.z = cellCoord.z + dz;
        res.sizeLog2 = sizeLog2;
        return res;
    }

    CellKey sibling(signed int dx, signed int dy, signed int dz) const {
        CellKey res;
        res.cellCoord.x = cellCoord.x + dx;
        res.cellCoord.y = cellCoord.y + dy;
        res.cellCoord.z = cellCoord.z + dz;
        res.sizeLog2 = sizeLog2;
        return res;
    }

    template <signed int dx, signed int dy, signed int dz>
    UintCoord siblingCoord() const {
        UintCoord res = getCoord<(dx > 0), (dy > 0), (dz > 0)>();
        if (dx < 0) {res.x--;}
        if (dy < 0) {res.y--;}
        if (dz < 0) {res.z--;}
        return res;
    }

    UintCoord::AxisType getSize() const {
        return static_cast<UintCoord::AxisType>(1) << sizeLog2;
    }

    template <signed int dx, signed int dy, signed int dz>
    UintCoord getCoord() const {
        UintCoord res;
        res.x = (cellCoord.x + dx) << sizeLog2;
        res.y = (cellCoord.y + dy) << sizeLog2;
        res.z = (cellCoord.z + dz) << sizeLog2;
        return res;
    }

    UintCoord getCoord(signed int dx, signed int dy, signed int dz) const {
        UintCoord res;
        res.x = (cellCoord.x + dx) << sizeLog2;
        res.y = (cellCoord.y + dy) << sizeLog2;
        res.z = (cellCoord.z + dz) << sizeLog2;
        return res;
    }

    glm::vec3 getPoint(glm::vec3 inner) const {
        glm::vec3 point = getCoord<0, 0, 0>().toPoint();
        point.x += std::ldexp(inner.x, sizeLog2);
        point.y += std::ldexp(inner.y, sizeLog2);
        point.z += std::ldexp(inner.z, sizeLog2);
        return point;
    }

    bool contains(UintCoord coord) const {
        return true
                && coord.x >> sizeLog2 == cellCoord.x
                && coord.y >> sizeLog2 == cellCoord.y
                && coord.z >> sizeLog2 == cellCoord.z;
    }

    bool surfaceContains(UintCoord coord) const {
        return true
                && (coord.x == cellCoord.x << sizeLog2 || coord.x == (cellCoord.x + 1) << sizeLog2)
                && (coord.y == cellCoord.y << sizeLog2 || coord.y == (cellCoord.y + 1) << sizeLog2)
                && (coord.z == cellCoord.z << sizeLog2 || coord.z == (cellCoord.z + 1) << sizeLog2);
    }

    glm::vec3 constrainPointInside(glm::vec3 point) const {
        glm::vec3 min = getCoord<0, 0, 0>().toPoint();
        glm::vec3 max = getCoord<1, 1, 1>().toPoint();

        if (point.x < min.x) {point.x = min.x;}
        else if (point.x > max.x) {point.x = max.x;}
        if (point.y < min.y) {point.y = min.y;}
        else if (point.y > max.y) {point.y = max.y;}
        if (point.z < min.z) {point.z = min.z;}
        else if (point.z > max.z) {point.z = max.z;}

        return point;
    }

    /*
    template <signed int x, signed int y, signed int z>
    CellKey plus() const {
        CellKey res;
        res.coord.x = coord.x + x;
        res.coord.y = coord.y + y;
        res.coord.z = coord.z + z;
        res.sizeLog2 = sizeLog2;
        return res;
    }
    */
};

struct CellKeyHasher {
    std::size_t operator()(CellKey cellKey) const {
        // This hash function should only be used right before a prime modulus
        // If right before a power of two modulus, need to do proper hashing here
        std::size_t res = 0;
        res ^= static_cast<std::size_t>(cellKey.cellCoord.x) << 0;
        res ^= static_cast<std::size_t>(cellKey.cellCoord.y) << 16;
        res ^= static_cast<std::size_t>(cellKey.cellCoord.z) << 32;
        res ^= static_cast<std::size_t>(cellKey.sizeLog2) << 48;
        return res;
    }
};

}
