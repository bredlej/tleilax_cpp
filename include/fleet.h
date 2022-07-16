//
// Created by geoco on 03.03.2022.
//

#ifndef TLEILAX_FLEET_H
#define TLEILAX_FLEET_H

#include <components.h>
#include <entt/entt.hpp>
#include <events.h>
#include <graph.h>
#include <pcg/pcg_random.hpp>
#include <raymath.h>
#include <ship.h>
#include <core.h>
#include <memory>

static constexpr uint32_t MAX_SHIPS_IN_FLEET = 10;

struct DistanceFunction {
    float operator()(const Vector3 first, const Vector3 second) const { return Vector3Distance(first, second); };
};

class FleetEntity {
public:
    explicit FleetEntity() noexcept : _entity{ entt::null } {};
    FleetEntity(const FleetEntity&) noexcept = delete;
    FleetEntity(FleetEntity&&) noexcept = delete;
    FleetEntity& operator=(const FleetEntity&) noexcept = delete;
    FleetEntity& operator=(FleetEntity&&) noexcept = delete;
    ~FleetEntity() noexcept = default;

    static entt::entity create(const std::shared_ptr<Core> &core, pcg32 &pcg, Vector3 position, const ShipComponentRepository &ship_components) {
        auto entity = core->registry.create();
        core->registry.emplace<Vector3>(entity, position);
        core->registry.emplace<components::Size>(entity, 1.5f);
        core->registry.emplace<components::Range>(entity, 20.0f);
        core->registry.emplace<components::KnownStarSystems>(entity);
        populate_fleet_with_ships(core->registry, entity, pcg, ship_components);
        core->dispatcher.enqueue<FleetCreationEvent>(entity);
        return entity;
    };
    void react_to_nova(const std::shared_ptr<Core> &core, pcg32 &pcg, const NovaSeekEvent &ev, const auto &ship_components, const Graph<GraphNode, float, GraphNodeHash, GraphNodeEqualFunc> &graph) {
        if (_entity == entt::null) {
            auto position = core->registry.get<Vector3>(ev.source);
            _entity = create(core,pcg, position, ship_components);
        }
        auto path_component = components::Path();
        path_component.checkpoints = calculate_path<Vector3, DistanceFunction, components::Star>(graph, core->registry, ev.source, ev.destination);
        core->registry.emplace<components::Path>(_entity, path_component);
    }
    static void update(const std::shared_ptr<Core> &, entt::entity entity, components::Fleet &fleet, Vector3&, components::Path &path);
    static void on_click(const entt::registry &, entt::entity entity);
    entt::entity get_entity() {return _entity;}
private:
    entt::entity _entity;
    static void populate_fleet_with_ships(entt::registry &registry, entt::entity fleet_entity, pcg32 &pcg, const ShipComponentRepository &ship_components) {
        auto amount_ships = pcg(MAX_SHIPS_IN_FLEET) + 1;
        std::vector<entt::entity> ships;
        std::vector<components::Weapon> weapons = get<components::Weapon>(ship_components);
        std::vector<components::Engine> engines = get<components::Engine>(ship_components);
        std::vector<components::Hull> hulls = get<components::Hull>(ship_components);
        std::vector<components::Shield> shields = get<components::Shield>(ship_components);
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
