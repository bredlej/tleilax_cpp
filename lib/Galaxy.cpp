//
// Created by geoco on 19.12.2021.
//

#include "Galaxy.h"
#include <cmath>

constexpr auto seed_function = [](uint32_t x, uint32_t y, uint32_t z) {
    return ((x + y) >> 1) * (x + y + 1) + y * ((x + z) >> 1) * (x + z + 1) + z;
};

constexpr auto local_to_global_coords = [](auto coordinates, auto visible_size) -> Vector3 {
    return {coordinates.x - static_cast<float>(visible_size.x / 2), coordinates.y - static_cast<float>(visible_size.y / 2), coordinates.z - static_cast<float>(visible_size.z / 2)};
};

constexpr auto is_star_at = [](auto x, auto y, auto z, auto chance) {
    return 0 == chance;
};

void Galaxy::populate() {
    auto before = std::chrono::high_resolution_clock::now();
    for (int32_t z = 0; z < _visible_size.z; z++) {
        for (int32_t y = 0; y < _visible_size.y; y++) {
            for (int32_t x = 0; x < _visible_size.x; x++) {
                pcg32 pcg(seed_function(x, y, z));
                if (is_star_at(x, y, z, pcg(_star_occurence_chance))) {
                    const auto star = _registry.create();
                    _registry.emplace<Vector3>(star, static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));

                    if (pcg(100) < 5) {
                        const auto explosion_counter = pcg(15) + 1;
                        _registry.emplace<Exploding>(star, static_cast<uint8_t>(explosion_counter));
                        _registry.emplace<StarColor>(star, StarColor{255, 255, 255, 255});
                        _registry.emplace<Size>(star, static_cast<float>(explosion_counter));
                    } else {
                        _registry.emplace<StarColor>(star, StarColor{static_cast<uint8_t>(pcg(255)), static_cast<uint8_t>(pcg(255)), static_cast<uint8_t>(pcg(255)), 255});
                        _registry.emplace<Size>(star, 1.0f);

                        if (pcg(100) < 10) {
                            _registry.emplace<NovaSeeker>(star, pcg(5) + 1);
                        }
                    }
                }
            }
        }
    }
    auto after = std::chrono::high_resolution_clock::now() - before;
    std::printf("Elapsed time: %lld ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(after).count());
}

void Galaxy::_render_visible() const {

    BeginMode3D(_camera);
    DrawCubeWires({0., 0., 0.}, _visible_size.x, _visible_size.y, _visible_size.z, YELLOW);

    _registry.view<Vector3, StarColor, Size>().each([&](const entt::entity entity, const Vector3 &coords, const StarColor color, const Size size) {
        Vector3 star_coords = local_to_global_coords(coords, _visible_size);
        DrawSphere(star_coords, size.size, {color.r, color.g, color.b, color.a});
        if (_registry.any_of<Nova>(entity)) {
            DrawSphereWires(star_coords, 5, 6, 6, VIOLET);
        }
    });
    _registry.view<Fleet, Vector3, Size>().each([&](const entt::entity entity, const Fleet &fleet, const Vector3 pos, const Size size) {
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
static void change_course_upon_nearer_explosion(const Vector3 &explosion_position, const entt::entity entity, Fleet &fleet, Vector3 &position, Destination &destination, const Size size) {
    auto destination_vector = Vector3{static_cast<float>(destination.dest.x), static_cast<float>(destination.dest.y), static_cast<float>(destination.dest.z)};
    if (Vector3Distance(explosion_position, position) < Vector3Distance(destination_vector, position)) {
        destination = Destination{static_cast<int32_t>(explosion_position.x), static_cast<int32_t>(explosion_position.y), static_cast<int32_t>(explosion_position.z)};
    }
}
void Galaxy::_explode_stars(const ExplosionEvent &ev) {
    auto explosion_position = _registry.get<Vector3>(ev.e);
    _registry.remove<Exploding>(ev.e);
    _registry.emplace<Nova>(ev.e);
    std::printf("Explosion at [%g, %g, %g]\n", explosion_position.x, explosion_position.y, explosion_position.z);
    auto nova_seekers = _registry.view<NovaSeeker>();
    nova_seekers.each([this, explosion_position](const entt::entity entity, NovaSeeker &nova_seeker) {
        if (nova_seeker.capacity > 0) {
            _dispatcher.enqueue<NovaSeekEvent>(entity, explosion_position);
            nova_seeker.capacity -= 1;
        }
    });

    auto fleets = _registry.view<Fleet, Vector3, Destination, Size>();
    fleets.each([=](const entt::entity entity, Fleet &fleet, Vector3 &position, Destination &destination, const Size size) {
        change_course_upon_nearer_explosion(explosion_position, entity, fleet, position, destination, size);
    });
}

void Galaxy::_initialize() {
    _dispatcher.sink<ExplosionEvent>().connect<&Galaxy::_explode_stars>(this);
    _dispatcher.sink<NovaSeekEvent>().connect<&Galaxy::_send_fleet_to_nova>(this);
    _fleet_onclick_handle = [](const entt::registry &registry, const entt::entity entity) {

    };
}

void Galaxy::_tick() {

    auto view = _registry.view<Exploding, Size, Vector3>();
    view.each([this](const entt::entity entity, Exploding &exploding, Size &size, Vector3 &position) {
        if (exploding.counter > 0) {
            exploding.counter -= 1;
            size.size -= 1.0f;
        } else {
            _dispatcher.enqueue<ExplosionEvent>(entity);
        }
    });

    auto fleets = _registry.view<Fleet, Vector3, Destination, Size>();
    fleets.each(FleetEntity::update);

    _dispatcher.update();
}

void Galaxy::_send_fleet_to_nova(const NovaSeekEvent &ev) {
    FleetEntity fleet;
    fleet.react_to_nova(_registry, ev);
}

void FleetEntity::react_to_nova(entt::registry &registry, const NovaSeekEvent &ev) {
    if (_entity == entt::null) {
        auto position = registry.get<Vector3>(ev.e);
        _entity = create(registry, position);
    }
    registry.emplace<Destination>(_entity, Coordinates{static_cast<int32_t>(ev.destination.x), static_cast<int32_t>(ev.destination.y), static_cast<int32_t>(ev.destination.z)});
}

entt::entity FleetEntity::create(entt::registry &registry, Vector3 position) {
    auto entity = registry.create();
    registry.emplace<Vector3>(entity, position);
    registry.emplace<Size>(entity, 1.5f);
    registry.emplace<Fleet>(entity);

    return entity;
}

void FleetEntity::update(const entt::entity entity, Fleet &fleet, Vector3 &pos, const Destination destination, const Size size) {
    auto new_position = Vector3{static_cast<float>(destination.dest.x), static_cast<float>(destination.dest.y), static_cast<float>(destination.dest.z)};
    new_position = Vector3Normalize(Vector3Subtract(new_position, pos));
    pos = Vector3Add(pos, new_position);
    pos = Vector3{ceil(pos.x), ceil(pos.y), ceil(pos.z)};
}

void FleetEntity::on_click(const entt::registry &registry, entt::entity entity) {
    auto position = registry.get<Vector3>(entity);
    auto destination = registry.get<Destination>(entity);
    std::printf("entity = [%d], fleet at [%.1f, %.1f, %.1f] moving towards [%d, %d, %d]\n", entity, position.x, position.y, position.z, destination.dest.x, destination.dest.y, destination.dest.z);
}
