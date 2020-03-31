#pragma once

#include <assert.h>
#include <stack>
#include <unordered_map>

#include "spatial/cellkey.h"

namespace spatial {

template <typename DerivedType, typename CellValue>
class HashTree {
public:
    typedef std::pair<const CellKey, CellValue> Cell;

    template <typename QueryType, bool returnBranches>
    class Iterator {
    public:
        Iterator(HashTree &hashTree, QueryType query)
            : hashTree(hashTree)
            , query(query)
        {}

        QueryType &getQuery() {
            return query;
        }

        void init(CellKey root) {
            assert(stack.empty());
            stack.push(root);

            loadNext();
        }

        bool has() const {
            return curCell;
        }

        std::pair<const CellKey, CellValue> *get() const {
            assert(has());
            return curCell;
        }

        void advance() {
            assert(has());
            loadNext();
        }

    private:
        HashTree &hashTree;
        QueryType query;
        std::stack<CellKey> stack;
        std::pair<const CellKey, CellValue> *curCell;

        void loadNext() {
            while (!stack.empty()) {
                CellKey top = stack.top();
                stack.pop();

                typename std::unordered_map<CellKey, CellValue, CellKeyHasher>::iterator found = hashTree.cellGrid.find(top);
                if (found == hashTree.cellGrid.cend()) {
                    continue;
                }

                if (found->second.isLeaf()) {
                    curCell = &*found;
                    return;
                } else {
                    CellKey child;

                    child = top.child<0, 0, 0>();
                    if (query.testCellKey(child)) {stack.push(child);}
                    child = top.child<0, 0, 1>();
                    if (query.testCellKey(child)) {stack.push(child);}
                    child = top.child<0, 1, 0>();
                    if (query.testCellKey(child)) {stack.push(child);}
                    child = top.child<0, 1, 1>();
                    if (query.testCellKey(child)) {stack.push(child);}
                    child = top.child<1, 0, 0>();
                    if (query.testCellKey(child)) {stack.push(child);}
                    child = top.child<1, 0, 1>();
                    if (query.testCellKey(child)) {stack.push(child);}
                    child = top.child<1, 1, 0>();
                    if (query.testCellKey(child)) {stack.push(child);}
                    child = top.child<1, 1, 1>();
                    if (query.testCellKey(child)) {stack.push(child);}

                    if (returnBranches) {
                        curCell = &*found;
                        return;
                    }
                }
            }

            curCell = nullptr;
        }
    };

    HashTree() {
        cellGrid.emplace(CellKey::makeRoot(), DerivedType::makeRootBranch());
    }

    template <typename... ExtraArgs>
    Cell *getLeafContaining(UintCoord coord, unsigned int guessSizeLog2, ExtraArgs... args) {
        return getCellContaining<true, true, ExtraArgs...>(coord, guessSizeLog2, std::forward<ExtraArgs>(args)...);
    }

    template <typename... ExtraArgs>
    Cell *getNodeContaining(CellKey cellKey, ExtraArgs... args) {
        return getCellContaining<true, false, ExtraArgs...>(cellKey.getCoord<0, 0, 0>(), cellKey.sizeLog2, std::forward<ExtraArgs>(args)...);
    }

    template <typename... ExtraArgs>
    Cell *findNodeMatching(CellKey cellKey) {
        typename std::unordered_map<CellKey, CellValue, CellKeyHasher>::iterator insert = cellGrid.find(cellKey);
        if (insert == cellGrid.end()) {
            return 0;
        } else {
            return &*insert;
        }
    }

    std::unordered_map<CellKey, CellValue, CellKeyHasher> &getMap() {
        return cellGrid;
    }

private:
    template <bool recurseTowardsRoot, bool recurseTowardsLeaves, typename... ExtraArgs>
    Cell *getCellContaining(UintCoord coord, unsigned int guessSizeLog2, ExtraArgs... args) {
        std::pair<typename std::unordered_map<CellKey, CellValue, CellKeyHasher>::iterator, bool> insert = cellGrid.emplace(CellKey::fromCoord(coord, guessSizeLog2), CellValue());
        if (insert.second) {
            if (recurseTowardsRoot) {
                Cell *parent = getCellContaining<true, false, ExtraArgs...>(coord, guessSizeLog2 + 1, std::forward<ExtraArgs>(args)...);
                if (parent->second.isLeaf()) {
                    // If the ancestor is a leaf, destroy this cell
                    cellGrid.erase(insert.first);
                    return parent;
                } else if (derived()->shouldSubdiv(&*insert.first)) {
                    // If the ancestor is a branch, and this cell should be too
                    derived()->setBranchWithParent(&*insert.first, parent, std::forward<ExtraArgs>(args)...);
                    assert(!insert.first->second.isLeaf());
                    if (recurseTowardsLeaves) {
                        return getCellContaining<false, true, ExtraArgs...>(coord, guessSizeLog2 - 1, std::forward<ExtraArgs>(args)...);
                    } else {
                        return &*insert.first;
                    }
                } else {
                    // If the ancestor is a branch, but this cell should be a leaf
                    derived()->setLeafWithParent(&*insert.first, parent, std::forward<ExtraArgs>(args)...);
                    assert(insert.first->second.isLeaf());
                    return &*insert.first;
                }
            } else if (recurseTowardsLeaves) {
                if (derived()->shouldSubdiv(&*insert.first)) {
                    derived()->setBranch(&*insert.first, std::forward<ExtraArgs>(args)...);
                    assert(!insert.first->second.isLeaf());
                    return getCellContaining<false, true, ExtraArgs...>(coord, guessSizeLog2 - 1, std::forward<ExtraArgs>(args)...);
                } else {
                    derived()->setLeaf(&*insert.first, std::forward<ExtraArgs>(args)...);
                    assert(insert.first->second.isLeaf());
                    return &*insert.first;
                }
            } else {
                if (derived()->shouldSubdiv(&*insert.first)) {
                    // If the ancestor is a branch, and this cell should be too
                    derived()->setBranch(&*insert.first, std::forward<ExtraArgs>(args)...);
                    assert(!insert.first->second.isLeaf());
                    return &*insert.first;
                } else {
                    // If the ancestor is a branch, but this cell should be a leaf
                    derived()->setLeaf(&*insert.first, std::forward<ExtraArgs>(args)...);
                    assert(insert.first->second.isLeaf());
                    return &*insert.first;
                }
            }
        } else {
            if (recurseTowardsLeaves) {
                if (insert.first->second.isLeaf()) {
                    // Happy path
                    return &*insert.first;
                } else {
                    return getCellContaining<false, true, ExtraArgs...>(coord, guessSizeLog2 - 1, std::forward<ExtraArgs>(args)...);
                }
            } else {
                return &*insert.first;
            }
        }
    }

    DerivedType *derived() {
        return static_cast<DerivedType *>(this);
    }

    std::unordered_map<CellKey, CellValue, CellKeyHasher> cellGrid;
};

}
