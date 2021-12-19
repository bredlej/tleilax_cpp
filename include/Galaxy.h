//
// Created by geoco on 19.12.2021.
//

#ifndef TLEILAX_GALAXY_H
#define TLEILAX_GALAXY_H

#include <pcg/pcg_random.hpp>
#include <components.h>
#include <cstdint>
#include <entt/entt.hpp>
class Galaxy {
public:
    Galaxy() = default;
    ~Galaxy() = default;

    void render();
    entt::registry registry;
    void recalculate();
private:
    struct {
        float x, y, z;
    } _offset{0.,0.,0.};
    struct {
        size_t x, y, z;
    } _visible_size{50, 50, 50};
    int64_t _star_chance = 1000;
};


#endif//TLEILAX_GALAXY_H
