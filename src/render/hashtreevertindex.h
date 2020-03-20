#pragma once

#include <unordered_map>

#include <glm/gtx/hash.hpp>

#include "spatial/hashtree.h"
#include "util/smallvectormanager.h"
#include "render/scenemanager.h"
#include "query/rectquery.h"

namespace game { class GameContext; }

namespace render {

class CellValue {
public:
    void setToBranch() {
        type = Type::Branch;
        assert(!isLeaf());
    }

    void setToLeaf() {
        type = Type::Leaf;
        assert(isLeaf());
    }

    bool isLeaf() const {
        return type == Type::Leaf;
    }

    util::SmallVectorManager<unsigned int>::Ref verts;

    enum class Type { Branch, Leaf };
    Type type;
};

class HashTreeVertIndex : private spatial::HashTree<HashTreeVertIndex, CellValue> {
    friend class spatial::HashTree<HashTreeVertIndex, CellValue>;

public:
    HashTreeVertIndex(game::GameContext &gameContext);
    ~HashTreeVertIndex();

    template <typename CreateFunc, typename DestroyFunc>
    void updateRegion(glm::vec3 min, glm::vec3 max, std::unordered_map<glm::vec3, unsigned int> &desiredVerts, CreateFunc createFunc, DestroyFunc destroyFunc) {
#ifndef NDEBUG
        std::unordered_map<glm::vec3, unsigned int>::const_iterator i = desiredVerts.cbegin();
        while (i != desiredVerts.cend()) {
            assert(i->second == static_cast<unsigned int>(-1));
            i++;
        }
#endif

        Iterator<query::RectQuery, false> it(*this, query::RectQuery(min, max));

        glm::vec3 size = max - min;
        float spacingEstimate = std::cbrt(size.x * size.y * size.z / (desiredVerts.size() + 1));
        assert(spacingEstimate > 0.0f);

        int exp_2;
        float frac = std::frexp(spacingEstimate, &exp_2);
        if (frac < 0.707f) {
            exp_2--;
        }

        assert(exp_2 >= 0);
        assert(exp_2 < 32);

        spatial::CellKey minKey = spatial::CellKey::fromCoord(spatial::UintCoord::fromPoint(min), exp_2);
        spatial::CellKey maxKey = spatial::CellKey::fromCoord(spatial::UintCoord::fromPoint(max), exp_2);

        spatial::CellKey key;
        for (key.cellCoord.x = minKey.cellCoord.x; key.cellCoord.x <= maxKey.cellCoord.x; key.cellCoord.x++) {
            for (key.cellCoord.y = minKey.cellCoord.y; key.cellCoord.y <= maxKey.cellCoord.y; key.cellCoord.y++) {
                for (key.cellCoord.z = minKey.cellCoord.z; key.cellCoord.z <= maxKey.cellCoord.z; key.cellCoord.z++) {
                    it.init(key);
                    while (it.has()) {
                        util::SmallVectorManager<unsigned int>::Ref &vertsVec = it.get()->second.verts;
                        unsigned int *verts = vertsVec.data(vertsVecManager);
                        std::size_t size = vertsVec.size();
                        for (unsigned int i = 0; i < size;) {
                            SceneManager::VertReader v = meshHandle.readVert(verts[i]);
                            std::unordered_map<glm::vec3, unsigned int>::iterator found = desiredVerts.find(v.shared.getPoint());
                            if (found != desiredVerts.cend() && found->second == static_cast<unsigned int>(-1)) {
                                // Vert is in the map, so it should exist
                                // Set the index, so that (1) it can't be found again, and (2) to report its index back to the caller
                                found->second = v.index;
                                i++;
                            } else {
                                destroyFunc(v);
                                verts[i] = verts[--size];
                                meshHandle.destroyVert(v.index);
                            }
                        }

                        if (size != vertsVec.size()) {
                            vertsVec.resize(vertsVecManager, size);
                        }

                        it.advance();
                    }
                }
            }
        }

        std::unordered_map<glm::vec3, unsigned int>::iterator j = desiredVerts.begin();
        while (j != desiredVerts.end()) {
            SceneManager::VertMutator vert = meshHandle.createVert();
            vert.shared.setPoint(j->first);
            j->second = vert.index;
            getLeafContaining(spatial::UintCoord::fromPoint(j->first), exp_2)->second.verts.push_back(vertsVecManager, vert.index);
            createFunc(vert);
            j++;
        }
    }

private:
    static CellValue makeRootBranch() {
        CellValue res;
        assert(!res.isLeaf());
        return res;
    }
    static void setBranch(Cell *cell) {
        cell->second.setToBranch();
    }
    static void setBranchWithParent(Cell *cell, const Cell *parent) {
        (void) parent;
        setBranch(cell);
    }

    static void setLeaf(Cell *cell) {
        cell->second.setToLeaf();
    }

    static void setLeafWithParent(Cell *cell, const Cell *parent) {
        (void) parent;
        setLeaf(cell);
    }

    static bool shouldSubdiv(Cell *cell) {
        return cell->first.sizeLog2 > 0;
    }

    util::SmallVectorManager<unsigned int> &vertsVecManager;
    SceneManager::MeshHandle meshHandle;
};

}
