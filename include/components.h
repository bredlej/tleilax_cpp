//
// Created by geoco on 19.12.2021.
//

#ifndef TLEILAX_COMPONENTS_H
#define TLEILAX_COMPONENTS_H
#include <cstdint>
#include <vector>
#include <entt/entt.hpp>

namespace components {
    struct Vector3;

    struct Size {
        float size;
    };
    struct StarColor{
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
        float damage;
    };
}

#endif//TLEILAX_COMPONENTS_H
