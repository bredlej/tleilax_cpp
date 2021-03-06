//
// Created by geoco on 19.12.2021.
//

#include "galaxy.hpp"
#include <name_generator.hpp>

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
                                if (distance < _distance_between_stars) {
                                    stars_graph.add_edge(starNode, next, distance, false);
                                    stars_paths.emplace_back(std::make_pair(coords, _coords));
                                }
                            }
                        });
            });
}

void Galaxy::_initialize() {
    _init_star_render_instance();
    _path = Path{};
    _core->dispatcher.sink<ExplosionEvent>().connect<&Galaxy::_explode_stars>(this);
    _core->dispatcher.sink<NovaSeekEvent>().connect<&Galaxy::_send_fleet_to_nova>(this);
    _core->dispatcher.sink<ArrivalEvent>().connect<&Galaxy::_fleet_arrived_at_star>(this);
    _core->dispatcher.sink<LeaveEvent>().connect<&Galaxy::_entity_left_vicinity>(this);
    _core->dispatcher.sink<StarSelectedEvent>().connect<&Galaxy::_on_star_selected>(this);
    _core->dispatcher.sink<StarScanEvent>().connect<&Galaxy::_on_star_scanned>(this);
}

void Galaxy::populate() {
    auto before = std::chrono::high_resolution_clock::now();
    _reset_all();
    _generate_objects();
    auto after = std::chrono::high_resolution_clock::now() - before;
    std::printf("Elapsed time: %lld ms\n", std::chrono::duration_cast<std::chrono::milliseconds>(after).count());
}
void Galaxy::_generate_objects() {
    _generate_stars();
    _recalculate_graph();
    _generate_player_entity();
    _generate_fleets();

    _core->registry.view<components::Star, Vector3>()
            .each([&](entt::entity entity, components::Star star, Vector3 position) {
                Color c{star.r, star.g, star.b, star.a};
                _place_star_instance_at(entity, position.x, position.y, position.z, c, _visible_size);
            });
}
void Galaxy::_reset_all() {
    _core->registry.clear();
    selected_paths.clear();
    _selected_fleet = entt::null;
    _core->game_log.clear();
    _star_systems.clear();
    _star_render_instance.matrices.clear();
    _star_render_instance.colors.clear();
    _star_render_instance.entities.clear();
    _star_render_instance.count = 0;
}

void Galaxy::_generate_stars() {
    _core->debug_log.message("Generating stars\n");
    for (int32_t z = 0; z < static_cast<int32_t>(_visible_size.z); z++) {
        for (int32_t y = 0; y < static_cast<int32_t>(_visible_size.y); y++) {
            for (int32_t x = 0; x < static_cast<int32_t>(_visible_size.x); x++) {
                StarEntity star(_star_occurence_chance, {100, 5}, {100, 10});
                _next_random_number(seed_function(x, y, z));
                star.create_at(_core->registry, _core, Vector3{static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});
            }
        }
    }
}

void Galaxy::_generate_fleets() {
    std::vector<entt::entity> stars;
    auto all_stars_view = _core->registry.view<components::Star, components::Name>();
    for (auto [entity, star, name] : all_stars_view.each()) {
        stars.emplace_back(entity);
    }
    auto spawn_star = stars[_core->pcg(stars.size())];
    auto spawn_star_name = _core->registry.get<components::Name>(spawn_star).name.c_str();
    entt::entity fleet_entity = FleetEntity::create(_core, _core->pcg, _core->registry.get<Vector3>(spawn_star), _ship_components);
    _core->game_log.message("A tleilaxian fleet has arrived at %s!\n", spawn_star_name);
    _core->registry.emplace<components::Tleilaxian>(fleet_entity);
    for (auto [entity, infectable, name] : _core->registry.view<components::Infectable, components::Name>().each()) {
        auto path = calculate_path<Vector3, DistanceFunction, components::Star>(stars_graph, _core->registry, spawn_star, entity);
        if (!path.empty()) {
            _core->game_log.message("The tleilaxian fleet is heading towards %s!\n", name.name.c_str());
            _core->registry.emplace<components::Path>(fleet_entity, path);
            break;
        }
    }
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
        _open_demo = !_open_demo;
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

    auto &entities = _star_render_instance.entities;
    for (size_t i = 0; i < entities.size(); i++) {
        if (_core->registry.valid(entities[i]) && _core->registry.try_get<components::Exploding>(entities[i])) {
            auto size = _core->registry.get<components::Size>(entities[i]);
            auto position = _core->registry.get<Vector3>(entities[i]);
            auto &matrix = _star_render_instance.matrices[i];

            Matrix m = MatrixIdentity();
            Vector3 v = local_to_global_coords(position, _visible_size);
            auto translation = MatrixTranslate(v.x, v.y, v.z);
            auto rotation = MatrixRotate({1, 0, 0}, 0);
            auto scaling = MatrixScale(size.size, size.size, size.size);

            m = MatrixMultiply(m, scaling);
            m = MatrixMultiply(m, translation);
            m = MatrixMultiply(m, rotation);

            matrix = m;
        }
    }

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
    _core->game_log.message("A scavenger fleet from %s is heading towards the remnants of %s.\n", _core->registry.get<components::Name>(ev.source).name.c_str(), _core->registry.get<components::Name>(ev.destination).name.c_str());
    add_vicinity(_core, fleet.get_entity(), ev.source);
}

void Galaxy::_fleet_arrived_at_star(const ArrivalEvent &ev) {
    add_vicinity(_core, ev.what, ev.where);
    add_vicinity(_core, ev.where, ev.what);
    auto is_tleilaxian = _core->registry.try_get<components::Tleilaxian>(ev.what);
    auto is_infectable = _core->registry.try_get<components::Infectable>(ev.where);
    if (is_tleilaxian && is_infectable) {
        auto counter = static_cast<uint8_t>(_core->pcg(40) + 1);
        _core->registry.emplace<components::Exploding>(ev.where, counter);
        auto &size = _core->registry.get<components::Size>(ev.where);
        size.size = counter;
        auto &star = _core->registry.get<components::Star>(ev.where);
        star.r = Colors::col_3.r;
        star.g = Colors::col_3.g;
        star.b = Colors::col_3.b;
        star.a = Colors::col_3.a / 2;
        _core->game_log.message("The tleilaxian fleet fired a bomb into %s! The star will explode in %d days!\n", _core->registry.get<components::Name>(ev.where).name.c_str(), counter);
    }
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
    _core->debug_log.message("OnStarSelected\n");
    if (!_ui_wants_to_set_course) {
        _clicked_star = ev.entity;
    } else {
        if (_core->registry.valid(ev.entity) && _core->registry.valid(_star_mouse_over)) {
            auto fleet = _core->registry.try_get<components::Fleet>(_selected_fleet);
            if (fleet) {
                _path.from = _selected_fleet;
                _path.to = _star_mouse_over;
                _ui_wants_to_set_course = false;

                _set_course_for_fleet(_path.from, _path.to);
            }
        }
    }
}

void Galaxy::_on_star_scanned(const StarScanEvent &ev) {
    if (_core->registry.valid(ev.scanner) && _core->registry.valid(ev.star)) {
        auto &known_systems = _core->registry.get<components::KnownStarSystems>(ev.scanner);
        known_systems.seeds.emplace_back(_core->get_seed_of(ev.star));
        _add_star_system(ev.star);
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
    auto star_name = _core->registry.get<components::Name>(random_star);
    _core->game_log.message("You arrived at %s\n", star_name.name.c_str());
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
    auto current_fleet_path = _core->registry.try_get<components::Path>(_selected_fleet);
    if (current_fleet_path) {
        current_fleet_path->checkpoints = courses[star_with_shortest_path];
    } else {
        _core->registry.emplace<components::Path>(_selected_fleet, path);
    }
}

entt::entity StarEntity::create_at(entt::registry &registry, const std::shared_ptr<Core> &core, Vector3 position) {
    auto &pcg = core->pcg;
    //static auto star_is_infectable = [&pcg](entt::entity) { return pcg(100) <= 10; };
    if (pcg(_occurence_chance) == 0) {
        _entity = registry.create();
        registry.emplace<Vector3>(_entity, position);
        components::Name star_name{core->name_generator.get_random_name<components::Star>(pcg)};
        registry.emplace<components::Name>(_entity, star_name);
        core->debug_log.message("Generating star [%s] at (%.0f, %.0f, %.0f)\n", star_name.name.c_str(), position.x, position.y, position.z);

        const auto star_classification = pcg(10000);
        if (star_classification <= 7600) {
            // M
            constexpr auto star_color = Colors::star_colors[0];
            registry.emplace<components::Star>(_entity, components::Star{star_color.r, star_color.g, star_color.b, star_color.a, components::StarClassification::M});
            registry.emplace<components::Size>(_entity, 0.45f);
        } else if (star_classification > 7600 && star_classification <= 8800) {
            // K
            constexpr auto star_color = Colors::star_colors[1];
            registry.emplace<components::Star>(_entity, components::Star{star_color.r, star_color.g, star_color.b, star_color.a, components::StarClassification::K});
            registry.emplace<components::Size>(_entity, 0.8f);
        } else if (star_classification > 8800 && star_classification <= 9550) {
            // G
            constexpr auto star_color = Colors::star_colors[2];
            registry.emplace<components::Star>(_entity, components::Star{star_color.r, star_color.g, star_color.b, star_color.a, components::StarClassification::G});
            registry.emplace<components::Infectable>(_entity);
            registry.emplace<components::Size>(_entity, 1.04f);
            core->debug_log.message("  - Marking this star as infectable\n");
        } else if (star_classification > 9550 && star_classification <= 9850) {
            // F
            constexpr auto star_color = Colors::star_colors[3];
            registry.emplace<components::Star>(_entity, components::Star{star_color.r, star_color.g, star_color.b, star_color.a, components::StarClassification::F});
            registry.emplace<components::Infectable>(_entity);
            registry.emplace<components::Size>(_entity, 1.4f);
            core->debug_log.message("  - Marking this star as infectable\n");
        } else if (star_classification > 9850 && star_classification <= 9950) {
            // A
            constexpr auto star_color = Colors::star_colors[4];
            registry.emplace<components::Star>(_entity, components::Star{star_color.r, star_color.g, star_color.b, star_color.a, components::StarClassification::A});
            registry.emplace<components::Infectable>(_entity);
            registry.emplace<components::Size>(_entity, 2.1f);
            core->debug_log.message("  - Marking this star as infectable\n");
        } else if (star_classification > 9950 && star_classification <= 9980) {
            constexpr auto star_color = Colors::star_colors[5];
            registry.emplace<components::Star>(_entity, components::Star{star_color.r, star_color.g, star_color.b, star_color.a, components::StarClassification::B});
            // B
            registry.emplace<components::Infectable>(_entity);
            registry.emplace<components::Size>(_entity, 10.0f);
            core->debug_log.message("  - Marking this star as infectable\n");
        } else {
            constexpr auto star_color = Colors::star_colors[6];
            registry.emplace<components::Star>(_entity, components::Star{star_color.r, star_color.g, star_color.b, star_color.a, components::StarClassification::O});
            registry.emplace<components::Size>(_entity, 16.0f);
            // O
        }

        if (pcg(_nova_seeker_chance.upper_bound) < _nova_seeker_chance.occurs_if_less_then) {
            registry.emplace<components::NovaSeeker>(_entity, pcg(5) + 1);
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

void Galaxy::_add_star_system(const entt::entity entity) {
    const auto seed = _core->get_seed_of(entity);
    if (!_star_systems.contains(seed)) {
        _star_systems[seed] = std::make_unique<StarSystem>(entity, _core);
    }
}
