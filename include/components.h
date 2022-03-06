//
// Created by geoco on 19.12.2021.
//

#ifndef TLEILAX_COMPONENTS_H
#define TLEILAX_COMPONENTS_H
#include <assets.h>
#include <cstdint>
#include <entt/entt.hpp>
#include <vector>
#include <functional>
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
        float speed;
        float max_speed;
    };

    struct Hull {
        float health;
        float max_health;
    };

    struct Shield {
        float absorption;
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
static std::vector<ComponentT> asset_parser(const Assets &assets, std::function<ComponentT(const nlohmann::json &)> converter_func) {
    std::vector<ComponentT> components;
    const auto &ship_components = assets.get_ship_components();
    for (const auto &json: ship_components[ship_component::names[AssetT]]) {
        components.emplace_back(converter_func(json));
    }
    return components;
};

static components::Weapon weapon_from_json(const nlohmann::json &json) {
    return components::Weapon{json["id"], json["name"], json["power_usage"], {1,4}, json["weight"]};
}

struct ShipComponents {
public:
    explicit ShipComponents(Assets &assets) : weapons{asset_parser<components::Weapon, ship_component::types::weapon>(assets, weapon_from_json)} {};

    std::vector<components::Weapon> weapons;

};

#endif//TLEILAX_COMPONENTS_H
