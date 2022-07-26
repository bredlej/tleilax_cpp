//
// Created by Patryk Szczypień on 11/07/2022.
//
#include <core.hpp>

uint32_t Core::get_seed_of(const entt::entity entity) const {
    if (const Vector3 *position = registry.try_get<Vector3>(entity)) {
        return seed_function(static_cast<uint32_t>(position->x), static_cast<uint32_t>(position->y), static_cast<uint32_t>(position->z));
    }
    else {
        return 0;
    }
}