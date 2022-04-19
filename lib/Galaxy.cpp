//
// Created by geoco on 19.12.2021.
//

#include "galaxy.h"
#include <imgui/imgui.h>

constexpr auto seed_function = [](const uint32_t x, const uint32_t y, const uint32_t z) {
    return ((x + y) >> 1) * (x + y + 1) + y * ((x + z) >> 1) * (x + z + 1) + z;
};

constexpr auto local_to_global_coords = [](const auto coordinates, const auto visible_size) -> Vector3 {
    return {coordinates.x - static_cast<float>(visible_size.x / 2), coordinates.y - static_cast<float>(visible_size.y / 2), coordinates.z - static_cast<float>(visible_size.z / 2)};
};
static float _distance_between_stars = 20.0f;

void Galaxy::_recalculate_graph() {
    _clear_paths();
    paths.clear();
    starGraph.get().clear();
    _core->registry.view<Vector3, components::Star, components::Size>().each(
            [&](const entt::entity entity, const Vector3 &coords, const components::Star color, const components::Size size) {
                GraphNode starNode{entity, true};
                _core->registry.view<Vector3, components::Star, components::Size>().each(
                        [&](const entt::entity _entity, const Vector3 &_coords, const components::Star _color, const components::Size _size) {
                            if (entity != _entity) {
                                GraphNode next{_entity, false};
                                const auto distance = Vector3Distance(coords, _coords);
                                if (distance < _distance_between_stars) {
                                    starGraph.add_edge(starNode, next, distance, false);
                                    paths.emplace_back(std::make_pair(coords, _coords));
                                }
                            }
                        });
            });
}
void Galaxy::populate() {
    auto before = std::chrono::high_resolution_clock::now();
    _core->registry.clear();
    selected_paths.clear();

    for (int32_t z = 0; z < _visible_size.z; z++) {
        for (int32_t y = 0; y < _visible_size.y; y++) {
            for (int32_t x = 0; x < _visible_size.x; x++) {
                StarEntity star(7000, {100, 5}, {100, 10});
                next_random_number(seed_function(x, y, z));
                star.create_at(_core->registry, _core->pcg, Vector3{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});
            }
        }
    }

    _recalculate_graph();
    auto after = std::chrono::high_resolution_clock::now() - before;
    std::printf("Elapsed time: %lld ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(after).count());
}

void Galaxy::_render_visible() {

    BeginMode3D(_camera);
    DrawCubeWires({0., 0., 0.}, _visible_size.x, _visible_size.y, _visible_size.z, YELLOW);

    _core->registry.view<Vector3, components::Star, components::Size>().each([&](const entt::entity entity, const Vector3 &coords, const components::Star color, const components::Size size) {
        Vector3 star_coords = local_to_global_coords(coords, _visible_size);
        bool star_is_selected = GetRayCollisionSphere(GetMouseRay(GetMousePosition(), _camera), star_coords, size.size).hit;
        StarEntity::render(_core->registry, _visible_size, entity, coords, color, size, star_is_selected);
        if (star_is_selected && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            _on_star_selected(entity);
        }
    });

    if (selected_paths.empty()) {
        std::for_each(paths.begin(), paths.end(), [&](const std::pair<Vector3, Vector3> &neighbours) {
            DrawLine3D(local_to_global_coords(neighbours.first, _visible_size), local_to_global_coords(neighbours.second, _visible_size), YELLOW);
        });
    } else {
        std::for_each(selected_paths.begin(), selected_paths.end(), [&](const std::pair<Vector3, Vector3> &neighbours) {
            DrawLine3D(local_to_global_coords(neighbours.first, _visible_size), local_to_global_coords(neighbours.second, _visible_size), BLUE);
        });
    }

    _core->registry.view<components::Fleet, Vector3, components::Size>().each([&](const entt::entity entity, const components::Fleet &fleet, const Vector3 pos, const components::Size size) {
        Vector3 fleet_coords = local_to_global_coords(pos, _visible_size);
        if (GetRayCollisionSphere(GetMouseRay(GetMousePosition(), _camera), fleet_coords, size.size).hit) {
            DrawSphereWires(fleet_coords, size.size, 6, 6, RED);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                FleetEntity::on_click(_core->registry, entity);
            }
        } else {
            DrawSphereWires(fleet_coords, size.size, 6, 6, GREEN);
        }
    });
    EndMode3D();
}

Camera Galaxy::_initialize_camera(const Vector3 &cameraInitialPosition, const float cameraDistance,
                                  const float horizontalDistance, const float horizontalAngle,
                                  const float verticalAngle) {
    Camera camera;
    camera.target = cameraInitialPosition;
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    camera.position.x = horizontalDistance * std::cosh(horizontalAngle * PI / 180.0f);
    camera.position.y = horizontalDistance * std::sinh(horizontalAngle * PI / 180.0f);
    camera.position.z = cameraDistance * sinf(verticalAngle * PI / 180.0f);
    SetCameraMode(camera, CAMERA_ORBITAL);
    UpdateCamera(&camera);
    return camera;
}

static bool open_demo = false;
void Galaxy::_draw_ui() {
    rlImGuiBegin();
    if (open_demo) {
        ImGui::ShowDemoWindow(&open_demo);
    } else {
        bool open = true;
        static bool use_work_area = true;
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

        // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
        // Based on your use case you may want one of the other.
        const ImGuiViewport *viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
        ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);
        if (ImGui::Begin("Tleilax control", &open, flags)) {
            if (_path.from == entt::null) {
                ImGui::Text("No paths selected - click on two stars to establish a path between them");
            }
            else {
                Vector3 from = _core->registry.get<Vector3>(_path.from);
                ImGui::Text("Establishing path from (%.0f, %.0f, %.0f) ", from.x, from.y, from.z);
                if (_path.to != entt::null) {
                    ImGui::SameLine();
                    Vector3 to = _core->registry.get<Vector3>(_path.to);
                    ImGui::Text("to (%.0f, %.0f, %.0f) ", to.x, to.y, to.z);
                    ImGui::SameLine();
                    if (ImGui::Button("Clear")) {
                        _clear_paths();
                    }
                }
            }
            ImGui::PushButtonRepeat(true);
            if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
                UpdateCamera(&_camera);
            }
            ImGui::PopButtonRepeat();
            ImGui::SameLine();
            if (ImGui::Button("Update")) {
                _tick();
            }
            float dist_from = 1.0f;
            float dist_to = 50.0f;
            ImGui::DragScalar("Scroll distance between stars", ImGuiDataType_Float,  &_distance_between_stars, 0.5f,  &dist_from, &dist_to, "%f");
            if  (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                _recalculate_graph();
            }
            if (ImGui::Button("Generate new world")) {
                populate();
            }
        }

        ImGui::End();
    }

    rlImGuiEnd();
}
void Galaxy::render() {
    BeginDrawing();
    ClearBackground(BLACK);
    _render_visible();
    DrawFPS(1240, 10);
    _draw_ui();
    EndDrawing();
}

void Galaxy::update() {
    if (IsKeyDown(KEY_A)) {
        UpdateCamera(&_camera);
    } else if (IsKeyPressed(KEY_T)) {
        _tick();
    } else if (IsKeyPressed(KEY_C)) {
        _clear_paths();
    } else if (IsKeyPressed(KEY_R)) {
        populate();
    }
    if (IsKeyPressed(KEY_D)) {
        open_demo = !open_demo;
    }
}

static void change_course_upon_nearer_explosion(const Vector3 &explosion_position, const entt::entity entity, components::Fleet &fleet, Vector3 &position, components::Destination &destination, const components::Size size) {
    auto destination_vector = Vector3{static_cast<float>(destination.dest.x), static_cast<float>(destination.dest.y), static_cast<float>(destination.dest.z)};
    if (Vector3Distance(explosion_position, position) < Vector3Distance(destination_vector, position)) {
        destination = components::Destination{static_cast<int32_t>(explosion_position.x), static_cast<int32_t>(explosion_position.y), static_cast<int32_t>(explosion_position.z)};
    }
}
void Galaxy::_explode_stars(const ExplosionEvent &ev) {
    auto explosion_position = _core->registry.get<Vector3>(ev.e);
    _core->registry.remove<components::Exploding>(ev.e);
    _core->registry.emplace<components::Nova>(ev.e);
    std::printf("Explosion at [%g, %g, %g]\n", explosion_position.x, explosion_position.y, explosion_position.z);
    auto nova_seekers = _core->registry.view<components::NovaSeeker>();
    nova_seekers.each([this, ev](const entt::entity entity, components::NovaSeeker &nova_seeker) {
        if (nova_seeker.capacity > 0) {
            _core->dispatcher.enqueue<NovaSeekEvent>(entity, ev.e);
            nova_seeker.capacity -= 1;
        }
    });

    auto fleets = _core->registry.view<components::Fleet, Vector3, components::Destination, components::Size>();
    fleets.each([=](const entt::entity entity, components::Fleet &fleet, Vector3 &position, components::Destination &destination, const components::Size size) {
        change_course_upon_nearer_explosion(explosion_position, entity, fleet, position, destination, size);
    });
}

void Galaxy::_initialize() {
    _path = Path{};
    _core->dispatcher.sink<ExplosionEvent>().connect<&Galaxy::_explode_stars>(this);
    _core->dispatcher.sink<NovaSeekEvent>().connect<&Galaxy::_send_fleet_to_nova>(this);
}

void Galaxy::_tick() {

    auto view = _core->registry.view<components::Exploding, components::Size, Vector3>();
    view.each([this](const entt::entity entity, components::Exploding &exploding, components::Size &size, Vector3 &position) {
        if (exploding.counter > 0) {
            exploding.counter -= 1;
            size.size -= 1.0f;
        } else {
            _core->dispatcher.enqueue<ExplosionEvent>(entity);
        }
    });

    auto fleets = _core->registry.view<components::Fleet, Vector3, components::Destination, components::Size>();
    fleets.each(FleetEntity::update);

    _core->dispatcher.update();
}

void Galaxy::_send_fleet_to_nova(const NovaSeekEvent &ev) {
    FleetEntity fleet;
    fleet.react_to_nova(_core->registry, _core->pcg, ev, _ship_components, starGraph);
}

struct DistanceFunction;

void Galaxy::_on_star_selected(const entt::entity entity) {
    if (_core->registry.valid(entity)) {
        StarEntity::on_click(_core->registry, entity);
        if (_path.from == entt::null) {
            std::printf("From: %d\n", entity);
            _path.from = entity;
        } else {
            std::printf("To: %d\n", entity);
            _path.to = entity;
        }
        if (_path.from != entt::null && _path.to != entt::null) {
            std::vector<entt::entity> calculated_path = calculate_path<Vector3, components::Star, DistanceFunction>(starGraph, _core->registry, _path.from, _path.to);
            if (!calculated_path.empty()) {
                selected_paths.clear();
                for (size_t i = 0; i < calculated_path.size() - 1; i++) {
                    Vector3 first = _core->registry.get<Vector3>(calculated_path[i]);
                    Vector3 second = _core->registry.get<Vector3>(calculated_path[i + 1]);
                    selected_paths.emplace_back(std::make_pair(first, second));
                }
            }
        }
    }
}
void Galaxy::_clear_paths() {
    _path.from = entt::null;
    _path.to = entt::null;
    _path.checkpoints.clear();
    selected_paths.clear();
}

entt::entity StarEntity::create_at(entt::registry &registry, pcg32 &pcg, Vector3 position) {
    if (pcg(_occurence_chance) == 0) {
        _entity = registry.create();
        registry.emplace<Vector3>(_entity, position);

        if (pcg(_exploding_chance.upper_bound) < _exploding_chance.occurs_if_less_then) {
            const auto explosion_counter = pcg(15) + 1;
            registry.emplace<components::Star>(_entity, components::Star{255, 255, 255, 255});
            registry.emplace<components::Exploding>(_entity, static_cast<uint8_t>(explosion_counter));
            registry.emplace<components::Size>(_entity, static_cast<float>(explosion_counter));
        } else {
            registry.emplace<components::Star>(_entity, components::Star{static_cast<uint8_t>(pcg(255)), static_cast<uint8_t>(pcg(255)), static_cast<uint8_t>(pcg(255)), 255});
            registry.emplace<components::Size>(_entity, 1.0f);

            if (pcg(_nova_seeker_chance.upper_bound) < _nova_seeker_chance.occurs_if_less_then) {
                registry.emplace<components::NovaSeeker>(_entity, pcg(5) + 1);
            }
        }
    }

    return _entity;
}

bool StarEntity::is_created() {
    return _entity != entt::null;
}
void StarEntity::render(const entt::registry &registry, const Vector3 &visible_size, const entt::entity entity, const Vector3 &coords, const components::Star color, const components::Size size, const bool is_selected) {
    Vector3 star_coords = local_to_global_coords(coords, visible_size);
    DrawSphere(star_coords, size.size, {color.r, color.g, color.b, color.a});
    if (is_selected) {
        DrawSphereWires(star_coords, size.size + 1, 6, 6, GREEN);
    }
    if (registry.any_of<components::Nova>(entity)) {
        DrawSphereWires(star_coords, 5, 6, 6, VIOLET);
    }
}

void StarEntity::on_click(const entt::registry &registry, const entt::entity entity) {
    auto position = registry.get<Vector3>(entity);
    std::printf("Clicked star=[%.1f, %.1f, %.1f]\n", position.x, position.y, position.z);
}
