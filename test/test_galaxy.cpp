//
// Created by Patryk Szczypień on 16/07/2022.
//
#include <cstdio>
#include <gtest/gtest.h>
#include <memory>
#include <tleilax.hpp>

class GalaxyTest : public testing::Test {
protected:
    GalaxyTest() = default;
};

int main(int ac, char *av[]) {
    testing::InitGoogleTest(&ac, av);
    return RUN_ALL_TESTS();
}