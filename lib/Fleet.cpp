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

void FleetEntity::on_click(const entt::registry &registry, entt::entity entity) {
    /* do nothing */
}