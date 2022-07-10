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
struct FleetCreationEvent {
    entt::entity e;
};
struct PathCreationEvent {
    entt::entity source;
    entt::entity destination;
};
struct ArrivalEvent {
    entt::entity what;
    entt::entity where;
};
struct LeaveEvent {
    entt::entity what;
    entt::entity where;
};
struct DestinationReachedEvent {
    entt::entity what;
    entt::entity where;
};
struct NovaSeekEvent {
    entt::entity source;
    entt::entity destination;
};
struct StarSelectedEvent {
    entt::entity entity;
    uint32_t seed;
};
struct StarScanEvent {
    entt::entity scanner;
    entt::entity star;
    uint32_t seed;
};
#endif//TLEILAX_EVENTS_H
