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
#include <graph.h>

static constexpr uint32_t MAX_SHIPS_IN_FLEET = 10;

struct DistanceFunction {
    float operator()(const Vector3 first, const Vector3 second) const { return Vector3Distance(first, second); };
};

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
    void react_to_nova(entt::registry &registry, pcg32 &pcg, const NovaSeekEvent &ev, const auto &ship_components, const Graph<GraphNode, float, GraphNodeHash, GraphNodeEqualFunc> &graph) {
        if (_entity == entt::null) {
            auto position = registry.get<Vector3>(ev.source);
            _entity = create(registry, pcg, position, ship_components);
        }
        auto path_component = components::Path();
        path_component.checkpoints = calculate_path<Vector3, components::Star, DistanceFunction>(graph, registry, _entity, ev.destination);;
        registry.emplace<components::Path>(_entity, path_component);
        //registry.emplace<components::Destination>(_entity, components::Coordinates{static_cast<int32_t>(ev.destination.x), static_cast<int32_t>(ev.destination.y), static_cast<int32_t>(ev.destination.z)});
    }
    static void update(entt::entity entity, components::Fleet &fleet, Vector3 &pos, components::Destination destination, components::Size size);
    static void on_click(const entt::registry &, entt::entity entity);
private:
    entt::entity _entity;
    static void populate_fleet_with_ships(entt::registry &registry, entt::entity fleet_entity, pcg32 &pcg, const ShipComponentRepository &ship_components) {
        auto amount_ships = pcg(MAX_SHIPS_IN_FLEET + 1);
        std::vector<entt::entity> ships;
        std::vector<components::Weapon> weapons = get<components::Weapon>(ship_components);
        std::vector<components::Engine> engines = get<components::Engine>(ship_components);
        std::vector<components::Hull> hulls = get<components::Hull>(ship_components);
        std::vector<components::Shield> shields = get<components::Shield>(ship_components);
//-DCMAKE_TOOLCHAIN_FILE=/Users/bredlej/Coding/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake -DPLATFORM=Web
        for (int i = 0; i < amount_ships; i++) {
            components::Engine engine = engines[pcg(engines.size())];
            components::Hull hull = hulls[pcg(hulls.size())];
            components::Shield shield = shields[pcg(shields.size())];
            components::Weapon weapon = weapons[pcg(weapons.size())];

            entt::entity ship = Ship<components::Engine, components::Hull, components::Shield, components::Weapon>::create_with_components(registry, engine, hull, shield, weapon);

            ships.emplace_back(ship);
        }
        registry.emplace<components::Fleet>(fleet_entity, ships);
    }
};


#endif//TLEILAX_FLEET_H
