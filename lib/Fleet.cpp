//
// Created by geoco on 03.03.2022.
//
#include <fleet.h>

void FleetEntity::react_to_nova(entt::registry &registry, pcg32 &pcg, const NovaSeekEvent &ev) {
    if (_entity == entt::null) {
        auto position = registry.get<Vector3>(ev.e);
        _entity = create(registry, pcg, position);
    }
    registry.emplace<components::Destination>(_entity, components::Coordinates{static_cast<int32_t>(ev.destination.x), static_cast<int32_t>(ev.destination.y), static_cast<int32_t>(ev.destination.z)});
}

entt::entity FleetEntity::create(entt::registry &registry, pcg32 &pcg, Vector3 position) {
    auto entity = registry.create();
    registry.emplace<Vector3>(entity, position);
    registry.emplace<components::Size>(entity, 1.5f);

    populate_fleet_with_ships(registry, entity, pcg);

    return entity;
}

void FleetEntity::update(const entt::entity entity, components::Fleet &fleet, Vector3 &pos, const components::Destination destination, const components::Size size) {
    auto new_position = Vector3{static_cast<float>(destination.dest.x), static_cast<float>(destination.dest.y), static_cast<float>(destination.dest.z)};
    new_position = Vector3Normalize(Vector3Subtract(new_position, pos));
    pos = Vector3Add(pos, new_position);
    pos = Vector3{ceil(pos.x), ceil(pos.y), ceil(pos.z)};
}

void print_ships_info(const entt::registry &registry, const entt::entity &fleet_entity) {
    const components::Fleet fleet = registry.get<components::Fleet>(fleet_entity);
    std::printf("Fleet=[%d] has %zd ships:\n", fleet_entity, fleet.ships.size());
    for (auto ship : fleet.ships) {
        const auto engine = registry.get<components::Engine>(ship);
        const auto hull = registry.get<components::Hull>(ship);
        const auto shield = registry.get<components::Shield>(ship);
        const auto weapon = registry.get<components::Weapon>(ship);

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
    auto destination = registry.get<components::Destination>(entity);
    std::printf("entity = [%d], fleet at [%.1f, %.1f, %.1f] moving towards [%d, %d, %d]\n", entity, position.x, position.y, position.z, destination.dest.x, destination.dest.y, destination.dest.z);
    print_ships_info(registry, entity);
}

void FleetEntity::populate_fleet_with_ships(entt::registry &registry, entt::entity fleet_entity, pcg32 &pcg) {
    auto amount_ships = pcg(MAX_SHIPS_IN_FLEET);
    std::vector<entt::entity> ships;
    for (int i = 0; i < amount_ships; i++) {
        components::Engine engine{static_cast<float>(pcg(10)), static_cast<float>(pcg(10) + pcg(10))};
        components::Hull hull{static_cast<float>(pcg(10)), static_cast<float>(pcg(10) + pcg(10))};
        components::Shield shield{static_cast<float>(pcg(10))};
        components::Weapon weapon{"w1", "w1", 1, {1,1}, 3};

        entt::entity ship = Ship<components::Engine, components::Hull, components::Shield, components::Weapon>::create_with_components(registry, engine, hull, shield, weapon);

        ships.emplace_back(ship);
    }
    registry.emplace<components::Fleet>(fleet_entity, ships);
}