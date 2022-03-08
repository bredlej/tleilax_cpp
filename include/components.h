//
// Created by geoco on 19.12.2021.
//

#ifndef TLEILAX_COMPONENTS_H
#define TLEILAX_COMPONENTS_H
#include <assets.h>
#include <cstdint>
#include <entt/entt.hpp>
#include <functional>
#include <tuple>
#include <vector>

namespace components {
    struct dice_roll {
        uint8_t amount;
        uint8_t sides;
    };

    struct Vector3;

    struct Size {
        float size;
    };
    struct StarColor {
        uint8_t r, g, b, a;
    };
    struct Exploding {
        uint8_t counter;
    };
    struct Nova {};
    struct NovaSeeker {
        uint32_t capacity;
    };

    struct Fleet {
        std::vector<entt::entity> ships;
    };

    struct Coordinates {
        int32_t x, y, z;
    };

    struct Destination {
        Coordinates dest;
    };

    // Ship Components

    struct Engine {
        std::string id;
        std::string name;
        uint32_t power;
        uint32_t weight;
    };

    struct Hull {
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

template<typename ComponentT, ship_component::types AssetT>
static std::vector<ComponentT> asset_parser(const Assets &assets, const std::function<ComponentT(const nlohmann::json &)> converter_func) {
    std::vector<ComponentT> components;
    const auto &ship_components = assets.get_ship_components();
    for (const auto &json : ship_components[ship_component::names[AssetT]]) {
        components.emplace_back(converter_func(json));
    }
    return components;
};

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

template<typename T, ship_component::types A, T (*json_parse_func)(const nlohmann::json &)>
struct ConverterT {
    using Type = T;

public:
    std::function<T(const nlohmann::json &)> converter_func = json_parse_func;
    const static ship_component::types Asset = A;
};

static ConverterT<components::Weapon, ship_component::types::weapon, weapon_from_json> WeaponRepository{};
static ConverterT<components::Shield, ship_component::types::shield, shield_from_json> ShieldRepository{};
static ConverterT<components::Engine, ship_component::types::engine, engine_from_json> EngineRepository{};

template<typename T>
constexpr auto instance_of_T = []() { T t; return t; };

template<typename T, typename... Ts>
struct ComponentRepository {
    explicit ComponentRepository() {
        components = std::make_tuple(std::vector<typename T::Type>(), std::vector<typename Ts::Type>()...);
    }
    explicit ComponentRepository(const Assets &assets) {
        components = std::make_tuple(
                asset_parser<typename T::Type, T::Asset>(assets, instance_of_T<T>().converter_func),
                asset_parser<typename Ts::Type, Ts::Asset>(assets, instance_of_T<Ts>().converter_func)...);
    }
    std::tuple<std::vector<typename T::Type>, std::vector<typename Ts::Type>...> components;
};

#endif//TLEILAX_COMPONENTS_H
