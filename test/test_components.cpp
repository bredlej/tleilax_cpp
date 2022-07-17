//
// Created by geoco on 08.03.2022.
//
#include <cstdio>
#include <gtest/gtest.h>
#include <tleilax.hpp>

class TleilaxComponentTest : public testing::Test {
protected:
    TleilaxComponentTest() = default;
};

TEST_F(TleilaxComponentTest, RunsTest) {
    EXPECT_TRUE(true);
}

TEST_F(TleilaxComponentTest, ConvertsStringToDiceRoll) {
    auto dice_roll = str_to_dice_roll("10d100");
    auto expected = components::dice_roll{10, 100};
    EXPECT_EQ(expected.amount, dice_roll.amount);
    EXPECT_EQ(expected.sides, dice_roll.sides);
}

int main(int ac, char *av[]) {
    testing::InitGoogleTest(&ac, av);
    return RUN_ALL_TESTS();
}