//
// Created by geoco on 18.12.2021.
//
#include <tleilax.h>
#include <gtest/gtest.h>

class TleilaxTest : public testing::Test {
protected:
    TleilaxTest() = default;
};

TEST_F(TleilaxTest, RunsTest) {
    EXPECT_TRUE(true);
}

int main(int ac, char *av[]) {
    testing::InitGoogleTest(&ac, av);
    return RUN_ALL_TESTS();
}