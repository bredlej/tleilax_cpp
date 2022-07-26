//
// Created by Patryk Szczypień on 03/07/2022.
//

#ifndef TLEILAX_STAR_SYSTEM_HPP
#define TLEILAX_STAR_SYSTEM_HPP

#include <core.hpp>
#include <graphics_base.hpp>
#include <components.hpp>
#include <raylib.h>
#include <memory>
#include <utility>

class StarSystem : public UIView {
public:
    explicit StarSystem(const entt::entity entity, std::shared_ptr<Core> core) noexcept
        : _core{std::move(core)},
          _registry{}
    {
        const auto main_star = _create_main_star_entity(entity);
        _populate(_core->get_seed_of(entity), main_star);
    }

    StarSystem() noexcept = delete;
    StarSystem(const StarSystem&) noexcept = delete;
    StarSystem(StarSystem&&) noexcept = delete;
    StarSystem& operator=(const StarSystem&) noexcept = delete;
    StarSystem& operator=(StarSystem&&) noexcept = delete;
    ~StarSystem() noexcept = default;

    void render() override;
    void update() override;
    entt::registry &get_registry() { return _registry; }

private:
    entt::entity _create_main_star_entity(const entt::entity);
    void _populate(uint32_t seed, entt::entity);
    entt::registry _registry;
    std::shared_ptr<Core> _core;
    void _populate_orbits_around_star(const entt::entity &main_star_entity);
    void _populate_planets();
};
#endif//TLEILAX_STAR_SYSTEM_HPP
