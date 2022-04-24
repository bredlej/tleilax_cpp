//
// Created by geoco on 15.04.2022.
//

#ifndef TLEILAX_CORE_H
#define TLEILAX_CORE_H

#include <entt/entt.hpp>
#include <pcg/pcg_random.hpp>
#include <name_generator.h>

struct UIView {
    virtual void render() = 0;
    virtual void update() = 0;
};

class Core {
public:
    entt::dispatcher dispatcher{};
    entt::registry registry{};
    pcg32 pcg;
    NameGenerator name_generator{};
};
#endif//TLEILAX_CORE_H
