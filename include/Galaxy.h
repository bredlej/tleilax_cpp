//
// Created by geoco on 19.12.2021.
//

#ifndef TLEILAX_GALAXY_H
#define TLEILAX_GALAXY_H

#include <chrono>
#include <components.h>
#include <cstdint>
#include <entt/entt.hpp>
#include <memory>
#include <pcg/pcg_random.hpp>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>


struct ExplosionEvent {
    entt::entity e;
};
struct NovaSeekEvent {
    entt::entity e;
    Vector3 destination;
};

struct RenderInstance {
    Model model;
    Shader shader;
    std::vector<Matrix> matrices;
    int count {0};
};

class Galaxy {
public:
    Galaxy() : _camera(_initialize_camera({0., 0., 0.}, 110., 10., 90., 90.)),
               _render_instance(_init_render_instance()) { _initialize(); };
    ~Galaxy();

    void render();
    void update();
    void populate();

private:
    entt::dispatcher _dispatcher{};
    entt::registry _registry{};
    RenderInstance _render_instance;
    Vector3 _offset{0., 0., 0.};
    const Vector3 _visible_size{75, 50, 75};
    static constexpr uint32_t _star_occurence_chance = 5000;
    void _initialize();
    Camera _camera;
    Camera _initialize_camera(const Vector3 &cameraInitialPosition, const float cameraDistance,
                              const float horizontalDistance, const float horizontalAngle,
                              const float verticalAngle);

    void _render_visible();
    void _tick();
    void _explode_stars(const ExplosionEvent &);
    void _send_fleet_to_nova(const NovaSeekEvent &);
    static RenderInstance _init_render_instance();
};

#endif//TLEILAX_GALAXY_H
