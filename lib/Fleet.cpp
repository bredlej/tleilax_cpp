//
// Created by geoco on 03.03.2022.
//
#include <fleet.h>

void FleetEntity::update(entt::registry &registry, const entt::entity entity, components::Fleet &fleet, Vector3 &pos, components::Path &path) {
    if (!path.checkpoints.empty()) {
        auto next_destination = registry.get<Vector3>(path.checkpoints.front());
        auto new_position = Vector3Normalize(Vector3Subtract(next_destination, pos));
        pos = Vector3Add(pos, new_position);
        if (Vector3Distance(pos, next_destination) < 1.0f) {
            path.checkpoints.erase(path.checkpoints.begin());
        }
    }
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
        print_ships_info(registry, entity);
    }
}