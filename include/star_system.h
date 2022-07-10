//
// Created by Patryk Szczypień on 03/07/2022.
//

#ifndef TLEILAX_STAR_SYSTEM_H
#define TLEILAX_STAR_SYSTEM_H

#include <core.h>
#include <graphics_base.h>
#include <memory>

class StarSystem : public UIView {
public:
    explicit StarSystem(uint32_t seed) {
        _registry = std::make_shared<entt::registry>();
        _populate(seed);
    }
    void render() override;
    void update() override;
    std::shared_ptr<entt::registry> get_registry() { return _registry; }

private:
    void _populate(uint32_t seed);
    std::shared_ptr<entt::registry> _registry;
    std::shared_ptr<Core> _core;
};
#endif//TLEILAX_STAR_SYSTEM_H
