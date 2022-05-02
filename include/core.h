//
// Created by geoco on 15.04.2022.
//

#ifndef TLEILAX_CORE_H
#define TLEILAX_CORE_H

#include <entt/entt.hpp>
#include <pcg/pcg_random.hpp>
#include <name_generator.h>
#include <colors.h>
struct UIView {
    virtual void render() = 0;
    virtual void update() = 0;
};

class Core {
public:
    explicit Core(int width, int height) : window{width, height} {};
    entt::dispatcher dispatcher{};
    entt::registry registry{};
    pcg32 pcg;
    NameGenerator name_generator{};
    constexpr static Colors colors;
    struct {int width; int height;} window;
};
#endif//TLEILAX_CORE_H
