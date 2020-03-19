#pragma once

#include <assert.h>

#include "graphics/circularindex.h"

namespace graphics {

template <typename DataType, unsigned int size>
class CircularArray {
public:
    template <typename... ArgTypes>
    void set(ArgTypes... args) {
        set<0, ArgTypes...>(std::forward<ArgTypes>(args)...);
    }

    bool contains(DataType value) const {
        for (unsigned int i = 0; i < size; i++) {
            if (data[i] == value) {
                return true;
            }
        }
        return false;
    }

    CircularIndex<size> find(DataType value) const {
        for (unsigned int i = 0; i < size; i++) {
            if (data[i] == value) {
                return CircularIndex<size>(i);
            }
        }
        assert(false);
        return CircularIndex<size>(static_cast<unsigned int>(-1));
    }

    CircularIndex<size> min() {
        DataType minVal = data[0];
        unsigned int minIndex = 0;
        for (unsigned int i = 1; i < size; i++) {
            if (data[i] < minVal) {
                minVal = data[i];
                minIndex = i;
            }
        }
        return CircularIndex<size>(minIndex);
    }
    CircularIndex<size> max() {
        DataType maxVal = data[0];
        unsigned int maxIndex = 0;
        for (unsigned int i = 1; i < size; i++) {
            if (data[i] > maxVal) {
                maxVal = data[i];
                maxIndex = i;
            }
        }
        return CircularIndex<size>(maxIndex);
    }

    const DataType &operator[](CircularIndex<size> index) const {
        return data[index.get()];
    }
    DataType &operator[](CircularIndex<size> index) {
        return data[index.get()];
    }

private:
    DataType data[size];

    template <unsigned int index, typename FirstArgType, typename... RestArgTypes>
    void set(FirstArgType firstArg, RestArgTypes... restArgs) {
        data[index] = firstArg;
        set<index + 1, RestArgTypes...>(std::forward<RestArgTypes>(restArgs)...);
    }

    template <unsigned int index>
    void set() {
        static_assert(index == size, "Set called with incorrect number of arguments");
    }
};

}
