//
// Created by geoco on 03.03.2022.
//

#ifndef TLEILAX_EVENTS_HPP
#define TLEILAX_EVENTS_HPP
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
};
struct FleetsMeetEvent {
    entt::entity fleet1;
    entt::entity fleet2;
};
struct FleetsSeparateEvent {
    entt::entity fleet1;
    entt::entity fleet2;
};
struct PlayerBattleStartEvent {
    entt::entity player;
    entt::entity opponent;
};
struct PlayerBattleEndEvent{};
#endif//TLEILAX_EVENTS_HPP
