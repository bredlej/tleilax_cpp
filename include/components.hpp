//
// Created by geoco on 19.12.2021.
//

#ifndef TLEILAX_COMPONENTS_HPP
#define TLEILAX_COMPONENTS_HPP
#include <assets.hpp>
#include <cstdint>
#include <entt/entt.hpp>
#include <functional>
#include <string>
#include <tuple>
#include <vector>

namespace components {
    struct dice_roll {
        uint8_t amount;
        uint8_t sides;
    };

    struct Name {
        std::string name;
    };

    struct Vector3;

    struct Range {
        float distance;
    };
    struct Size {
        float size;
    };
    enum class StarClassification : uint8_t {
        M, K, G, F, A, B, O, Neutron, Black_Hole
    };
    struct Star {
        uint8_t r, g, b, a;
        StarClassification classification;
    };
    struct Exploding {
        uint8_t counter;
    };
    struct Nova {};
    struct Infectable { int i; };
    struct NovaSeeker {
        uint32_t capacity;
    };

    struct Fleet {
        std::vector<entt::entity> ships;
    };
    struct Tleilaxian { int i; };
    struct Coordinates {
        int32_t x, y, z;
    };

    struct Destination {
        Coordinates dest;
    };

    struct Path {
        std::vector<entt::entity> checkpoints;
    };

    struct PlayerControlled { int i; };

    struct Vicinity {
        std::vector<entt::entity> objects;
    };

    struct KnownStarSystems {
        std::vector<uint32_t> seeds;
    };

    struct GravityCenter {
        int index;
        float mass;
    };

    struct Orbit {
        entt::entity attached_to;
        float size;
        float degrees;
    };

    struct Planet {
        int i;
    };

    struct Moon {
        int i;
    };

    struct SpaceStation {
        int i;
    };

    struct AsteroidBelt {
        int i;
    };

    struct Anomaly {
        int i;
    };
    // Ship Components

    struct Engine {
        std::string id;
        std::string name;
        uint32_t power;
        uint32_t weight;
    };

    struct Hull {
        std::string id;
        std::string name;
        float health;
        float max_health;
    };

    struct Shield {
        std::string id;
        std::string name;
        uint32_t power_usage;
        uint32_t defense;
        uint32_t weight;
    };

    struct Weapon {
        std::string id;
        std::string name;
        uint32_t power_usage;
        dice_roll damage;
        uint32_t weight;
    };
}// namespace components

constexpr auto str_to_dice_roll = [](const std::string &s) {
    std::string delimiter = "d";
    return components::dice_roll{
            static_cast<uint8_t>(std::stoi(s.substr(0, s.find(delimiter)))),
            static_cast<uint8_t>(std::stoi(s.substr(s.find(delimiter) + 1, s.length())))};
};

static components::Weapon weapon_from_json(const nlohmann::json &json) {
    return components::Weapon{json["id"], json["name"], json["power_usage"], str_to_dice_roll(json["damage"]), json["weight"]};
}

static components::Shield shield_from_json(const nlohmann::json &json) {
    return components::Shield{json["id"], json["name"], json["power_usage"], json["defense"], json["weight"]};
}

static components::Engine engine_from_json(const nlohmann::json &json) {
    return components::Engine{json["id"], json["name"], json["power"], json["weight"]};
}

static components::Hull hull_from_json(const nlohmann::json &json) {
    return components::Hull{json["id"], json["name"], json["health"], json["max_health"]};
}

/*
 * Returns a vector of Components parsed from given json data with specified json->object converter function
 */
template<typename ComponentT, assets::types AssetT>
static std::vector<ComponentT> json_to_component_converter(const nlohmann::json &json_data, const std::function<ComponentT(const nlohmann::json &)>& converter_func) {
    std::vector<ComponentT> components;
    const auto &json_components = json_data;
    for (const auto &json : json_components[assets::names[AssetT]]) {
        components.emplace_back(converter_func(json));
    }
    return components;
};

/*
 * Helper struct holding info how to convert from json to an internal component type
 */
template<typename T, assets::types A, T (*json_parse_func)(const nlohmann::json &)>
struct RepositoryT {
    using Type = T;
    std::function<T(const nlohmann::json &)> converter_func = json_parse_func;
    const static assets::types Asset = A;
};

static RepositoryT<components::Weapon, assets::types::weapon, weapon_from_json> WeaponRepository{};
static RepositoryT<components::Shield, assets::types::shield, shield_from_json> ShieldRepository{};
static RepositoryT<components::Engine, assets::types::engine, engine_from_json> EngineRepository{};
static RepositoryT<components::Hull, assets::types::hull, hull_from_json> HullRepository{};

template<typename T>
constexpr auto instance_of_T = []() { T t; return t; };

template<typename T, typename... Ts>
class ComponentRepository {
public:
    explicit ComponentRepository() noexcept {
        components = std::make_tuple(std::vector<typename T::Type>(), std::vector<typename Ts::Type>()...);
    }
    explicit ComponentRepository(const nlohmann::json &json_data) noexcept {
        components = std::make_tuple(
                json_to_component_converter<typename T::Type, T::Asset>(json_data, instance_of_T<T>().converter_func),
                json_to_component_converter<typename Ts::Type, Ts::Asset>(json_data, instance_of_T<Ts>().converter_func)...);
    }
    ComponentRepository(const ComponentRepository&) noexcept = delete;
    ComponentRepository(ComponentRepository&&) noexcept = delete;
    ComponentRepository& operator=(const ComponentRepository&) noexcept = delete;
    ComponentRepository& operator=(ComponentRepository&&) noexcept = delete;
    ~ComponentRepository() noexcept = default;

    std::tuple<std::vector<typename T::Type>, std::vector<typename Ts::Type>...> components;
};

template<typename T>
std::vector<T> get(const auto &component_repository) {
    return std::get<std::vector<T>>(component_repository.components);
}

using ShipComponentRepository = ComponentRepository<
        decltype(WeaponRepository),
        decltype(ShieldRepository),
        decltype(EngineRepository),
        decltype(HullRepository)>;


#endif//TLEILAX_COMPONENTS_HPP
