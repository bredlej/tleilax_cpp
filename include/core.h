//
// Created by geoco on 15.04.2022.
//

#ifndef TLEILAX_CORE_H
#define TLEILAX_CORE_H

#include <entt/entt.hpp>
#include <pcg/pcg_random.hpp>

struct UIView {
    virtual void render() = 0;
    virtual void update() = 0;
};

class Core {
public:
    entt::dispatcher dispatcher{};
    entt::registry registry{};
    pcg32 pcg;
};
#endif//TLEILAX_CORE_H
