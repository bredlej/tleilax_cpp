//
// Created by geoco on 02.05.2022.
//
#include <events.hpp>
#include <galaxy.hpp>

void Galaxy::_draw_ui() {
    rlImGuiBegin();

    if (_open_demo) {
        ImGui::ShowDemoWindow(&_open_demo);
    } else {
        bool open = true;
        static ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;

        if (ImGui::Begin("Tleilax control", &open, flags)) {
            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if (ImGui::IsMousePosValid())
                if (ImGui::BeginTabBar("Controls", tab_bar_flags)) {
                    _draw_ui_tab_main();
                    _draw_ui_tab_debug();
                    ImGui::EndTabBar();
                }
        }
        ImGui::End();
        _draw_ui_main_entity_selection();
        _draw_ui_debug_log_window();
        _draw_ui_game_log_window();
    }

    rlImGuiEnd();
}
void Galaxy::_draw_ui_tab_main() {
    if (ImGui::BeginTabItem("Main")) {
        ImGui::PushButtonRepeat(true);
        if (ImGui::Button("Update")) {
            _tick();
        }
        ImGui::PopButtonRepeat();
        _draw_ui_tab_camera();

        ImGui::EndTabItem();
    }
}

void Galaxy::_draw_ui_fleet_window() {
    auto *fleet = _core->registry.try_get<components::Fleet>(_selected_fleet);
    if (fleet) {
        bool open = true;
        auto *path = _core->registry.try_get<components::Path>(_selected_fleet);
        components::PlayerControlled *player_controlled = _core->registry.try_get<components::PlayerControlled>(_selected_fleet);
        auto position = _core->registry.get<Vector3>(_selected_fleet);
        static int corner = 1;
        ImGuiIO &io = ImGui::GetIO();
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize;
        ImVec2 work_pos = ImGui::GetMainViewport()->WorkPos;// Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = ImGui::GetMainViewport()->WorkSize;
        ImVec2 window_pos, window_pos_pivot;
        if (corner != -1) {
            const float PAD = 30.0f;

            window_pos.x = (corner & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
            window_pos.y = (corner & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
            window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
            window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Once, window_pos_pivot);
        }
        ImGui::SetNextWindowBgAlpha(0.85f);// Transparent background
        if (ImGui::Begin("Fleet control", &open, window_flags)) {
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (player_controlled) {
                ImGui::TextColored(ImVec4(static_cast<float>(Colors::col_16.r) / 255.0f, static_cast<float>(Colors::col_16.g) / 255.0f, static_cast<float>(Colors::col_16.b) / 255.0f, 1), "Player fleet");
            } else {
                ImGui::Text("Selected fleet");
            }
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(static_cast<float>(Colors::col_5.r) / 255.0f, static_cast<float>(Colors::col_5.g) / 255.0f, static_cast<float>(Colors::col_5.b) / 255.0f, 1), "%d", _selected_fleet);

            auto vicinity = _core->registry.try_get<components::Vicinity>(_selected_fleet);

            if (vicinity && !vicinity->objects.empty()) {
                ImGui::Text("Vicinity %lu objects: ", vicinity->objects.size());
                ImGui::Text("Is near of");
                int i = 0;
                std::for_each(vicinity->objects.begin(), vicinity->objects.end(), [&](entt::entity object) {
                   if (components::Fleet *fleet = _core->registry.try_get<components::Fleet>(object)) {
                        ImGui::Text("Fleet: ");
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(static_cast<float>(Colors::col_5.r) / 255.0f, static_cast<float>(Colors::col_5.g) / 255.0f, static_cast<float>(Colors::col_5.b) / 255.0f, 1), "%d (%lu ships)", object, fleet->ships.size());
                        if (ImGui::IsItemClicked()) {
                            _selected_fleet = object;
                        }
                        if (player_controlled) {
                            ImGui::SameLine();
                            char button_label[12];
                            ImGui::PushID(i++);
                            if (ImGui::Button("Attack")) {
                                _core->dispatcher.enqueue<PlayerBattleStartEvent>(_selected_fleet, object);
                            }
                            ImGui::PopID();
                        }
                    }
                    components::Name *object_name = _core->registry.try_get<components::Name>(object);
                    if (object_name) {
                        const auto star = _core->registry.try_get<components::Star>(object);
                        ImGui::TextColored(ImVec4(static_cast<float>(star->r) / 255.0f, static_cast<float>(star->g) / 255.0f, static_cast<float>(star->b) / 255.0f, 1.0f), "%s", object_name->name.c_str());
                        if (star) {
                            if (player_controlled) {
                                const auto star_position = _core->registry.get<Vector3>(object);
                                const auto seed = seed_function(static_cast<uint32_t>(star_position.x), static_cast<uint32_t>(star_position.y), static_cast<uint32_t>(star_position.z));
                                const auto known_systems = _core->registry.get<components::KnownStarSystems>(_selected_fleet);
                                if (std::find(known_systems.seeds.begin(), known_systems.seeds.end(), seed) == known_systems.seeds.end()) {
                                    ImGui::SameLine();
                                    if (ImGui::Button("Scan")) {
                                        _core->game_log.message("Scanning %s [%d]\n", object_name->name.c_str(), seed);
                                        _core->dispatcher.enqueue<StarScanEvent>(_selected_fleet, object);
                                    }
                                }
                            }
                        } else {
                            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "%s", object_name->name.c_str());
                        }
                    }
                });
            }
            ImGui::Text("Currently at");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(static_cast<float>(Colors::col_5.r) / 255.0f, static_cast<float>(Colors::col_5.g) / 255.0f, static_cast<float>(Colors::col_5.b) / 255.0f, 1), "%.0f, %.0f, %0.f", position.x, position.y, position.z);
            if (path && !path->checkpoints.empty()) {
                const auto destination = path->checkpoints.back();
                auto &destination_position = _core->registry.get<Vector3>(destination);
                auto &destination_name = _core->registry.get<components::Name>(destination);
                const auto *destination_color = _core->registry.try_get<components::Star>(destination);

                const auto next_stop = path->checkpoints.front();
                auto &next_stop_position = _core->registry.get<Vector3>(next_stop);
                auto &next_stop_name = _core->registry.get<components::Name>(next_stop);
                const auto *next_stop_color = _core->registry.try_get<components::Star>(next_stop);
                ImGui::Text("Heading towards");
                ImGui::SameLine();
                if (destination_color) {
                    ImGui::TextColored(ImVec4(static_cast<float>(destination_color->r) / 255.0f, static_cast<float>(destination_color->g) / 255.0f, static_cast<float>(destination_color->b) / 255.0f, 1.0f), "%s", destination_name.name.c_str());
                } else {
                    ImGui::TextColored(ImVec4(static_cast<float>(Colors::col_16.r) / 255.0f, static_cast<float>(Colors::col_16.g) / 255.0f, static_cast<float>(Colors::col_16.b) / 255.0f, 1), "%s", destination_name.name.c_str());
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%.0f, %.0f, %0.f", destination_position.x, destination_position.y, destination_position.z);
                ImGui::Text(" -> next stop");
                ImGui::SameLine();
                if (destination_color) {
                    ImGui::TextColored(ImVec4(static_cast<float>(next_stop_color->r) / 255.0f, static_cast<float>(next_stop_color->g) / 255.0f, static_cast<float>(next_stop_color->b) / 255.0f, 1.0f), "%s", next_stop_name.name.c_str());
                } else {
                    ImGui::TextColored(ImVec4(static_cast<float>(Colors::col_16.r) / 255.0f, static_cast<float>(Colors::col_16.g) / 255.0f, static_cast<float>(Colors::col_16.b) / 255.0f, 1), "%s", next_stop_name.name.c_str());
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%.0f, %.0f, %0.f", next_stop_position.x, next_stop_position.y, next_stop_position.z);
            }
            if (!_ui_wants_to_set_course && ImGui::Button("Travel to")) {
                _ui_wants_to_set_course = true;
            } else if (_ui_wants_to_set_course && ImGui::Button("Cancel")) {
                _ui_wants_to_set_course = false;
            }
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
                ImGui::TextColored(ImVec4(static_cast<float>(Colors::col_5.r) / 255.0f, static_cast<float>(Colors::col_5.g) / 255.0f, static_cast<float>(Colors::col_5.b) / 255.0f, 1), "%.d/%.d", engine.power, engine.weight);
                ImGui::Text("%s", hull.name.c_str());
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(static_cast<float>(Colors::col_5.r) / 255.0f, static_cast<float>(Colors::col_5.g) / 255.0f, static_cast<float>(Colors::col_5.b) / 255.0f, 1), "%.1f/%.1f", hull.health, hull.max_health);
                ImGui::Text("%s", shield.name.c_str());
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(static_cast<float>(Colors::col_5.r) / 255.0f, static_cast<float>(Colors::col_5.g) / 255.0f, static_cast<float>(Colors::col_5.b) / 255.0f, 1), "%.1d", shield.defense);
                ImGui::Text("%s", weapon.name.c_str());
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(static_cast<float>(Colors::col_5.r) / 255.0f, static_cast<float>(Colors::col_5.g) / 255.0f, static_cast<float>(Colors::col_5.b) / 255.0f, 1), "%dd%d", weapon.damage.amount, weapon.damage.sides);
                ImGui::TreePop();
                ImGui::Separator();
            }
        }
        ImGui::Separator();
        if (ImGui::Button("Exit")) {
            _selected_fleet = entt::null;
            _ui_wants_to_set_course = false;
        }
        ImGui::End();
    }
}

void Galaxy::_draw_ui_star_window() {
    if (_core->registry.valid(_clicked_star)) {
        auto *star = _core->registry.try_get<components::Star>(_clicked_star);
        if (star) {
            bool open = true;
            static int corner = 1;
            ImGuiIO &io = ImGui::GetIO();
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize;
            ImVec2 work_pos = ImGui::GetMainViewport()->WorkPos;// Use work area to avoid menu-bar/task-bar, if any!
            ImVec2 work_size = ImGui::GetMainViewport()->WorkSize;
            ImVec2 window_pos, window_pos_pivot;
            if (corner != -1) {
                const float PAD = 30.0f;

                window_pos.x = (corner & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
                window_pos.y = (corner & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
                window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
                window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
                ImGui::SetNextWindowPos(window_pos, ImGuiCond_Once, window_pos_pivot);
            }
            ImGui::SetNextWindowBgAlpha(0.85f);// Transparent background

            auto star_name = _core->registry.get<components::Name>(_clicked_star);
            Vector3 star_position = _core->registry.get<Vector3>(_clicked_star);
            const uint32_t star_seed = seed_function(static_cast<uint32_t>(star_position.x), static_cast<uint32_t>(star_position.y), static_cast<uint32_t>(star_position.z));
            if (ImGui::Begin("Star details", &open, window_flags)) {
                ImGui::TextColored(ImVec4(static_cast<float>(star->r) / 255.0f, static_cast<float>(star->g) / 255.0f, static_cast<float>(star->b) / 255.0f, 1.0f), "%s", star_name.name.c_str());
                bool player_knows_star = false;
                _core->registry.view<components::PlayerControlled, components::KnownStarSystems>().each([&player_knows_star, star_seed](const auto entity, const auto player, const components::KnownStarSystems &known_stars) {
                    if (std::find(known_stars.seeds.begin(), known_stars.seeds.end(), star_seed) != known_stars.seeds.end()) {
                        player_knows_star = true;
                    }
                });
                if (player_knows_star) {
                    ImGui::Text("Star is known");
                    const auto &star_registry = _star_systems[star_seed].get()->get_registry();
                    ImGui::Text("%lu bodies here", star_registry.size());
                }
                ImGui::End();
            }
        }
    }
}
void Galaxy::_draw_ui_main_entity_selection() {
    if (_selected_fleet != entt::null) {
        _draw_ui_fleet_window();
    }
    if (_clicked_star != entt::null) {
        _draw_ui_star_window();
    }
}
void Galaxy::_draw_ui_tab_debug() {
    if (ImGui::BeginTabItem("Debug")) {
        auto work_pos = ImGui::GetMainViewport()->WorkPos;
        ImGui::Text("Work pos: %.1f, %.1f", work_pos.x, work_pos.y);
        auto work_size = ImGui::GetMainViewport()->WorkSize;
        ImGui::Text("Work size: %.1f, %.1f", work_size.x, work_size.y);
        float dist_from = 1.0f;
        float dist_to = 50.0f;
        if (_core->registry.valid(_star_mouse_over)) {
            ImGui::Text("Selected star: %s", _core->registry.get<components::Name>(_star_mouse_over).name.c_str());
        }
        ImGui::DragScalar("Distance between stars (scroll value)", ImGuiDataType_Float, &_distance_between_stars, 0.5f, &dist_from, &dist_to, "%f");
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            _recalculate_graph();
        }
        if (ImGui::Button("Generate new world")) {
            populate();
        }
        if (!_ui_show_log && ImGui::Button("Show log")) {
            _ui_show_log = true;
        }
        if (_ui_show_log && ImGui::Button("Hide log")) {
            _ui_show_log = false;
        }
        // fleet battles
        static int selected_fleet_idx = 0;
        const auto fleet_view = _core->registry.view<components::Fleet>();
        std::vector<entt::entity> fleet_entities;
        std::vector<size_t> fleet_sizes;
        static entt::entity player_fleet = entt::null;
        static entt::entity selected_fleet = entt::null;
        auto &registry = _core->registry;
        fleet_view.each([&registry, &fleet_entities, &fleet_sizes](const entt::entity entity, const components::Fleet &fleet) {
            components::PlayerControlled *player_controlled = registry.try_get<components::PlayerControlled>(entity);
            if (!player_controlled) {
                fleet_entities.push_back(entity);
                fleet_sizes.push_back(fleet.ships.size());
            } else {
                player_fleet = entity;
            }
        });
        if (!fleet_entities.empty()) {
            char first_selection[40];
            std::sprintf(first_selection, "%d (%llu ships)", fleet_entities[selected_fleet_idx], fleet_sizes[selected_fleet_idx]);
            if (ImGui::BeginCombo("Fleets", first_selection)) {
                for (int idx = 0; idx < fleet_entities.size(); idx++) {
                    const bool is_selected = (selected_fleet_idx == idx);
                    char combo_item[20];
                    std::sprintf(combo_item, "%d (%llu ships)", fleet_entities[idx], fleet_sizes[idx]);
                    if (ImGui::Selectable(combo_item, is_selected)) {
                        selected_fleet_idx = idx;
                        selected_fleet = fleet_entities[selected_fleet_idx];
                        _core->game_log.message("Selected %d\n", selected_fleet);
                    }
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            if (player_fleet != entt::null && selected_fleet != entt::null) {
                ImGui::SameLine();
                if (ImGui::Button("Attack")) {
                    _core->dispatcher.enqueue<PlayerBattleStartEvent>(player_fleet, selected_fleet);
                }
                ImGui::SameLine();
                if (ImGui::Button("Select")) {
                    _selected_fleet = selected_fleet;
                }
            }
        }
        ImGui::EndTabItem();
    }
}

void Galaxy::_draw_ui_tab_camera() {
    static constexpr auto rotate_horizontal = [](auto &camera, auto &camera_settings) {
        UpdateCamera(&camera);
    };
    ImGui::PushButtonRepeat(true);
    if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
        _camera_settings.angle.x -= GetFrameTime() * 20.0f;
        rotate_horizontal(_camera, _camera_settings);
    }
    ImGui::SameLine();
    if (!_camera_settings.focus_on_clicked && ImGui::Button("Focus camera on selection")) {
        _camera_settings.focus_on_clicked = true;
        if (_selected_fleet != entt::null) {
            focus_camera(_camera, local_to_global_coords(_core->registry.get<Vector3>(_selected_fleet), _visible_size), _camera.fovy);
        }
    }

    float dist_from = 5.0f;
    float dist_to = 45.0f;
    ImGui::DragScalar("Zoom (scroll value)", ImGuiDataType_Float, &_camera.fovy, 0.25f, &dist_from, &dist_to, "< %.0f >");

    if (_camera_settings.focus_on_clicked && ImGui::Button("Reset camera")) {
        ImGui::SameLine();
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            _recalculate_graph();
        }
        _camera_settings.focus_on_clicked = false;
        focus_camera(_camera, Vector3{0.0f, 0.0f, 0.0f});
    }
    ImGui::PopButtonRepeat();
}

void Galaxy::_draw_ui_debug_log_window() {
    if (_ui_show_log) {
        _core->debug_log.render("Tleilax log");
    }
}

void Galaxy::_draw_ui_game_log_window() {
    bool open = true;
    _core->game_log.render_no_border("Messages", &open);
}