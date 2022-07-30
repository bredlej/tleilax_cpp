//
// Created by geoco on 26.07.2022.
//

#ifndef TLEILAX_BATTLE_HPP
#define TLEILAX_BATTLE_HPP

#include <core.hpp>
#include <deque>
#include <entt/entt.hpp>
#include <events.hpp>
#include <raylib.h>
#include <memory>

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
        explicit Battle(std::shared_ptr<Core> core, entt::entity attacker, entt::entity opponent) noexcept
            : _core{core},
              _attacker{attacker},
              _opponent{opponent},
              _camera{_initialize_camera({0., 0., 0.}, 101., 10., 90., 90.)}
        {
            _initialize();

        }
        ~Battle() { UnloadModel(_spaceships.model);
            UnloadShader(_spaceships.shader); }
        void render() override;
        void update() override;
    private:
        void _initialize();
        RenderInstance _spaceships;
        std::shared_ptr<Core> _core;
        entt::entity _attacker;
        entt::entity _opponent;
        Camera _camera;
        bool _open_demo{false};
        void _draw_ui();
        Camera _initialize_camera(const Vector3 &cameraInitialPosition, const float cameraDistance, const float horizontalDistance, const float horizontalAngle, const float verticalAngle);
        void _render_visible();
        void _render_grid();
    };
}// namespace battle
#endif//TLEILAX_BATTLE_HPP
