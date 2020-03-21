#pragma once

#include "spatial/hashtree.h"
#include "game/tickercontext.h"
#include "world/chunk.h"
#include "world/spacestate.h"
#include "worldgen/worldgenerator.h"

namespace pointgen { class Chunk; }

namespace world {

class CellValue {
public:
    void setToBranch() {
        state = SpaceState::SubdividedAsBranch;
        assert(!isLeaf());
    }

    void setToLeaf(SpaceState newState) {
        assert(newState != SpaceState::SubdividedAsBranch);
        assert(newState != SpaceState::SubdividedAsChunk);
        state = newState;
        assert(isLeaf());
    }
    void setToLeaf(Chunk *newChunk) {
        state = SpaceState::SubdividedAsChunk;
        chunk = newChunk;
        assert(isLeaf());
    }

    bool isLeaf() const {
        return state != SpaceState::SubdividedAsBranch;
    }

    // TODO: state could be represented as sentinel values of chunk
    SpaceState state;
    Chunk *chunk;
    pointgen::Chunk *points = 0;
};

class HashTreeWorld : private spatial::HashTree<HashTreeWorld, CellValue>, public game::TickerContext::TickableBase<HashTreeWorld> {
    friend class spatial::HashTree<HashTreeWorld, CellValue>;

public:
    HashTreeWorld(game::GameContext &gameContext);

    void tick(game::TickerContext &tickerContext);

    CellValue &getCellValueContaining(glm::vec3 point);

    struct RaytestResult {
        SpaceState state;
        float pointDistance;
    };
    RaytestResult testRay(glm::vec3 origin, glm::vec3 dir, float distanceLimit);

    SpaceState &getClosestPointState(glm::vec3 point);

    void emitMeshUpdate(glm::vec3 changedMin, glm::vec3 changedMax, float pointSpacing);

private:
    class WorldGenRequest : public worldgen::WorldGenerator::Request {
    public:
        WorldGenRequest(HashTreeWorld &hashTreeWorld, spatial::CellKey cube, const pointgen::Chunk *points, Chunk *dstChunk)
            : hashTreeWorld(hashTreeWorld)
            , cube(cube)
            , points(points)
            , dstChunk(dstChunk)
        {}

        spatial::CellKey getCube() const {
            return cube;
        }
        const pointgen::Chunk *getPoints() const {
            return points;
        }
        Chunk *getDstChunk() const {
            return dstChunk;
        }

        void onComplete(SpaceState chunkState) {
            hashTreeWorld.finishWorldGen(this, chunkState);
        }

    private:
        HashTreeWorld &hashTreeWorld;
        spatial::CellKey cube;
        const pointgen::Chunk *points;
        Chunk *dstChunk;
    };

    static CellValue makeRootBranch() {
        CellValue res;
        res.setToBranch();
        return res;
    }
    static void setBranch(Cell *cell) {
        cell->second.setToBranch();
    }
    static void setBranchWithParent(Cell *cell, const Cell *parent) {
        (void) parent;
        setBranch(cell);
    }

    void setLeaf(Cell *cell) {
        cell->second.setToLeaf(SpaceState::Generating);
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
    void finishWorldGen(const WorldGenRequest *worldGenRequest, SpaceState chunkState);
};

}
