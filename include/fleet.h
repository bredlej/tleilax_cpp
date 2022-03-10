//
// Created by geoco on 03.03.2022.
//

#ifndef TLEILAX_FLEET_H
#define TLEILAX_FLEET_H

#include <entt/entt.hpp>
#include <pcg/pcg_random.hpp>
#include <components.h>
#include <events.h>
#include <raymath.h>
#include <ship.h>

static constexpr uint32_t MAX_SHIPS_IN_FLEET = 10;

class FleetEntity {
public:
    FleetEntity() : _entity{ entt::null } {};
    static entt::entity create(entt::registry &registry, pcg32 &pcg, Vector3 position, const ShipComponentRepository &ship_components) {
        auto entity = registry.create();
        registry.emplace<Vector3>(entity, position);
        registry.emplace<components::Size>(entity, 1.5f);

        populate_fleet_with_ships(registry, entity, pcg, ship_components);

        return entity;
    };
    void react_to_nova(entt::registry &registry, pcg32 &pcg, const NovaSeekEvent &ev, const auto &ship_components) {
        if (_entity == entt::null) {
            auto position = registry.get<Vector3>(ev.source);
            _entity = create(registry, pcg, position, ship_components);
        }
        registry.emplace<components::Destination>(_entity, components::Coordinates{static_cast<int32_t>(ev.destination.x), static_cast<int32_t>(ev.destination.y), static_cast<int32_t>(ev.destination.z)});
    }
    static void update(entt::entity entity, components::Fleet &fleet, Vector3 &pos, components::Destination destination, components::Size size);
    static void on_click(const entt::registry &, entt::entity entity);
private:
    entt::entity _entity;
    static void populate_fleet_with_ships(entt::registry &registry, entt::entity fleet_entity, pcg32 &pcg, const ShipComponentRepository &ship_components) {
        auto amount_ships = pcg(MAX_SHIPS_IN_FLEET);
        std::vector<entt::entity> ships;
        std::vector<components::Weapon> weapons = get<components::Weapon>(ship_components);
        std::vector<components::Engine> engines = get<components::Engine>(ship_components);
        for (int i = 0; i < amount_ships; i++) {
            components::Engine engine = engines[pcg(engines.size())];
            components::Hull hull{static_cast<float>(pcg(10)), static_cast<float>(pcg(10) + pcg(10))};
            components::Shield shield{"s1", "s1", 1, 1, 3};
            components::Weapon weapon = weapons[pcg(weapons.size())];

            entt::entity ship = Ship<components::Engine, components::Hull, components::Shield, components::Weapon>::create_with_components(registry, engine, hull, shield, weapon);

            ships.emplace_back(ship);
        }
        registry.emplace<components::Fleet>(fleet_entity, ships);
    }
};


#endif//TLEILAX_FLEET_H
