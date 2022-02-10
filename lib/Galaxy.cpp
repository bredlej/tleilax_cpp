//
// Created by geoco on 19.12.2021.
//

#include "Galaxy.h"
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

    _registry.view<Vector3, StarColor, Size>().each([&](const entt::entity entity, const Vector3 &coords, const StarColor color, const Size size) {
        StarEntity::render(_registry, _visible_size, entity, coords, color, size);
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
    fleet.react_to_nova(_registry, _pcg, ev);
}

void FleetEntity::react_to_nova(entt::registry &registry, pcg32 &pcg, const NovaSeekEvent &ev) {
    if (_entity == entt::null) {
        auto position = registry.get<Vector3>(ev.e);
        _entity = create(registry, pcg, position);
    }
    registry.emplace<Destination>(_entity, Coordinates{static_cast<int32_t>(ev.destination.x), static_cast<int32_t>(ev.destination.y), static_cast<int32_t>(ev.destination.z)});
}

entt::entity FleetEntity::create(entt::registry &registry, pcg32 &pcg, Vector3 position) {
    auto entity = registry.create();
    registry.emplace<Vector3>(entity, position);
    registry.emplace<Size>(entity, 1.5f);

    populate_fleet_with_ships(registry, entity, pcg);

    return entity;
}

void FleetEntity::update(const entt::entity entity, Fleet &fleet, Vector3 &pos, const Destination destination, const Size size) {
    auto new_position = Vector3{static_cast<float>(destination.dest.x), static_cast<float>(destination.dest.y), static_cast<float>(destination.dest.z)};
    new_position = Vector3Normalize(Vector3Subtract(new_position, pos));
    pos = Vector3Add(pos, new_position);
    pos = Vector3{ceil(pos.x), ceil(pos.y), ceil(pos.z)};
}

void print_ships_info(const entt::registry &registry, const entt::entity &fleet_entity) {
    const Fleet fleet = registry.get<Fleet>(fleet_entity);
    std::printf("Fleet=[%d] has %d ships:\n", fleet_entity, fleet.ships.size());
    for (auto ship : fleet.ships) {
        const auto engine = registry.get<Engine>(ship);
        const auto hull = registry.get<Hull>(ship);
        const auto shield = registry.get<Shield>(ship);
        const auto weapon = registry.get<Weapon>(ship);

        std::printf("Ship=[%d] | Engine=[%.1f/%.1f], Hull=[%.1f/%.1f], Shield=[%.1f], Weapon=[%.1f]\n",
                    ship,
                    engine.speed, engine.max_speed,
                    hull.health, hull.max_health,
                    shield.absorption,
                    weapon.damage);
    }
}
void FleetEntity::on_click(const entt::registry &registry, entt::entity entity) {
    auto position = registry.get<Vector3>(entity);
    auto destination = registry.get<Destination>(entity);
    std::printf("entity = [%d], fleet at [%.1f, %.1f, %.1f] moving towards [%d, %d, %d]\n", entity, position.x, position.y, position.z, destination.dest.x, destination.dest.y, destination.dest.z);
    print_ships_info(registry, entity);
}

void FleetEntity::populate_fleet_with_ships(entt::registry &registry, entt::entity fleet_entity, pcg32 &pcg) {
    auto amount_ships = pcg(MAX_SHIPS_IN_FLEET);
    std::vector<entt::entity> ships;
    for (int i = 0; i < amount_ships; i++) {
        entt::entity ship = registry.create();

        Engine engine{static_cast<float>(pcg(10)), static_cast<float>(pcg(10) + pcg(10))};
        Hull hull{static_cast<float>(pcg(10)), static_cast<float>(pcg(10) + pcg(10))};
        Shield shield{static_cast<float>(pcg(10))};
        Weapon weapon{static_cast<float>(pcg(10))};

        registry.emplace<Engine>(ship, engine);
        registry.emplace<Hull>(ship, hull);
        registry.emplace<Shield>(ship, shield);
        registry.emplace<Weapon>(ship, weapon);

        ships.emplace_back(ship);
    }
    registry.emplace<Fleet>(fleet_entity, ships);
}

entt::entity StarEntity::create_at(entt::registry &registry, pcg32 &pcg, Vector3 position) {
    if (pcg(_occurence_chance) == 0) {
        _entity = registry.create();
        registry.emplace<Vector3>(_entity, position);

        if (pcg(_exploding_chance.upper_bound) < _exploding_chance.occurs_if_less_then) {
            const auto explosion_counter = pcg(15) + 1;
            registry.emplace<Exploding>(_entity, static_cast<uint8_t>(explosion_counter));
            registry.emplace<StarColor>(_entity, StarColor{255, 255, 255, 255});
            registry.emplace<Size>(_entity, static_cast<float>(explosion_counter));
        } else {
            registry.emplace<StarColor>(_entity, StarColor{static_cast<uint8_t>(pcg(255)), static_cast<uint8_t>(pcg(255)), static_cast<uint8_t>(pcg(255)), 255});
            registry.emplace<Size>(_entity, 1.0f);

            if (pcg(_nova_seeker_chance.upper_bound) < _nova_seeker_chance.occurs_if_less_then) {
                registry.emplace<NovaSeeker>(_entity, pcg(5) + 1);
            }
        }
    }

    return _entity;
}

bool StarEntity::is_created() {
    return _entity != entt::null;
}
void StarEntity::render(const entt::registry &registry, const Vector3 &visible_size, const entt::entity entity, const Vector3 &coords, const StarColor color, const Size size) {
    Vector3 star_coords = local_to_global_coords(coords, visible_size);
    DrawSphere(star_coords, size.size, {color.r, color.g, color.b, color.a});
    if (registry.any_of<Nova>(entity)) {
        DrawSphereWires(star_coords, 5, 6, 6, VIOLET);
    }
}
