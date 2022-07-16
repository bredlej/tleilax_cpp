//
// Created by geoco on 13.03.2022.
//

#ifndef TLEILAX_PATH_H
#define TLEILAX_PATH_H

#include <entt/entt.hpp>
#include <vector>

struct Node {
    entt::entity entity;
    float cost;

    bool operator<(const Node &node) const {
        return cost < node.cost;
    }
};

struct Path {
public:
    explicit Path() noexcept : from{entt::null}, to{entt::null} {};
    std::vector<Node> checkpoints;
    entt::entity from;
    entt::entity to;
};

#endif//TLEILAX_PATH_H
