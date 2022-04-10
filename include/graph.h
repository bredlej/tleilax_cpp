//
// Created by geoco on 19.03.2022.
//

#ifndef TLEILAX_GRAPH_H
#define TLEILAX_GRAPH_H

#include <unordered_map>
#include <vector>

template<typename NodeT, typename DistanceT, typename HashFuncT, typename EqualFuncT>
class Graph {
    using AdjacencyList = std::unordered_map<NodeT, std::vector<std::pair<NodeT, DistanceT>>, HashFuncT, EqualFuncT>;

public:
    void add_edge(NodeT source_node, NodeT target_node, DistanceT distance, bool is_bidirectional);
    AdjacencyList get() const { return adjacency_list; } ;

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

template<typename NodeT>
struct SearchNode {
    NodeT *previous;
    NodeT current;
};

template<typename NodeT, typename CostFunc, typename HeuristicFunc>
struct AStarPath {
    std::vector<SearchNode<NodeT>> get_between(const auto &Graph, const NodeT&, const NodeT&);
};
#endif//TLEILAX_GRAPH_H
