//
// Created by geoco on 03.03.2022.
//

#ifndef TLEILAX_EVENTS_H
#define TLEILAX_EVENTS_H
#include <entt/entt.hpp>
#include <raylib.h>

struct ExplosionEvent {
    entt::entity e;
};
struct NovaSeekEvent {
    entt::entity e;
    Vector3 destination;
};
#endif//TLEILAX_EVENTS_H
