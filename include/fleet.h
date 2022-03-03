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
    static entt::entity create(entt::registry&, pcg32&, Vector3);
    void react_to_nova(entt::registry &, pcg32 &, const NovaSeekEvent &);
    static void update(entt::entity entity, components::Fleet &fleet, Vector3 &pos, components::Destination destination, components::Size size);
    static void on_click(const entt::registry &, entt::entity entity);
private:
    entt::entity _entity;
    static void populate_fleet_with_ships(entt::registry &, entt::entity, pcg32 &);
};


#endif//TLEILAX_FLEET_H
