//
// Created by geoco on 02.05.2022.
//
#include <galaxy.h>

void Galaxy::render() {
    BeginDrawing();

    ClearBackground(_core->colors.col_0);
    _render_visible();
    DrawFPS(1240, 10);
    _draw_ui();
    EndDrawing();
}

void Galaxy::_render_visible() {

    BeginMode3D(_camera);
    _entities_under_cursor.clear();
    _render_stars();
    _render_paths();
    _render_fleets();
    _render_mouse_selection();
    EndMode3D();
}

void Galaxy::_render_fleets() {
    _core->registry.view<components::Fleet, Vector3, components::Size>().each([&](const entt::entity entity, const components::Fleet &fleet, const Vector3 pos, const components::Size size) {
        Vector3 fleet_coords = local_to_global_coords(pos, _visible_size);
        fleet_coords.y += 1;
        const auto fleet_size_top = size.size / 2;
        const auto fleet_size_bottom = size.size / 4;
        BoundingBox fleet_bounds{};
        fleet_bounds.min = {fleet_coords.x - fleet_size_bottom, fleet_coords.y - (size.size) + (size.size), fleet_coords.z - fleet_size_bottom};
        fleet_bounds.max = {fleet_coords.x + fleet_size_bottom, fleet_coords.y + (size.size) + (size.size), fleet_coords.z + fleet_size_bottom};
        if (GetRayCollisionBox(GetMouseRay(GetMousePosition(), _camera), fleet_bounds).hit) {
            DrawCylinder(fleet_coords, fleet_size_top, fleet_size_bottom, 3, 4, Colors::col_15);
            _entities_under_cursor.emplace_back(entity);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (_camera_settings.focus_on_clicked) {
                    focus_camera(_camera, fleet_coords, 15.0f);
                }
                FleetEntity::on_click(_core->registry, entity);
                _selected_fleet = entity;
                auto *fleet_path = _core->registry.try_get<components::Path>(entity);
                if (fleet_path && !fleet_path->checkpoints.empty()) {
                    _register_path_selection(fleet_path->checkpoints);
                }
            }
        } else {
            const auto is_player_fleet = _core->registry.try_get<components::PlayerControlled>(entity);
            if (is_player_fleet) {
                DrawCylinder(fleet_coords, fleet_size_top, fleet_size_bottom, 3, 4, Colors::col_16);
            } else {
                const auto is_tleilaxian = _core->registry.try_get<components::Tleilaxian>(entity);
                if (!is_tleilaxian) {
                    DrawCylinder(fleet_coords, fleet_size_top, fleet_size_bottom, 3, 4, Colors::col_9);
                } else {
                    DrawCylinder(fleet_coords, fleet_size_top, fleet_size_bottom, 3, 4, Colors::col_11);
                }
            }
        }
    });
}

void Galaxy::_render_paths() {
    std::for_each(stars_paths.begin(), stars_paths.end(), [&](const std::pair<Vector3, Vector3> &neighbours) {
        DrawLine3D(local_to_global_coords(neighbours.first, _visible_size), local_to_global_coords(neighbours.second, _visible_size), Colors::col_3);
    });
}

void Galaxy::_render_stars() {
    _core->registry.view<Vector3, components::Star, components::Size>().each([&](const entt::entity entity, const Vector3 &coords, const components::Star color, const components::Size size) {
        Vector3 star_coords = local_to_global_coords(coords, _visible_size);
        bool star_is_selected = GetRayCollisionSphere(GetMouseRay(GetMousePosition(), _camera), star_coords, size.size).hit;
        StarEntity::render(_core, _camera, _visible_size, entity, coords, color, size, star_is_selected);
        if (star_is_selected) {
            _entities_under_cursor.emplace_back(entity);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (_camera_settings.focus_on_clicked) {
                    focus_camera(_camera, star_coords, _camera.fovy);
                }
                _core->dispatcher.enqueue<StarSelectedEvent>(entity);
            }
            _selected_star = entity;
        }
    });
}

void StarEntity::render(const std::shared_ptr<Core> &core, const Camera &camera, const Vector3 &visible_size, const entt::entity entity, const Vector3 &coords, const components::Star color, const components::Size size, const bool is_selected) {
    Vector3 star_coords = local_to_global_coords(coords, visible_size);
    DrawSphere(star_coords, size.size, {color.r, color.g, color.b, color.a});
    EndMode3D();

    auto *name = core->registry.try_get<components::Name>(entity);
    if (name) {
        auto name_pos = IsWindowFullscreen() ? GetWorldToScreenEx(star_coords, camera, GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor())) : GetWorldToScreenEx(star_coords, camera, core->window.width, core->window.height);
        DrawRectangle(static_cast<int>(name_pos.x - 22), static_cast<int>(name_pos.y - 22), name->name.length() * 8, 12, Colors::col_0);
        DrawText(name->name.c_str(), static_cast<int>(name_pos.x - 20), static_cast<int>(name_pos.y - 20), 10, Colors::col_16);
    }

    BeginMode3D(camera);
}

void Galaxy::_render_mouse_selection() {
    if (!_entities_under_cursor.empty()) {
        auto front_entity = _entities_under_cursor.back();
        auto size = _core->registry.get<components::Size>(front_entity);
        auto position = local_to_global_coords(_core->registry.get<Vector3>(front_entity), _visible_size);
        if (_core->registry.try_get<components::Star>(front_entity)) {
            DrawSphereWires(position, size.size + 2, 6, 6, Colors::col_16);
        }
    }
}