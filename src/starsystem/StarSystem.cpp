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

    _registry.emplace<components::GravityCenter>(main_star_entity, 0, 100.0f);
    _populate_orbits_around_star(main_star_entity);
    _populate_planets();
}

void StarSystem::_populate_planets() {
    _registry.view<components::Orbit, components::Planet>()
            .each([&](const entt::entity planet, const components::Orbit, const components::Planet)
                  {
                      const auto amount_bodies_around_planet = _core->pcg(3);
                      if (amount_bodies_around_planet > 0) {
                          _registry.emplace<components::GravityCenter>(planet, 0, 10.0f);
                      }
                      for (int i = 0; i < amount_bodies_around_planet; i++) {
                          const auto body_orbiting_planet = _registry.create();
                          _registry.emplace<components::Orbit>(body_orbiting_planet, planet, i + 1.0f, 0.0f);
                          const auto d6 = _core->pcg(6) + 1;
                          if (d6 < 5) {
                              _registry.emplace<components::Moon>(body_orbiting_planet, i);
                          } else if (d6 == 5) {
                              _registry.emplace<components::SpaceStation>(body_orbiting_planet, i);
                          } else {
                              _registry.emplace<components::Anomaly>(body_orbiting_planet, i);
                          }
                      }
                  });
}
void StarSystem::_populate_orbits_around_star(const entt::entity &main_star_entity) {
    const auto amount_bodies_on_star_orbit = _core->pcg(18);
    for (int i = 0; i < amount_bodies_on_star_orbit; i++) {
        const auto star_orbiting_body = _registry.create();
        _registry.emplace<components::Orbit>(star_orbiting_body, main_star_entity, i + 1.0f, 0.0f);

        const auto d100 = _core->pcg(100) + 1;
        if (d100 <= 70) {
            _registry.emplace<components::Planet>(star_orbiting_body, i);
        }
        else if (d100 > 70 && d100 <= 95) {
            _registry.emplace<components::AsteroidBelt>(star_orbiting_body, i);
        }
        else {
            _registry.emplace<components::Anomaly>(star_orbiting_body, i);
        }
    }
}
