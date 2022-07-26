//
// Created by geoco on 19.03.2022.
//
#include <cstdio>
#include <gtest/gtest.h>
#include <tleilax.hpp>


class TleilaxPathTest : public testing::Test {
protected:
    TleilaxPathTest() = default;
};

TEST_F(TleilaxPathTest, RunsTest) {
    EXPECT_TRUE(true);
}

TEST_F(TleilaxPathTest, DeterminesNeighbours) {
}

int main(int ac, char *av[]) {
    testing::InitGoogleTest(&ac, av);
    return RUN_ALL_TESTS();
}