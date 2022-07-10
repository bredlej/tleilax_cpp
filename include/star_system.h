//
// Created by Patryk Szczypień on 03/07/2022.
//

#ifndef TLEILAX_STAR_SYSTEM_H
#define TLEILAX_STAR_SYSTEM_H

#include <core.h>
#include <graphics_base.h>

class StarSystem: public UIView {
public:
    void render() override;
    void update() override;
    void populate(const uint32_t seed);
private:
    entt::registry _registry;
    std::shared_ptr<Core> _core;
};
#endif//TLEILAX_STAR_SYSTEM_H
