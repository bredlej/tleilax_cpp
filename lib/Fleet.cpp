//
// Created by geoco on 03.03.2022.
//
#include <fleet.h>


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

        std::printf("Ship=[%d] | Engine=[%s: %.d/%.d], Hull=[%s: %.1f/%.1f], Shield=[%s: %.1d], Weapon=[%s: %dd%d]\n",
                    ship,
                    engine.name.c_str(), engine.power, engine.weight,
                    hull.name.c_str(), hull.health, hull.max_health,
                    shield.name.c_str(), shield.defense,
                    weapon.name.c_str(), weapon.damage.amount, weapon.damage.sides);
    }
}
void FleetEntity::on_click(const entt::registry &registry, entt::entity entity) {
    if (registry.valid(entity)) {
        auto position = registry.get<Vector3>(entity);
        auto destination = registry.get<components::Destination>(entity);
        std::printf("entity = [%d], fleet at [%.1f, %.1f, %.1f] moving towards [%d, %d, %d]\n", entity, position.x, position.y, position.z, destination.dest.x, destination.dest.y, destination.dest.z);
        print_ships_info(registry, entity);
    }
}