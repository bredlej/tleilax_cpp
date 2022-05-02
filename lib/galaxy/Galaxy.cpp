//
// Created by geoco on 19.12.2021.
//

#include "galaxy.h"
#include <name_generator.h>

template<>
std::string NameGenerator::get_random_name<components::Star>(pcg32 &pcg) {
    auto select_random_name = [&]() {
        auto category = static_cast<NameType>(pcg(_names.size()));

        return _names[category][pcg(_names[category].size())];//.//[pcg(_names[static_cast<std::underlying_type_t<NameType>>(category)].size())];
    };
    auto randomize_name = [&]() {
        auto name = select_random_name();
        bool has_suffix = pcg(100) <= 50;
        if (has_suffix) {
            bool is_suffix_a_name = pcg(100) <= 50;
            if (is_suffix_a_name) {
                return name + " " + select_random_name();
            } else {
                return name + " " + star_suffixes[pcg(star_suffixes.size())];
            }
        }
        return name;
    };
    return randomize_name();
}

void Galaxy::_recalculate_graph() {
    _clear_paths();
    stars_paths.clear();
    stars_graph.clear();
    _core->registry.view<Vector3, components::Star, components::Size>().each(
            [&](const entt::entity entity, const Vector3 &coords, const components::Star color, const components::Size size) {
                GraphNode starNode{entity, true};
                _core->registry.view<Vector3, components::Star, components::Size>().each(
                        [&](const entt::entity _entity, const Vector3 &_coords, const components::Star _color, const components::Size _size) {
                            if (entity != _entity) {
                                GraphNode next{_entity, false};
                                const auto distance = Vector3Distance(coords, _coords);
                                if (distance < distance_between_stars) {
                                    stars_graph.add_edge(starNode, next, distance, false);
                                    stars_paths.emplace_back(std::make_pair(coords, _coords));
                                }
                            }
                        });
            });
}

void Galaxy::_initialize() {
    _path = Path{};
    _core->dispatcher.sink<ExplosionEvent>().connect<&Galaxy::_explode_stars>(this);
    _core->dispatcher.sink<NovaSeekEvent>().connect<&Galaxy::_send_fleet_to_nova>(this);
    _core->dispatcher.sink<ArrivalEvent>().connect<&Galaxy::_fleet_arrived_at_star>(this);
    _core->dispatcher.sink<LeaveEvent>().connect<&Galaxy::_entity_left_vicinity>(this);
    _core->dispatcher.sink<StarSelectedEvent>().connect<&Galaxy::_on_star_selected>(this);
}

void Galaxy::populate() {
    auto before = std::chrono::high_resolution_clock::now();
    _core->registry.clear();
    selected_paths.clear();
    _selected_entity = entt::null;

    for (int32_t z = 0; z < static_cast<int32_t>(_visible_size.z); z++) {
        for (int32_t y = 0; y < static_cast<int32_t>(_visible_size.y); y++) {
            for (int32_t x = 0; x < static_cast<int32_t>(_visible_size.x); x++) {
                StarEntity star(_star_occurence_chance, {100, 5}, {100, 10});
                next_random_number(seed_function(x, y, z));
                star.create_at(_core->registry, _core, Vector3{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});
            }
        }
    }

    _generate_player_entity();
    _recalculate_graph();

    auto after = std::chrono::high_resolution_clock::now() - before;
    std::printf("Elapsed time: %lld ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(after).count());
}



void Galaxy::update() {
    if (IsKeyDown(KEY_A)) {
        UpdateCamera(&_camera);
    } else if (IsKeyPressed(KEY_T)) {
        _tick();
    } else if (IsKeyPressed(KEY_C)) {
        _clear_paths();
    } else if (IsKeyPressed(KEY_R)) {
        populate();
    }
    if (IsKeyPressed(KEY_D)) {
        open_demo = !open_demo;
    }
    _core->dispatcher.update();
}

void Galaxy::_tick() {

    auto view = _core->registry.view<components::Exploding, components::Size, Vector3>();
    view.each([this](const entt::entity entity, components::Exploding &exploding, components::Size &size, Vector3 &position) {
        if (exploding.counter > 0) {
            exploding.counter -= 1;
            size.size -= 1.0f;
        } else {
            _core->dispatcher.enqueue<ExplosionEvent>(entity);
        }
    });

    auto fleets = _core->registry.view<components::Fleet, Vector3, components::Path>();
    fleets.each([this](const entt::entity entity, components::Fleet &fleet, Vector3 &position, components::Path &path) {
        FleetEntity::update(_core, entity, fleet, position, path);

    });

    _update_vicinities();
}

static void change_course_upon_nearer_explosion(const Vector3 &explosion_position, const entt::entity entity, components::Fleet &fleet, Vector3 &position, components::Destination &destination, const components::Size size) {
    auto destination_vector = Vector3{static_cast<float>(destination.dest.x), static_cast<float>(destination.dest.y), static_cast<float>(destination.dest.z)};
    if (Vector3Distance(explosion_position, position) < Vector3Distance(destination_vector, position)) {
        destination = components::Destination{static_cast<int32_t>(explosion_position.x), static_cast<int32_t>(explosion_position.y), static_cast<int32_t>(explosion_position.z)};
    }
}

void Galaxy::_explode_stars(const ExplosionEvent &ev) {
    auto explosion_position = _core->registry.get<Vector3>(ev.e);
    _core->registry.remove<components::Exploding>(ev.e);
    _core->registry.emplace<components::Nova>(ev.e);
    auto nova_seekers = _core->registry.view<components::NovaSeeker>();
    nova_seekers.each([this, ev](const entt::entity entity, components::NovaSeeker &nova_seeker) {
        if (nova_seeker.capacity > 0) {
            _core->dispatcher.enqueue<NovaSeekEvent>(entity, ev.e);
            nova_seeker.capacity -= 1;
        }
    });

    auto fleets = _core->registry.view<components::Fleet, Vector3, components::Destination, components::Size>();
    fleets.each([=](const entt::entity entity, components::Fleet &fleet, Vector3 &position, components::Destination &destination, const components::Size size) {
        change_course_upon_nearer_explosion(explosion_position, entity, fleet, position, destination, size);
    });
}


void Galaxy::_send_fleet_to_nova(const NovaSeekEvent &ev) {
    FleetEntity fleet;
    fleet.react_to_nova(_core, _core->pcg, ev, _ship_components, stars_graph);

    add_vicinity(_core, fleet.get_entity(), ev.source);
}

void Galaxy::_fleet_arrived_at_star(const ArrivalEvent &ev) {
    add_vicinity(_core, ev.what, ev.where);
    add_vicinity(_core, ev.where, ev.what);
}

void Galaxy::_entity_left_vicinity(const LeaveEvent &ev) {
    components::Vicinity &vicinity_of_what = _core->registry.get<components::Vicinity>(ev.what);
    vicinity_of_what.objects.erase(std::remove(vicinity_of_what.objects.begin(), vicinity_of_what.objects.end(), ev.where), vicinity_of_what.objects.end());
}

void Galaxy::_update_vicinities() {
    _core->registry.view<components::Vicinity>().each([&](entt::entity entity, components::Vicinity &vicinity) {
        auto entity_position = _core->registry.get<Vector3>(entity);
        std::for_each(vicinity.objects.begin(), vicinity.objects.end(), [&](entt::entity object) {
            auto object_position = _core->registry.get<Vector3>(object);
            if (Vector3Distance(entity_position, object_position) > 1.0f) {
                _core->dispatcher.enqueue<LeaveEvent>(entity, object);
            }
        });
    });
}
std::vector<entt::entity> Galaxy::_get_nearest_stars(const entt::entity of_entity) {
    std::vector<entt::entity> nearest_stars;
    if (_core->registry.try_get<components::Fleet>(of_entity)) {
        auto fleet_range = _core->registry.get<components::Range>(of_entity);
        auto fleet_position = _core->registry.get<Vector3>(of_entity);
        _core->registry.view<components::Star, Vector3>().each([&](auto entity, auto star, auto position) {
            if (Vector3Distance(fleet_position, position) <= fleet_range.distance) {
                nearest_stars.emplace_back(entity);
            }
        });
    }
    return nearest_stars;
}
static constexpr auto get_calculated_distance = [](entt::registry &registry, std::vector<entt::entity> &entities) {
    float calculated_distance = 0;
    if (entities.size() > 1) {
        for (size_t i = 0; i < entities.size() - 1; i++) {
            calculated_distance += Vector3Distance(registry.get<Vector3>(entities[i]), registry.get<Vector3>(entities[i + 1]));
        }
    }
    return calculated_distance;
};
void Galaxy::_on_star_selected(const StarSelectedEvent &ev) {
    if (_core->registry.valid(ev.entity)) {
        if (_ui_wants_to_set_course && _core->registry.try_get<components::Fleet>(_selected_entity)) {
            _path.from = _selected_entity;
            _path.to = ev.entity;
            _ui_wants_to_set_course = false;

            _set_course_for_fleet(_path.from, _path.to);
        }
    }
}

void Galaxy::_register_path_selection(const std::vector<entt::entity> &calculated_path) {
    selected_paths.clear();
    for (size_t i = 0; i < calculated_path.size() - 1; i++) {
        Vector3 first = _core->registry.get<Vector3>(calculated_path[i]);
        Vector3 second = _core->registry.get<Vector3>(calculated_path[i + 1]);
        selected_paths.emplace_back(std::make_pair(first, second));
    }
}

void Galaxy::_clear_paths() {
    _path.from = entt::null;
    _path.to = entt::null;
    _path.checkpoints.clear();
    selected_paths.clear();
}

Camera Galaxy::_initialize_camera(const Vector3 &cameraInitialPosition, const float cameraDistance,
                                  const float horizontalDistance, const float horizontalAngle,
                                  const float verticalAngle) {
    Camera camera;
    camera.target = cameraInitialPosition;
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    camera.position.x = horizontalDistance * std::cosh(horizontalAngle * PI / 180.0f);
    camera.position.y = horizontalDistance * std::sinh(horizontalAngle * PI / 180.0f);
    camera.position.z = cameraDistance * sinf(verticalAngle * PI / 180.0f);
    SetCameraMode(camera, CAMERA_ORBITAL);
    UpdateCamera(&camera);
    return camera;
}

void Galaxy::_generate_player_entity() {
    std::vector<entt::entity> all_stars;
    _core->registry.view<components::Star>().each([&](auto entity, auto star) {
        all_stars.emplace_back(entity);
    });
    entt::entity random_star = all_stars[_core->pcg(all_stars.size())];

    entt::entity player_fleet = FleetEntity::create(_core, _core->pcg, _core->registry.get<Vector3>(random_star), _ship_components);
    _core->registry.emplace<components::PlayerControlled>(player_fleet);
    add_vicinity(_core, player_fleet, random_star);
}

void Galaxy::_set_course_for_fleet(const entt::entity from, const entt::entity to) {
    std::vector<entt::entity> nearest_stars = _get_nearest_stars(from);
    std::unordered_map<entt::entity, std::vector<entt::entity>> courses;
    entt::entity star_with_shortest_path = entt::null;
    float shortest_distance = 0.0f;
    std::for_each(nearest_stars.begin(), nearest_stars.end(), [&](entt::entity near_star) {
        courses[near_star] = calculate_path<Vector3, DistanceFunction, components::Star>(stars_graph, _core->registry, near_star, to);
        auto course_distance = get_calculated_distance(_core->registry, courses[near_star]);
        if (star_with_shortest_path == entt::null || (!courses[near_star].empty() && course_distance < shortest_distance)) {
            star_with_shortest_path = near_star;
            shortest_distance = course_distance;
        }
    });
    auto path = components::Path{courses[star_with_shortest_path]};
    auto current_fleet_path = _core->registry.try_get<components::Path>(_selected_entity);
    if (current_fleet_path) {
        current_fleet_path->checkpoints = courses[star_with_shortest_path];
    } else {
        _core->registry.emplace<components::Path>(_selected_entity, path);
    }
}

entt::entity StarEntity::create_at(entt::registry &registry, const std::shared_ptr<Core> &core, Vector3 position) {
    auto &pcg = core->pcg;
    if (pcg(_occurence_chance) == 0) {
        _entity = registry.create();
        registry.emplace<Vector3>(_entity, position);
        registry.emplace<components::Name>(_entity, components::Name{core->name_generator.get_random_name<components::Star>(pcg)});
        if (pcg(_exploding_chance.upper_bound) < _exploding_chance.occurs_if_less_then) {
            const auto explosion_counter = pcg(15) + 1;
            registry.emplace<components::Star>(_entity, components::Star{Colors::col_3.r, Colors::col_3.g, Colors::col_3.b, Colors::col_3.a / 2});

            registry.emplace<components::Exploding>(_entity, static_cast<uint8_t>(explosion_counter));
            registry.emplace<components::Size>(_entity, static_cast<float>(explosion_counter));
        } else {
            auto star_color = Colors::star_colors[pcg(Colors::star_colors.size())];
            registry.emplace<components::Star>(_entity, components::Star{star_color.r, star_color.g, star_color.b, star_color.a});
            registry.emplace<components::Size>(_entity, 1.0f);

            if (pcg(_nova_seeker_chance.upper_bound) < _nova_seeker_chance.occurs_if_less_then) {
                registry.emplace<components::NovaSeeker>(_entity, pcg(5) + 1);
            }
        }
    }

    return _entity;
}

bool StarEntity::is_created() {
    return _entity != entt::null;
}

void StarEntity::on_click(const entt::registry &registry, const entt::entity entity) {
    auto position = registry.get<Vector3>(entity);
    std::printf("Clicked star=[%.1f, %.1f, %.1f]\n", position.x, position.y, position.z);
}
