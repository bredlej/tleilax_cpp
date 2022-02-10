//
// Created by geoco on 19.12.2021.
//

#ifndef TLEILAX_GALAXY_H
#define TLEILAX_GALAXY_H

#include <components.h>
#include <cstdint>
#include <entt/entt.hpp>
#include <pcg/pcg_random.hpp>
#include <raylib.h>
#include <raymath.h>
#include <chrono>
#include <functional>

struct ExplosionEvent {
    entt::entity e;
};
struct NovaSeekEvent {
    entt::entity e;
    Vector3 destination;
};

struct Chance {
    uint32_t upper_bound;
    uint32_t occurs_if_less_then;
};

class StarEntity {
public:
    StarEntity(const uint32_t occurence_chance, const Chance &&exploding_chance, const Chance &&nova_seeker_chance)
        : _entity { entt::null },
          _occurence_chance {occurence_chance},
          _exploding_chance {exploding_chance},
          _nova_seeker_chance {nova_seeker_chance}
          {};

    entt::entity create_at(entt::registry &, pcg32 &, Vector3 position);
    bool is_created();
    static void render(const entt::registry &, const Vector3 &, const entt::entity entity, const Vector3 &coords, const StarColor color, const Size size);
private:
    entt::entity _entity;
    uint32_t _occurence_chance;
    Chance _exploding_chance;
    Chance _nova_seeker_chance;
};

static constexpr uint32_t MAX_SHIPS_IN_FLEET = 10;

class FleetEntity {
public:
    FleetEntity() : _entity{ entt::null } {};
    static entt::entity create(entt::registry&, pcg32&, Vector3);
    void react_to_nova(entt::registry &, pcg32 &, const NovaSeekEvent &);
    static void update(entt::entity entity, Fleet &fleet, Vector3 &pos, Destination destination, Size size);
    static void on_click(const entt::registry &, entt::entity entity);
private:
    entt::entity _entity;
    static void populate_fleet_with_ships(entt::registry &, entt::entity, pcg32 &);
};

class Galaxy {
public:
    Galaxy() : _camera(_initialize_camera({0., 0., 0.}, 110., 10., 90., 90.)) { _initialize(); };
    ~Galaxy() = default;

    void render() const;
    void update();
    void populate();  
    uint32_t next_random_number(const uint32_t max) { return max > 0 ? _pcg(max) : 0; };
private:
    entt::dispatcher _dispatcher{};
    entt::registry _registry{};    
    pcg32 _pcg;
    Vector3 _offset{0., 0., 0.};
    const Vector3 _visible_size{75, 50, 75};
    static constexpr uint32_t _star_occurence_chance = 5000;
    void _initialize();
    Camera _camera;
    Camera _initialize_camera(const Vector3 &cameraInitialPosition, const float cameraDistance,
                              const float horizontalDistance, const float horizontalAngle,
                              const float verticalAngle);

    void _render_visible() const;
    void _tick();
    void _explode_stars(const ExplosionEvent &);
    void _send_fleet_to_nova(const NovaSeekEvent &);
    std::function<void(const entt::registry &, const entt::entity)> _fleet_onclick_handle;
};

#endif//TLEILAX_GALAXY_H
