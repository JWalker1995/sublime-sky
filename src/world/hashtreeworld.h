#pragma once

#include "spatial/hashtree.h"
#include "game/tickercontext.h"
#include "world/chunk.h"
#include "worldgen/worldgenerator.h"

namespace pointgen { class Chunk; }

namespace world {

class CellValue {
public:
    CellValue()
        : chunkId(nextChunkId)
    {
        nextChunkId += Chunk::size * Chunk::size * Chunk::size;
    }

    bool isLeaf() const {
        return type != Type::Branch;
    }

    static unsigned int nextChunkId;
    unsigned int chunkId;

    enum class Type {
        Branch,
        LeafGenerating,
        LeafConstant,
        LeafChunk,
    };
    Type type;

    VoronoiCell::MaterialIndex constantMaterialIndex;
    Chunk *chunk = 0;
    pointgen::Chunk *points = 0;
    bool needsRegen[Chunk::size][Chunk::size][Chunk::size] = {{{0}}};
};

class HashTreeWorld : private spatial::HashTree<HashTreeWorld, CellValue>, public game::TickerContext::TickableBase<HashTreeWorld> {
    friend class spatial::HashTree<HashTreeWorld, CellValue>;

public:
    template <bool populateState, bool populatePoints>
    class CellIterator {
    public:
        void init(spatial::UintCoord coord) {
        }

        CellValue &getNearby(spatial::UintCoord coord) {
            unsigned int cellIndex = coord.x % Chunk::size;

            coord.x /= Chunk::size;
            coord.y /= Chunk::size;
            coord.z /= Chunk::size;

            coord -= min;
            assert(coord.x < (static_cast<spatial::UintCoord::AxisType>(1) << sizeLog2_x));
            assert(coord.y < (static_cast<spatial::UintCoord::AxisType>(1) << sizeLog2_y));
            assert(coord.z < (static_cast<spatial::UintCoord::AxisType>(1) << sizeLog2_z));

            unsigned int index = (((coord.x << sizeLog2_y) | coord.y) << sizeLog2_z) | coord.z;

            if (!data[index]) {
                fillChunkRef(coord , data[index]);
            }

            return *data[index];
        }

    private:
        HashTreeWorld &hashTreeWorld;

        spatial::UintCoord min;
        CellValue **data;
#ifndef NDEBUG
        unsigned int sizeLog2_x;
#endif
        unsigned int sizeLog2_y;
        unsigned int sizeLog2_z;

        void fillChunkRef(spatial::UintCoord chunkCoord, CellValue *&cellValue) {
            spatial::CellKey cellKey(min + chunkCoord, Chunk::sizeLog2);
            cellValue = hashTreeWorld.findNodeMatching(cellKey);

            if (populateState) {

            }
        }
    };

    HashTreeWorld(game::GameContext &gameContext);

    void tick(game::TickerContext &tickerContext);

    CellValue &getCellValueContaining(spatial::UintCoord coord);
    VoronoiCell::MaterialIndex getMaterialIndex(spatial::UintCoord coord);
    VoronoiCell &getVoronoiCell(spatial::UintCoord coord);
    glm::vec3 getPoint(spatial::UintCoord coord);
    bool &getNeedsRegen(spatial::UintCoord coord);
    unsigned int getCellId(spatial::UintCoord coord);

    struct RaytestResult {
        spatial::UintCoord hitCoord;
        VoronoiCell::MaterialIndex materialIndex;
        float pointDistance;
    };
    RaytestResult testRay(glm::vec3 origin, glm::vec3 dir, float distanceLimit);
    RaytestResult testRaySlow(glm::vec3 origin, glm::vec3 dir, float distanceLimit);

    spatial::UintCoord getContainingCoord(glm::vec3 point);

    void finishWorldGen(spatial::CellKey cube, VoronoiCell::MaterialIndex constantMaterialIndex, Chunk *chunk);

//    void emitMeshUpdate(glm::vec3 changedMin, glm::vec3 changedMax, float pointSpacing);

    bool isTransparent(VoronoiCell::MaterialIndex materialIndex) const;

private:
    static CellValue makeRootBranch() {
        CellValue res;
        res.type = CellValue::Type::Branch;
        return res;
    }
    static void setBranch(Cell *cell) {
        cell->second.type = CellValue::Type::Branch;
    }
    static void setBranchWithParent(Cell *cell, const Cell *parent) {
        (void) parent;
        setBranch(cell);
    }

    void setLeaf(Cell *cell) {
        cell->second.type = CellValue::Type::LeafGenerating;
        requestWorldGen(cell);
    }

    void setLeafWithParent(Cell *cell, const Cell *parent) {
        (void) parent;
        setLeaf(cell);
    }

    bool shouldSubdiv(Cell *cell) const {
        return cell->first.sizeLog2 > Chunk::sizeLog2;
    }

    const pointgen::Chunk *getChunkPoints(Cell *cell);

    void requestWorldGen(Cell *cell);
};

}
