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
#include <memory>
#include <path.h>
#include <queue>
#include <graphics_base.h>

#include <utility>
#include <variant>


constexpr auto seed_function = [](const uint32_t x, const uint32_t y, const uint32_t z) {
    return ((x + y) >> 1) * (x + y + 1) + y * ((x + z) >> 1) * (x + z + 1) + z;
};

struct DistanceFunction;
constexpr auto local_to_global_coords = [](const auto coordinates, const auto visible_size) -> Vector3 {
    return {coordinates.x - static_cast<float>(visible_size.x / 2), coordinates.y - static_cast<float>(visible_size.y / 2), coordinates.z - static_cast<float>(visible_size.z / 2)};
};

static void add_vicinity(std::shared_ptr<Core> &core, entt::entity what, entt::entity where) {
    auto *vicinity = core->registry.try_get<components::Vicinity>(what);
    if (vicinity) {
        if (std::find(vicinity->objects.begin(), vicinity->objects.end(), where) != where) {
            vicinity->objects.emplace_back(where);
        }
    } else {
        std::vector<entt::entity> vicinity{where};
        core->registry.emplace<components::Vicinity>(what, vicinity);
    }
}

static void focus_camera(Camera &camera, Vector3 target, float fov = 45.0f) {
    camera.target = target;
    camera.fovy = fov;
    UpdateCamera(&camera);
}

struct Chance {
    uint32_t upper_bound;
    uint32_t occurs_if_less_then;
};

struct CameraSettings {
    float horizontalDistance = 10.0f;
    float cameraDistance = 110.0f;
    Vector3 target = {0.0f, 0.0f, 0.0f};
    Vector2 angle = {45.0f, 45.0f};
    bool focus_on_clicked = false;
    float camera_focused_angle = 30.0f;
    float camera_unfocused_angle = 45.0f;
    float zoom_angle = camera_focused_angle;
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
    static void render(const std::shared_ptr<Core> &, const Camera &, const Vector3 &, entt::entity entity, const Vector3 &coords, const components::Star color, const components::Size size, bool is_selected);
    static void on_click(const entt::registry &, entt::entity);

private:
    entt::entity _entity;
    uint32_t _occurence_chance;
    Chance _exploding_chance;
    Chance _nova_seeker_chance;
};

struct StarInstance {
    Vector3 position;
    Color color;
};

class Galaxy : public UIView {
public:
    explicit Galaxy(std::shared_ptr<Core> core, Assets &assets)
        : _core {std::move(core)},
          _camera(_initialize_camera({0., 0., 0.}, 101., 10., 90., 90.)),
          _ship_components{assets.get_ship_components()} {_initialize();};
    ~Galaxy() = default;

    void render() override;
    void update() override;
    void populate();
    uint32_t next_random_number(const uint32_t max) { return max > 0 ? _core->pcg(max) : 0; };
private:
    RenderInstance _star_render_instance;
    static constexpr uint32_t _star_occurence_chance = 2000;
    const Vector3 _visible_size{75, 50, 75};
    Graph<GraphNode, float, GraphNodeHash, GraphNodeEqualFunc> stars_graph;
    std::vector<std::pair<Vector3, Vector3>> stars_paths;
    std::vector<std::pair<Vector3, Vector3>> selected_paths;
    std::vector<entt::entity> _entities_under_cursor;
    ShipComponentRepository _ship_components;
    Camera _camera;
    CameraSettings _camera_settings;
    Path _path;
    std::shared_ptr<Core> _core;
    Vector3 _offset{0., 0., 0.};
    entt::entity _selected_fleet{entt::null};
    entt::entity _selected_star{entt::null};
    bool _ui_wants_to_set_course = false;
    bool _ui_show_log = false;
    bool _rotate = false;
    float _distance_between_stars = 20.0f;
    bool  _open_demo = false;
    std::vector<entt::entity> _get_nearest_stars(const entt::entity of_entity);
    void _initialize();
    static Camera _initialize_camera(const Vector3 &cameraInitialPosition, float cameraDistance,
                              float horizontalDistance, float horizontalAngle,
                              float verticalAngle);

    CameraSettings _cameraSettings;
    void _init_star_render_instance();
    void _place_star_instance_at(float x, float y, float z, Color c, const Vector3 size);
    void _render_visible();
    void _tick();
    void _explode_stars(const ExplosionEvent &);
    void _send_fleet_to_nova(const NovaSeekEvent &);
    void _fleet_arrived_at_star(const ArrivalEvent &);
    void _entity_left_vicinity(const LeaveEvent &);
    void _on_star_selected(const StarSelectedEvent &);
    void _update_vicinities();

    void _set_course_for_fleet(const entt::entity from, const entt::entity to);
    void _generate_player_entity();
    void _recalculate_graph();
    void _clear_paths();
    void _generate_stars();
    void _generate_fleets();

    void _render_stars();
    void _render_paths();
    void _render_fleets();
    void _render_mouse_selection();

    void _draw_ui();
    void _draw_ui_tab_main();
    void _draw_ui_tab_debug();
    void _draw_ui_tab_camera();
    void _draw_ui_main_entity_selection();
    void _draw_ui_fleet_window();
    void _draw_ui_debug_log_window();
    void _draw_ui_game_log_window();
    void _register_path_selection(const std::vector<entt::entity> &calculated_path);
};

#endif//TLEILAX_GALAXY_H
