//
// Created by geoco on 02.05.2022.
//
#include <galaxy.hpp>
#include <raylib_extension.h>
#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif
void Galaxy::render() {
    BeginDrawing();
    ClearBackground(_core->colors.col_0);
    _render_visible();
    DrawFPS(1200, 10);
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
    raylib_ext::RenderInstanced(_star_render_instance.model.meshes[0], _star_render_instance.model.materials[0], _star_render_instance.matrices, _star_render_instance.colors,_star_render_instance.count);
    _core->registry.view<Vector3, components::Star, components::Size>().each([&](const entt::entity entity, const Vector3 &coords, const components::Star color, const components::Size size) {
        Vector3 star_coords = local_to_global_coords(coords, _visible_size);
        bool is_mouse_over_star = GetRayCollisionSphere(GetMouseRay(GetMousePosition(), _camera), star_coords, size.size).hit;
        StarEntity::render(_core, _camera, _visible_size, entity, coords, color, size, is_mouse_over_star);
        if (is_mouse_over_star) {
            _entities_under_cursor.emplace_back(entity);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (_camera_settings.focus_on_clicked) {
                    focus_camera(_camera, star_coords, _camera.fovy);
                }
                _core->dispatcher.enqueue<StarSelectedEvent>(entity, seed_function(static_cast<uint32_t>(coords.x), static_cast<uint32_t>(coords.y), static_cast<uint32_t>(coords.z)));
            }
            _star_mouse_over = entity;
        }
    });
}

void StarEntity::render(const std::shared_ptr<Core> &core, const Camera &camera, const Vector3 &visible_size, const entt::entity entity, const Vector3 &coords, const components::Star color, const components::Size size, const bool is_selected) {
    Vector3 star_coords = local_to_global_coords(coords, visible_size);
    //DrawSphere(star_coords, size.size, {color.r, color.g, color.b, color.a});
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

static void _init_render_instance(Model &model, Shader &shader, EntityRenderInstance& render_instance) {
    render_instance.shader = shader;
    shader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(render_instance.shader, "mvp");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(render_instance.shader, "viewPos");
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(render_instance.shader, "instanceTransform");
    shader.locs[SHADER_LOC_VERTEX_COLOR] = GetShaderLocationAttrib(render_instance.shader, "vertexColor");

    // Ambient light level
    int ambientLoc = GetShaderLocation(render_instance.shader, "ambient");
    const float f[4] {1.0f, 1.0f, 1.0f, 1.0f};
    SetShaderValue(render_instance.shader, ambientLoc, f, SHADER_UNIFORM_VEC4);
    Vector3 v{50, 50, 0};
    CreateLight(LIGHT_DIRECTIONAL, v, Vector3Zero(), WHITE, render_instance.shader);

    // NOTE: We are assigning the intancing shader to material.shader
    // to be used on mesh drawing with DrawMeshInstanced()
    Material material = LoadMaterialDefault();
    material.shader = render_instance.shader;
    //material.maps[MATERIAL_MAP_DIFFUSE].color = YELLOW;
    model.materials[0] = material;

    render_instance.model = model;
}

void Galaxy::_init_star_render_instance() {
    Model m = LoadModelFromMesh(GenMeshSphere(1.0f, 12, 12));
    Shader shader = LoadShader("assets/shaders/glsl100/base_lighting_instanced.vs", "assets/shaders/glsl100/lighting.fs");
    _init_render_instance(m, shader, _star_render_instance);
}

static void _place_render_instance_at(EntityRenderInstance &render_instance, entt::entity entity, float x, float y, float z, Color c, const Vector3 visible_size, const Vector3 scale) {
    Matrix m = MatrixIdentity();
    Vector3 v = local_to_global_coords(Vector3{x, y, z}, visible_size);
    auto translation = MatrixTranslate(v.x, v.y, v.z);
    auto rotation = MatrixRotate({1,0,0}, 0);
    auto scaling = MatrixScale(scale.x, scale.y, scale.z);

    m = MatrixMultiply(m, scaling);
    m = MatrixMultiply(m, translation);
    m = MatrixMultiply(m, rotation);

    render_instance.matrices.emplace_back(m);
    render_instance.colors.emplace_back(c);
    render_instance.count += 1;
    render_instance.entities.emplace_back(entity);
}

void Galaxy::_place_star_instance_at(entt::entity entity, float x, float y, float z, Color c, const Vector3 visible_size) {
    const components::Size *size = _core->registry.try_get<components::Size>(entity);
    _place_render_instance_at(_star_render_instance, entity, x, y, z, c, visible_size, size ? Vector3{size->size, size->size, size->size} : Vector3{1.0f, 1.0f, 1.0f});
}
