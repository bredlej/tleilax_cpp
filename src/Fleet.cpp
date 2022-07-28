//
// Created by geoco on 03.03.2022.
//
#include <fleet.hpp>

void FleetEntity::update(const std::shared_ptr<Core>& core, const entt::entity entity, components::Fleet &fleet, Vector3 &pos, components::Path &path) {
    if (!path.checkpoints.empty()) {
        auto next_destination = core->registry.get<Vector3>(path.checkpoints.front());
        auto new_position = Vector3Normalize(Vector3Subtract(next_destination, pos));
        pos = Vector3Add(pos, new_position);
        if (Vector3Distance(pos, next_destination) < 1.0f) {
            core->dispatcher.enqueue<ArrivalEvent>(entity, path.checkpoints.front());
            path.checkpoints.erase(path.checkpoints.begin());
        }
    }
    const auto &my_vicinity = core->registry.get<components::Vicinity>(entity);
    core->registry.view<components::Fleet, Vector3, components::Vicinity>().each([&core, my_vicinity, entity, pos](entt::entity other, components::Fleet fleet, Vector3 position, components::Vicinity &vicinity){
        if (entity != other) {
            if (Vector3Distance(pos, position) < 1.5f) {
                if (std::find(my_vicinity.objects.begin(), my_vicinity.objects.end(), other) == my_vicinity.objects.end()) {
                    core->dispatcher.enqueue<ArrivalEvent>(entity, other);
                }
            }
        }
    });
}

void FleetEntity::on_click(const entt::registry &registry, entt::entity entity) {
    /* do nothing */
}