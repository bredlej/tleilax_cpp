//
// Created by geoco on 13.03.2022.
//

#ifndef TLEILAX_PATH_H
#define TLEILAX_PATH_H

#include <entt/entt.hpp>
#include <functional>
#include <memory>
#include <queue>

struct Node {
    entt::entity entity;
    float cost;

    bool operator<(const Node &node) const {
        return cost < node.cost;
    }
};

struct Path {
public:
    explicit Path() : from{entt::null}, to{entt::null} {};
    std::vector<Node> checkpoints;
    entt::entity from;
    entt::entity to;
    std::priority_queue<Node> calculate(const entt::registry &, const std::function<std::priority_queue<Node>(const entt::registry &, const entt::entity)> &) const;
};

#endif//TLEILAX_PATH_H
