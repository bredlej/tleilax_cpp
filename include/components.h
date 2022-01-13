//
// Created by geoco on 19.12.2021.
//

#ifndef TLEILAX_COMPONENTS_H
#define TLEILAX_COMPONENTS_H
#include <cstdint>

struct Vector3;

struct Size {
    float size;
};
struct StarColor{
    uint8_t r, g, b, a;
};
struct Exploding {
    uint8_t counter;
};
struct Nova {};
struct NovaSeeker {
    uint32_t capacity;
};
struct Fleet {
    int i;
};

struct Coordinates {
    int32_t x, y, z;
};

struct Destination {
    Coordinates dest;
};
#endif//TLEILAX_COMPONENTS_H
