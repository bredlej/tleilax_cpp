//
// Created by geoco on 19.12.2021.
//

#ifndef TLEILAX_GALAXY_H
#define TLEILAX_GALAXY_H

#include <components.h>
#include <cstdint>
#include <cmath>
#include <core.h>
#include <raylib.h>
#include <raymath.h>
#include <chrono>
#include <functional>
#include <fleet.h>
#include <events.h>
#include <path.h>
#include <queue>
#include <graph.h>
#include <utility>
#include <variant>
#include <memory>

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
    static void render(const entt::registry &, const Vector3 &, const entt::entity entity, const Vector3 &coords, const components::Star color, const components::Size size, bool is_selected);
    static void on_click(const entt::registry &, entt::entity);

private:
    entt::entity _entity;
    uint32_t _occurence_chance;
    Chance _exploding_chance;
    Chance _nova_seeker_chance;
};

class Galaxy : public UIView {
public:
    explicit Galaxy(std::shared_ptr<Core> core, Assets &assets)
        : _core {std::move(core)},
          _camera(_initialize_camera({0., 0., 0.}, 110., 10., 90., 90.)),
          _ship_components{assets.get_ship_components()} {_initialize();};
    ~Galaxy() = default;

    void render() override;
    void update() override;
    void populate();
    uint32_t next_random_number(const uint32_t max) { return max > 0 ? _core->pcg(max) : 0; };
private:
    Graph<GraphNode, float, GraphNodeHash, GraphNodeEqualFunc> starGraph;
    std::vector<std::pair<Vector3, Vector3>> paths;
    std::vector<std::pair<Vector3, Vector3>> selected_paths;
    ShipComponentRepository _ship_components;

    std::shared_ptr<Core> _core;

    Vector3 _offset{0., 0., 0.};
    const Vector3 _visible_size{75, 50, 75};
    Path _path;
    static constexpr uint32_t _star_occurence_chance = 5000;
    void _initialize();
    Camera _camera;
    Camera _initialize_camera(const Vector3 &cameraInitialPosition, float cameraDistance,
                              float horizontalDistance, float horizontalAngle,
                              float verticalAngle);

    void _render_visible();
    void _tick();
    void _explode_stars(const ExplosionEvent &);
    void _send_fleet_to_nova(const NovaSeekEvent &);
    void _on_star_selected(const entt::entity);

    void _clear_paths();
    std::function<void(const entt::registry &, const entt::entity)> _fleet_onclick_handle;
};

#endif//TLEILAX_GALAXY_H
