//
// Created by geoco on 19.12.2021.
//

#include "galaxy.h"
#include <imgui/imgui.h>
#include <name_generator.h>

template<>
std::string NameGenerator::get_random_name<components::Star>(pcg32 &pcg) {
    auto select_random_name = [&] () {
        NameType category = static_cast<NameType>(pcg(_names.size()));

        return _names[category][pcg(_names[category].size())];//.//[pcg(_names[static_cast<std::underlying_type_t<NameType>>(category)].size())];
    };
    auto randomize_name = [&] () {
        auto name = select_random_name();
        bool has_suffix = pcg(100) <= 50;
        if (has_suffix) {
            bool is_suffix_a_name = pcg(100) <= 50;
            if (is_suffix_a_name) {
                return name + " " + select_random_name();
            } else {
                return name + " " + star_suffixes[pcg(star_suffixes.size())];
            }
        }
        return name;
    };
    return randomize_name();
}

constexpr auto seed_function = [](const uint32_t x, const uint32_t y, const uint32_t z) {
    return ((x + y) >> 1) * (x + y + 1) + y * ((x + z) >> 1) * (x + z + 1) + z;
};

struct DistanceFunction;
constexpr auto local_to_global_coords = [](const auto coordinates, const auto visible_size) -> Vector3 {
    return {coordinates.x - static_cast<float>(visible_size.x / 2), coordinates.y - static_cast<float>(visible_size.y / 2), coordinates.z - static_cast<float>(visible_size.z / 2)};
};

static float distance_between_stars = 20.0f;

static bool open_demo = false;

void Galaxy::_recalculate_graph() {
    _clear_paths();
    stars_paths.clear();
    stars_graph.get().clear();
    _core->registry.view<Vector3, components::Star, components::Size>().each(
            [&](const entt::entity entity, const Vector3 &coords, const components::Star color, const components::Size size) {
                GraphNode starNode{entity, true};
                _core->registry.view<Vector3, components::Star, components::Size>().each(
                        [&](const entt::entity _entity, const Vector3 &_coords, const components::Star _color, const components::Size _size) {
                            if (entity != _entity) {
                                GraphNode next{_entity, false};
                                const auto distance = Vector3Distance(coords, _coords);
                                if (distance < distance_between_stars) {
                                    stars_graph.add_edge(starNode, next, distance, false);
                                    stars_paths.emplace_back(std::make_pair(coords, _coords));
                                }
                            }
                        });
            });
}

void Galaxy::_initialize() {
    _path = Path{};
    _core->dispatcher.sink<ExplosionEvent>().connect<&Galaxy::_explode_stars>(this);
    _core->dispatcher.sink<NovaSeekEvent>().connect<&Galaxy::_send_fleet_to_nova>(this);
}

void Galaxy::populate() {
    auto before = std::chrono::high_resolution_clock::now();
    _core->registry.clear();
    selected_paths.clear();
    _selected_entity = entt::null;

    for (int32_t z = 0; z < _visible_size.z; z++) {
        for (int32_t y = 0; y < _visible_size.y; y++) {
            for (int32_t x = 0; x < _visible_size.x; x++) {
                StarEntity star(_star_occurence_chance, {100, 5}, {100, 10});
                next_random_number(seed_function(x, y, z));
                star.create_at(_core->registry, _core, Vector3{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});
            }
        }
    }

    _generate_player_entity();
    _recalculate_graph();

    auto after = std::chrono::high_resolution_clock::now() - before;
    std::printf("Elapsed time: %lld ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(after).count());
}

void Galaxy::render() {
    BeginDrawing();
    ClearBackground(BLACK);
    _render_visible();
    DrawFPS(1240, 10);
    _draw_ui();
    EndDrawing();
}

void Galaxy::_render_visible() {

    BeginMode3D(_camera);
    DrawCubeWires({0., 0., 0.}, _visible_size.x, _visible_size.y, _visible_size.z, YELLOW);

    _render_stars();
    _render_paths();
    _render_fleets();

    EndMode3D();
}

void Galaxy::_render_fleets() {
    _core->registry.view<components::Fleet, Vector3, components::Size>().each([&](const entt::entity entity, const components::Fleet &fleet, const Vector3 pos, const components::Size size) {
        Vector3 fleet_coords = local_to_global_coords(pos, _visible_size);
        if (GetRayCollisionSphere(GetMouseRay(GetMousePosition(), _camera), fleet_coords, size.size).hit) {
            DrawSphereWires(fleet_coords, size.size, 6, 6, RED);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                FleetEntity::on_click(_core->registry, entity);
                _selected_entity = entity;
                auto *fleet_path = _core->registry.try_get<components::Path>(entity);
                if (fleet_path && !fleet_path->checkpoints.empty()) {
                    _register_path_selection(fleet_path->checkpoints);
                }
            }
        } else {
            DrawSphereWires(fleet_coords, size.size, 6, 6, SKYBLUE);
        }
    });
}

void Galaxy::_render_paths() {
    if (selected_paths.empty()) {
        std::for_each(stars_paths.begin(), stars_paths.end(), [&](const std::pair<Vector3, Vector3> &neighbours) {
            DrawLine3D(local_to_global_coords(neighbours.first, _visible_size), local_to_global_coords(neighbours.second, _visible_size), YELLOW);
        });
    } else {
        std::for_each(selected_paths.begin(), selected_paths.end(), [&](const std::pair<Vector3, Vector3> &neighbours) {
            DrawLine3D(local_to_global_coords(neighbours.first, _visible_size), local_to_global_coords(neighbours.second, _visible_size), BLUE);
        });
    }
}

void Galaxy::_render_stars() {
    _core->registry.view<Vector3, components::Star, components::Size>().each([&](const entt::entity entity, const Vector3 &coords, const components::Star color, const components::Size size) {
        Vector3 star_coords = local_to_global_coords(coords, _visible_size);
        bool star_is_selected = GetRayCollisionSphere(GetMouseRay(GetMousePosition(), _camera), star_coords, size.size).hit;
        StarEntity::render(_core->registry, _visible_size, entity, coords, color, size, star_is_selected);
        if (star_is_selected && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            _on_star_selected(entity);
        }
        EndMode3D();

        auto *name = _core->registry.try_get<components::Name>(entity);
        if (name) {
            auto name_pos = GetWorldToScreenEx(star_coords, _camera, 1280, 720);
            DrawRectangle(name_pos.x - 22, name_pos.y - 22, name->name.length() * 8, 12, BLACK);
            DrawText(name->name.c_str(), name_pos.x - 20, name_pos.y - 20, 10, WHITE);
        }

        BeginMode3D(_camera);
    });
}

void Galaxy::_draw_ui() {
    rlImGuiBegin();
    if (open_demo) {
        ImGui::ShowDemoWindow(&open_demo);
    } else {
        bool open = true;
        static bool use_work_area = true;
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
        const ImGuiViewport *viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
        ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);
        if (ImGui::Begin("Tleilax control", &open, flags)) {
            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if (ImGui::IsMousePosValid())
                if (ImGui::BeginTabBar("Controls", tab_bar_flags)) {
                    _draw_ui_tab_main(viewport);
                    _draw_ui_tab_debug();
                    ImGui::EndTabBar();
                }
        }
        ImGui::End();
    }

    rlImGuiEnd();
}
void Galaxy::_draw_ui_tab_main(const ImGuiViewport *pViewport) {
    if (ImGui::BeginTabItem("Main")) {
        ImGui::PushButtonRepeat(true);
        if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
            UpdateCamera(&_camera);
        }
        ImGui::PopButtonRepeat();
        ImGui::PushButtonRepeat(true);
        ImGui::SameLine();
        if (ImGui::Button("Update")) {
            _tick();
        }
        ImGui::PopButtonRepeat();
        _draw_ui_main_path_selection();
        _draw_ui_main_entity_selection(pViewport);
        ImGui::EndTabItem();
    }
}
void Galaxy::_draw_ui_main_path_selection() {
    if (_path.from == entt::null) {
        ImGui::Text("No paths selected - click on two stars to establish a path between them");
    } else {
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
}
void Galaxy::_draw_ui_main_entity_selection(const ImGuiViewport *pViewport) {
    if (_selected_entity != entt::null) {
        auto *fleet = _core->registry.try_get<components::Fleet>(_selected_entity);
        if (fleet) {
            auto *path = _core->registry.try_get<components::Path>(_selected_entity);
            auto position = _core->registry.get<Vector3>(_selected_entity);
            static int corner = 1;
            ImGuiIO &io = ImGui::GetIO();
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
            ImVec2 work_pos = pViewport->WorkPos;// Use work area to avoid menu-bar/task-bar, if any!
            ImVec2 work_size = pViewport->WorkSize;
            ImVec2 window_pos, window_pos_pivot;
            if (corner != -1) {
                const float PAD = 30.0f;

                window_pos.x = (corner & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
                window_pos.y = (corner & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
                window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
                window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
                ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
                window_flags |= ImGuiWindowFlags_NoMove;
            }
            ImGui::SetNextWindowBgAlpha(0.85f);// Transparent background
            if (ImGui::BeginChild("Selection", ImVec2(350, 600), true)) {
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                ImGui::Text("Selected fleet");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 1.0f, 1.0f), "%d", _selected_entity);
                ImGui::Text("Currently at");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%.0f, %.0f, %0.f", position.x, position.y, position.z);
                if (path && !path->checkpoints.empty()) {
                    auto &destination_position = _core->registry.get<Vector3>(path->checkpoints.back());
                    auto &destination_name = _core->registry.get<components::Name>(path->checkpoints.back());
                    auto &next_stop_position = _core->registry.get<Vector3>(path->checkpoints.front());
                    auto &next_stop_name = _core->registry.get<components::Name>(path->checkpoints.front());
                    ImGui::Text("Heading towards");
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "%s", destination_name.name.c_str());
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%.0f, %.0f, %0.f", destination_position.x, destination_position.y, destination_position.z);
                    ImGui::Text(" -> next stop");
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "%s", next_stop_name.name.c_str());
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%.0f, %.0f, %0.f", next_stop_position.x, next_stop_position.y, next_stop_position.z);
                }
                ImGui::Separator();
                for (int i = 0; i < fleet->ships.size(); i++) {
                    const auto ship = fleet->ships[i];
                    const auto engine = _core->registry.get<components::Engine>(ship);
                    const auto hull = _core->registry.get<components::Hull>(ship);
                    const auto shield = _core->registry.get<components::Shield>(ship);
                    const auto weapon = _core->registry.get<components::Weapon>(ship);

                    if (ImGui::TreeNode((void *) (intptr_t) i, "Ship %d", ship)) {
                        ImGui::Text("%s", engine.name.c_str());
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%.d/%.d", engine.power, engine.weight);
                        ImGui::Text("%s", hull.name.c_str());
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%.1f/%.1f", hull.health, hull.max_health);
                        ImGui::Text("%s", shield.name.c_str());
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%.1d", shield.defense);
                        ImGui::Text("%s", weapon.name.c_str());
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%dd%d", weapon.damage.amount, weapon.damage.sides);
                        ImGui::TreePop();
                        ImGui::Separator();
                    }
                }
                ImGui::Separator();
                if (ImGui::Button("Exit")) {
                    _selected_entity = entt::null;
                }
                ImGui::EndChild();
            }
        }
    }
}
void Galaxy::_draw_ui_tab_debug() {
    if (ImGui::BeginTabItem("Debug")) {
        float dist_from = 1.0f;
        float dist_to = 50.0f;
        ImGui::DragScalar("Scroll distance between stars", ImGuiDataType_Float, &distance_between_stars, 0.5f, &dist_from, &dist_to, "%f");
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            _recalculate_graph();
        }
        if (ImGui::Button("Generate new world")) {
            populate();
        }
        ImGui::EndTabItem();
    }
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

    auto fleets = _core->registry.view<components::Fleet, Vector3, components::Path>();

    fleets.each([this](const entt::entity entity, components::Fleet &fleet, Vector3 &position, components::Path &path) {
        FleetEntity::update(_core->registry, entity, fleet, position, path);
    });

    _core->dispatcher.update();
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

void Galaxy::_send_fleet_to_nova(const NovaSeekEvent &ev) {
    FleetEntity fleet;
    fleet.react_to_nova(_core->registry, _core->pcg, ev, _ship_components, stars_graph);
}

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
            std::vector<entt::entity> calculated_path = calculate_path<Vector3, components::Star, DistanceFunction>(stars_graph, _core->registry, _path.from, _path.to);
            if (!calculated_path.empty()) {
                _register_path_selection(calculated_path);
            }
        }
    }
}
void Galaxy::_register_path_selection(const std::vector<entt::entity> &calculated_path) {
    selected_paths.clear();
    for (size_t i = 0; i < calculated_path.size() - 1; i++) {
        Vector3 first = _core->registry.get<Vector3>(calculated_path[i]);
        Vector3 second = _core->registry.get<Vector3>(calculated_path[i + 1]);
        selected_paths.emplace_back(std::make_pair(first, second));
    }
}

void Galaxy::_clear_paths() {
    _path.from = entt::null;
    _path.to = entt::null;
    _path.checkpoints.clear();
    selected_paths.clear();
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

void Galaxy::_generate_player_entity() {
    std::vector<entt::entity> all_stars;
    _core->registry.view<components::Star>().each([&](auto entity, auto star){
        all_stars.emplace_back(entity);
    });
    entt::entity random_star = all_stars[_core->pcg(all_stars.size())];

    entt::entity player_fleet = FleetEntity::create(_core->registry, _core->pcg, _core->registry.get<Vector3>(random_star), _ship_components);
    _core->registry.emplace<components::PlayerControlled>(player_fleet);
}

entt::entity StarEntity::create_at(entt::registry &registry, const std::shared_ptr<Core> &core, Vector3 position) {
    auto &pcg = core->pcg;
    if (pcg(_occurence_chance) == 0) {
        _entity = registry.create();
        registry.emplace<Vector3>(_entity, position);
        registry.emplace<components::Name>(_entity, components::Name{core->name_generator.get_random_name<components::Star>(pcg)});
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
