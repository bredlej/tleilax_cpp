//
// Created by geoco on 19.03.2022.
//

#ifndef TLEILAX_GRAPH_H
#define TLEILAX_GRAPH_H

#include <unordered_map>
#include <vector>
#include <entt/entt.hpp>
#include <queue>

template<typename NodeT, typename DistanceT, typename HashFuncT, typename EqualFuncT>
class Graph {
    using AdjacencyList = std::unordered_map<NodeT, std::vector<std::pair<NodeT, DistanceT>>, HashFuncT, EqualFuncT>;

public:
    void add_edge(NodeT source_node, NodeT target_node, DistanceT distance, bool is_bidirectional);
    [[nodiscard]] AdjacencyList get() const { return adjacency_list; } ;

private:
    AdjacencyList adjacency_list;
};

template<typename T, typename D, typename H, typename E>
void Graph<T, D, H, E>::add_edge(T source_node, T target_node, D distance, bool is_bidirectional) {
    adjacency_list[source_node].push_back(std::make_pair(target_node, distance));
    if (is_bidirectional) {
        adjacency_list[target_node].push_back(std::make_pair(source_node, distance));
    }
}

struct GraphNode {
    entt::entity entity;
    bool visited;
};

struct PathNode {
    entt::entity entity;
    float cost;

    bool operator<(const PathNode &node) const {
        return cost < node.cost;
    }
};

struct GraphNodeHash {
    std::size_t operator() (const GraphNode node) const {return static_cast<size_t>(node.entity);}
};

struct GraphNodeEqualFunc {
    bool operator() (const GraphNode first, const GraphNode second) const {return first.entity == second.entity;};
};

static std::vector<entt::entity> reconstruct_path(std::unordered_map<entt::entity, entt::entity>& came_from, const entt::entity current) {
    std::vector<entt::entity> total_path{current};
    entt::entity cur = current;
    while(came_from.contains(cur)) {
        cur = came_from[cur];
        total_path.insert(total_path.begin(), cur);
    }
    return total_path;
}

/**
 *
 * @tparam ComponentT
 * @tparam EntityMarker Used to group entities by specific component types, like e.g. all Star entities must have a StarColor component
 * @tparam HeuristicFunc
 * @param graph
 * @param registry
 * @param from
 * @param to
 * @return
 */
template<typename ComponentT, typename HeuristicFunc, typename ... EntityMarkerT>
static std::vector<entt::entity> calculate_path(const Graph<GraphNode, float, GraphNodeHash, GraphNodeEqualFunc> &graph, const entt::registry &registry, entt::entity from, entt::entity to) {
    auto h = HeuristicFunc();
    std::vector<entt::entity> calculated_path;
    ComponentT destination_component = registry.get<ComponentT>(to);
    std::priority_queue<PathNode> open_set;
    std::vector<entt::entity> open_set_v;
    open_set.push(PathNode{from, 0});
    open_set_v.push_back(from);
    std::unordered_map<entt::entity, entt::entity> came_from;
    std::unordered_map<entt::entity, float> g_score;

    registry.view<ComponentT, EntityMarkerT...>()
            .each([&](const entt::entity entity, const ComponentT &component, const EntityMarkerT... marker) {
                if (entity != from) {
                    g_score[entity] = 999999.9f;
                } else {
                    g_score[entity] = 0.0f;
                }
            });

    while (!open_set.empty()) {
        PathNode this_node = open_set.top();
        if (this_node.entity == to) {
            calculated_path = reconstruct_path(came_from, this_node.entity);
            break;
        }
        ComponentT current_component = registry.get<ComponentT>(this_node.entity);
        open_set.pop();
        open_set_v.erase(std::remove(open_set_v.begin(), open_set_v.end(), this_node.entity), open_set_v.end());
        const auto neighbours = graph.get()[GraphNode{this_node.entity, false}];
        for (const auto neighbour : neighbours) {
            const auto neighbour_entity = neighbour.first.entity;
            ComponentT neighbour_component = registry.get<ComponentT>(neighbour_entity);
            auto tentative_score = g_score[this_node.entity] + h(current_component, neighbour_component);
            if (tentative_score < g_score[neighbour_entity]) {
                came_from[neighbour_entity] = this_node.entity;
                g_score[neighbour_entity] = tentative_score;
                if (!(std::find(open_set_v.begin(), open_set_v.end(), neighbour_entity) != open_set_v.end())) {
                    open_set.push(PathNode{neighbour_entity, tentative_score + h(neighbour_component, destination_component)});
                    open_set_v.push_back(neighbour_entity);
                }
            }
        }
    }
    return calculated_path;
}
#endif//TLEILAX_GRAPH_H
