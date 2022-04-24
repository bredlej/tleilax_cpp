//
// Created by geoco on 19.12.2021.
//

#ifndef TLEILAX_GALAXY_H
#define TLEILAX_GALAXY_H

#include <chrono>
#include <cmath>
#include <components.h>
#include <core.h>
#include <cstdint>
#include <events.h>
#include <fleet.h>
#include <functional>
#include <graph.h>
#include <imgui/imgui.h>
#include <imgui/rlImGui.h>
#include <memory>
#include <path.h>
#include <queue>
#include <raylib.h>
#include <raymath.h>
#include <utility>
#include <variant>

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

    entt::entity create_at(entt::registry &, const std::shared_ptr<Core> &, Vector3 position);
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
    static constexpr uint32_t _star_occurence_chance = 7000;
    const Vector3 _visible_size{75, 50, 75};
    Graph<GraphNode, float, GraphNodeHash, GraphNodeEqualFunc> stars_graph;
    std::vector<std::pair<Vector3, Vector3>> stars_paths;
    std::vector<std::pair<Vector3, Vector3>> selected_paths;
    ShipComponentRepository _ship_components;
    Camera _camera;
    Path _path;
    std::shared_ptr<Core> _core;
    Vector3 _offset{0., 0., 0.};
    entt::entity _selected_entity{entt::null};

    void _initialize();
    Camera _initialize_camera(const Vector3 &cameraInitialPosition, float cameraDistance,
                              float horizontalDistance, float horizontalAngle,
                              float verticalAngle);

    void _render_visible();
    void _tick();
    void _explode_stars(const ExplosionEvent &);
    void _send_fleet_to_nova(const NovaSeekEvent &);
    void _on_star_selected(const entt::entity);

    void _recalculate_graph();
    void _clear_paths();

    void _render_stars();
    void _render_paths();
    void _render_fleets();

    void _draw_ui();
    void _draw_ui_tab_main(const ImGuiViewport *pViewport);
    void _draw_ui_tab_debug();
    void _draw_ui_main_path_selection();
    void _draw_ui_main_entity_selection(const ImGuiViewport *pViewport);
};

#endif//TLEILAX_GALAXY_H
