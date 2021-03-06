//
// Created by geoco on 20.02.2022.
//

#ifndef TLEILAX_SHIP_HPP
#define TLEILAX_SHIP_HPP

#include <components.hpp>
#include <entt/entt.hpp>
#include <pcg/pcg_random.hpp>

template <typename Component, typename ... Components>
class Ship {
public:
    explicit Ship() noexcept : _entity{entt::null} {};
    Ship(const Ship&) noexcept = delete;
    Ship(Ship&&) noexcept = delete;
    Ship& operator=(const Ship&) noexcept = delete;
    Ship& operator=(Ship&&) noexcept = delete;
    ~Ship() noexcept = default;

    static entt::entity create_with_components(entt::registry &, Component, Components...);
    explicit Ship(entt::registry &registry, Component c, Components ... cs) : _entity{ Ship<Component, Components...>::create_with_components(registry, c, cs...) } {};
    entt::entity get();
private:
    [[maybe_unused]] entt::entity _entity;
};

template<typename Component, typename... Components>
entt::entity Ship<Component, Components...>::create_with_components(entt::registry &registry, Component component, Components... components) {
    entt::entity result = registry.create();
    registry.emplace<Component>(result, component);
    (registry.emplace<Components>(result, components), ...);
    return result;
}

template<typename Component, typename... Components>
entt::entity Ship<Component, Components...>::get() {
    return _entity;
}
#endif//TLEILAX_SHIP_HPP
