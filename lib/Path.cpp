//
// Created by geoco on 16.03.2022.
//

#include <path.h>

std::priority_queue<Node> Path::calculate(const entt::registry &registry, const std::function<std::priority_queue<Node>(const entt::registry &, const entt::entity)> &neighbour_func) const {
    std::priority_queue<Node> neighbour_nodes = neighbour_func(registry, from);
    return neighbour_nodes;
}