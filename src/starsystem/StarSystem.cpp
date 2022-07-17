//
// Created by Patryk Szczypień on 10/07/2022.
//
#include <star_system.hpp>

void StarSystem::update() {

}

entt::entity StarSystem::_create_main_star_entity(const entt::entity entity) {
    const components::Name star_name = _core->registry.get<components::Name>(entity);
    const components::Star star_type = _core->registry.get<components::Star>(entity);

    entt::entity main_star = _registry.create();
    _registry.emplace<components::Name>(main_star, star_name);
    _registry.emplace<components::Star>(main_star, star_type);

    return main_star;
}

void StarSystem::_populate(const uint32_t seed, const entt::entity main_star_entity) {
    _core->pcg.seed(seed);
    _core->debug_log.message("Populating %d\n", seed);

    _registry.emplace<components::GravityCenter>(main_star_entity);
}