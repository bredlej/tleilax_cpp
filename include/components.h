//
// Created by geoco on 19.12.2021.
//

#ifndef TLEILAX_COMPONENTS_H
#define TLEILAX_COMPONENTS_H
#include <cstdint>

struct Size {
    float size;
};
struct StarColor{
    uint8_t r, g, b, a;
};
struct Exploding {
    uint8_t counter;
};
#endif//TLEILAX_COMPONENTS_H
