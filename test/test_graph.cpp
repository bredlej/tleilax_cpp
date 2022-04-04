//
// Created by geoco on 19.03.2022.
//
#include <gtest/gtest.h>
#include <cstdio>
#include <graph.h>

struct Node {
    explicit Node(int number) : number(number) {};
    int number;
    bool operator==(const Node other) const {
        return number == other.number;
    }
};
struct NodeHash {
    std::size_t operator() (const Node node) const {return node.number;};
};

struct NodeEqual {
    bool operator() (const Node first, const Node second) const {return first.number == second.number ;};
};

class TleilaxGraphTest : public testing::Test {
protected:
    TleilaxGraphTest() = default;
};

TEST_F(TleilaxGraphTest, RunsTest) {
    EXPECT_TRUE(true);
}

TEST_F(TleilaxGraphTest, CreatesGraphWithTwoNodesNonDirectional) {
    // 0 -> 1
    Graph<Node, int, NodeHash, NodeEqual> graph;
    graph.add_edge(Node{0}, Node {1}, 1, false);
    auto adjacency_list = graph.get();

    EXPECT_EQ(1, adjacency_list.size());
    EXPECT_EQ(1, adjacency_list[Node{0}].size());
    EXPECT_EQ(1, adjacency_list[Node{0}][0].first.number);
    EXPECT_EQ(1, adjacency_list[Node{0}][0].second);
}

TEST_F(TleilaxGraphTest, CreatesGraphWithTwoNodesBiDirectional) {
    // 0 <-> 1
    Graph<Node, int, NodeHash, NodeEqual> graph;
    graph.add_edge(Node{0}, Node {1}, 1, true);
    auto adjacency_list = graph.get();

    EXPECT_EQ(2, adjacency_list.size());
    EXPECT_EQ(1, adjacency_list[Node{0}].size());
    EXPECT_EQ(1, adjacency_list[Node{0}][0].first.number);
    EXPECT_EQ(1, adjacency_list[Node{0}][0].second);
    EXPECT_EQ(1, adjacency_list[Node{1}].size());
    EXPECT_EQ(0, adjacency_list[Node{1}][0].first.number);
    EXPECT_EQ(1, adjacency_list[Node{1}][0].second);
}

TEST_F(TleilaxGraphTest, CreatesGraphWithThreeNodesNonDirectional) {
    // 0 -> 1 -> 2
    Graph<Node, int, NodeHash, NodeEqual> graph;
    graph.add_edge(Node{0}, Node {1}, 1, false);
    graph.add_edge(Node{1}, Node {2}, 1, false);
    auto adjacency_list = graph.get();

    EXPECT_EQ(2, adjacency_list.size());
    EXPECT_EQ(1, adjacency_list[Node{0}].size());
    EXPECT_EQ(1, adjacency_list[Node{0}][0].first.number);
    EXPECT_EQ(1, adjacency_list[Node{0}][0].second);
    EXPECT_EQ(1, adjacency_list[Node{1}].size());
    EXPECT_EQ(2, adjacency_list[Node{1}][0].first.number);
    EXPECT_EQ(1, adjacency_list[Node{1}][0].second);
}

TEST_F(TleilaxGraphTest, CreatesGraphWithThreeNodesBiDirectional) {
    // 0 <-> 1 <-> 2
    Graph<Node, int, NodeHash, NodeEqual> graph;
    graph.add_edge(Node{0}, Node {1}, 1, true);
    graph.add_edge(Node{1}, Node {2}, 1, true);
    auto adjacency_list = graph.get();

    EXPECT_EQ(3, adjacency_list.size());
    EXPECT_EQ(1, adjacency_list[Node{0}].size());
    EXPECT_EQ(1, adjacency_list[Node{0}][0].first.number);
    EXPECT_EQ(1, adjacency_list[Node{0}][0].second);
    EXPECT_EQ(2, adjacency_list[Node{1}].size());
    EXPECT_EQ(0, adjacency_list[Node{1}][0].first.number);
    EXPECT_EQ(1, adjacency_list[Node{1}][0].second);
    EXPECT_EQ(2, adjacency_list[Node{1}][1].first.number);
    EXPECT_EQ(1, adjacency_list[Node{1}][1].second);
    EXPECT_EQ(1, adjacency_list[Node{2}].size());
    EXPECT_EQ(1, adjacency_list[Node{2}][0].first.number);
    EXPECT_EQ(1, adjacency_list[Node{2}][0].second);
}

TEST_F(TleilaxGraphTest, CreatesGraphWithThreeBranchingNodesNonDirectional) {
    // 0 -> 1
    // 0 -> 2
    Graph<Node, int, NodeHash, NodeEqual> graph;
    graph.add_edge(Node{0}, Node {1}, 1, false);
    graph.add_edge(Node{0}, Node {2}, 1, false);
    auto adjacency_list = graph.get();

    EXPECT_EQ(1, adjacency_list.size());
    EXPECT_EQ(2, adjacency_list[Node{0}].size());
    EXPECT_EQ(1, adjacency_list[Node{0}][0].first.number);
    EXPECT_EQ(1, adjacency_list[Node{0}][0].second);
    EXPECT_EQ(2, adjacency_list[Node{0}][1].first.number);
    EXPECT_EQ(1, adjacency_list[Node{0}][1].second);
}

TEST_F(TleilaxGraphTest, CreatesGraphWithThreeBranchingNodesBiDirectional) {
    // 0 <-> 1
    // 0 <-> 2
    Graph<Node, int, NodeHash, NodeEqual> graph;
    graph.add_edge(Node{0}, Node {1}, 1, true);
    graph.add_edge(Node{0}, Node {2}, 1, true);
    auto adjacency_list = graph.get();

    EXPECT_EQ(3, adjacency_list.size());
    EXPECT_EQ(2, adjacency_list[Node{0}].size());
    EXPECT_EQ(1, adjacency_list[Node{0}][0].first.number);
    EXPECT_EQ(1, adjacency_list[Node{0}][0].second);
    EXPECT_EQ(2, adjacency_list[Node{0}][1].first.number);
    EXPECT_EQ(1, adjacency_list[Node{0}][1].second);
    EXPECT_EQ(1, adjacency_list[Node{1}].size());
    EXPECT_EQ(0, adjacency_list[Node{1}][0].first.number);
    EXPECT_EQ(1, adjacency_list[Node{1}][0].second);
    EXPECT_EQ(1, adjacency_list[Node{2}].size());
    EXPECT_EQ(0, adjacency_list[Node{2}][0].first.number);
    EXPECT_EQ(1, adjacency_list[Node{2}][0].second);

}

int main(int ac, char *av[]) {
    testing::InitGoogleTest(&ac, av);
    return RUN_ALL_TESTS();
}