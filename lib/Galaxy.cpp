//
// Created by geoco on 19.12.2021.
//

#include "galaxy.h"
#include <cmath>

constexpr auto seed_function = [](const uint32_t x, const uint32_t y, const uint32_t z) {
    return ((x + y) >> 1) * (x + y + 1) + y * ((x + z) >> 1) * (x + z + 1) + z;
};

constexpr auto local_to_global_coords = [](const auto coordinates, const auto visible_size) -> Vector3 {
    return {coordinates.x - static_cast<float>(visible_size.x / 2), coordinates.y - static_cast<float>(visible_size.y / 2), coordinates.z - static_cast<float>(visible_size.z / 2)};
};

void Galaxy::populate() {
    auto before = std::chrono::high_resolution_clock::now();
    for (int32_t z = 0; z < _visible_size.z; z++) {
        for (int32_t y = 0; y < _visible_size.y; y++) {
            for (int32_t x = 0; x < _visible_size.x; x++) {
                StarEntity star(7000, {100, 5}, {100, 10});
                next_random_number(seed_function(x, y, z));
                star.create_at(_registry, _pcg, Vector3{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});
            }
        }
    }
    auto after = std::chrono::high_resolution_clock::now() - before;
    std::printf("Elapsed time: %lld ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(after).count());
}

void Galaxy::_render_visible() const {

    BeginMode3D(_camera);
    DrawCubeWires({0., 0., 0.}, _visible_size.x, _visible_size.y, _visible_size.z, YELLOW);

    _registry.view<Vector3, components::StarColor, components::Size>().each([&](const entt::entity entity, const Vector3 &coords, const components::StarColor color, const components::Size size) {
        StarEntity::render(_registry, _visible_size, entity, coords, color, size);
    });

    _registry.view<components::Fleet, Vector3, components::Size>().each([&](const entt::entity entity, const components::Fleet &fleet, const Vector3 pos, const components::Size size) {
        Vector3 fleet_coords = local_to_global_coords(pos, _visible_size);
        if (GetRayCollisionSphere(GetMouseRay(GetMousePosition(), _camera), fleet_coords, size.size).hit) {
            DrawSphereWires(fleet_coords, size.size, 6, 6, RED);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                FleetEntity::on_click(_registry, entity);
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

void Galaxy::render() const {
    BeginDrawing();
    ClearBackground(BLACK);
    _render_visible();
    DrawFPS(10, 10);
    EndDrawing();
}

void Galaxy::update() {
    if (IsKeyDown(KEY_A)) {
        UpdateCamera(&_camera);
    }
    if (IsKeyPressed(KEY_SPACE)) {
        _tick();
    }
}
static void change_course_upon_nearer_explosion(const Vector3 &explosion_position, const entt::entity entity, components::Fleet &fleet, Vector3 &position, components::Destination &destination, const components::Size size) {
    auto destination_vector = Vector3{static_cast<float>(destination.dest.x), static_cast<float>(destination.dest.y), static_cast<float>(destination.dest.z)};
    if (Vector3Distance(explosion_position, position) < Vector3Distance(destination_vector, position)) {
        destination = components::Destination{static_cast<int32_t>(explosion_position.x), static_cast<int32_t>(explosion_position.y), static_cast<int32_t>(explosion_position.z)};
    }
}
void Galaxy::_explode_stars(const ExplosionEvent &ev) {
    auto explosion_position = _registry.get<Vector3>(ev.e);
    _registry.remove<components::Exploding>(ev.e);
    _registry.emplace<components::Nova>(ev.e);
    std::printf("Explosion at [%g, %g, %g]\n", explosion_position.x, explosion_position.y, explosion_position.z);
    auto nova_seekers = _registry.view<components::NovaSeeker>();
    nova_seekers.each([this, explosion_position](const entt::entity entity, components::NovaSeeker &nova_seeker) {
        if (nova_seeker.capacity > 0) {
            _dispatcher.enqueue<NovaSeekEvent>(entity, explosion_position);
            nova_seeker.capacity -= 1;
        }
    });

    auto fleets = _registry.view<components::Fleet, Vector3, components::Destination, components::Size>();
    fleets.each([=](const entt::entity entity, components::Fleet &fleet, Vector3 &position, components::Destination &destination, const components::Size size) {
        change_course_upon_nearer_explosion(explosion_position, entity, fleet, position, destination, size);
    });
}

void Galaxy::_initialize() {
    _dispatcher.sink<ExplosionEvent>().connect<&Galaxy::_explode_stars>(this);
    _dispatcher.sink<NovaSeekEvent>().connect<&Galaxy::_send_fleet_to_nova>(this);
}

void Galaxy::_tick() {

    auto view = _registry.view<components::Exploding, components::Size, Vector3>();
    view.each([this](const entt::entity entity, components::Exploding &exploding, components::Size &size, Vector3 &position) {
        if (exploding.counter > 0) {
            exploding.counter -= 1;
            size.size -= 1.0f;
        } else {
            _dispatcher.enqueue<ExplosionEvent>(entity);
        }
    });

    auto fleets = _registry.view<components::Fleet, Vector3, components::Destination, components::Size>();
    fleets.each(FleetEntity::update);

    _dispatcher.update();
}

void Galaxy::_send_fleet_to_nova(const NovaSeekEvent &ev) {
    FleetEntity fleet;
    fleet.react_to_nova(_registry, _pcg, ev);
}


entt::entity StarEntity::create_at(entt::registry &registry, pcg32 &pcg, Vector3 position) {
    if (pcg(_occurence_chance) == 0) {
        _entity = registry.create();
        registry.emplace<Vector3>(_entity, position);

        if (pcg(_exploding_chance.upper_bound) < _exploding_chance.occurs_if_less_then) {
            const auto explosion_counter = pcg(15) + 1;
            registry.emplace<components::Exploding>(_entity, static_cast<uint8_t>(explosion_counter));
            registry.emplace<components::StarColor>(_entity, components::StarColor{255, 255, 255, 255});
            registry.emplace<components::Size>(_entity, static_cast<float>(explosion_counter));
        } else {
            registry.emplace<components::StarColor>(_entity, components::StarColor{static_cast<uint8_t>(pcg(255)), static_cast<uint8_t>(pcg(255)), static_cast<uint8_t>(pcg(255)), 255});
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
void StarEntity::render(const entt::registry &registry, const Vector3 &visible_size, const entt::entity entity, const Vector3 &coords, const components::StarColor color, const components::Size size) {
    Vector3 star_coords = local_to_global_coords(coords, visible_size);
    DrawSphere(star_coords, size.size, {color.r, color.g, color.b, color.a});
    if (registry.any_of<components::Nova>(entity)) {
        DrawSphereWires(star_coords, 5, 6, 6, VIOLET);
    }
}
