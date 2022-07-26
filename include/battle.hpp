//
// Created by geoco on 26.07.2022.
//

#ifndef TLEILAX_BATTLE_HPP
#define TLEILAX_BATTLE_HPP

#include <deque>
#include <entt/entt.hpp>
#include <core.hpp>
#include <events.hpp>

namespace battle {
    class Actor {
    public:
        explicit Actor(entt::entity entity) noexcept {}
    private:
        entt::entity entity;
        int32_t ap;
    };

    class Renderer  {

    };

    struct Turn {

    };

    struct Action {

    };

    class Battle : public UIView {
    public:
        explicit Battle(std::shared_ptr<Core> core) noexcept : _core{core} {}
        void render() override;
        void update() override;
    private:
        std::shared_ptr<Core> _core;
    };
}// namespace battle
#endif//TLEILAX_BATTLE_HPP
