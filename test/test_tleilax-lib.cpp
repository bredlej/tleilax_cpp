//
// Created by geoco on 18.12.2021.
//
#include <cstdio>
#include <gtest/gtest.h>
#include <tleilax.hpp>

class TleilaxTest : public testing::Test {
protected:
    TleilaxTest() = default;
};

TEST_F(TleilaxTest, RunsTest) {
    EXPECT_TRUE(true);
}

TEST_F(TleilaxTest, DispatcherTest) {
    
    entt::dispatcher dispatcher{};
    
    struct Emission {
        int i;
    };
    
    struct listener {
        void receive(const Emission &emission) {
            ASSERT_EQ(1, emission.i);
        };
    };
    
    listener listener;
    dispatcher.sink<Emission>().connect<&listener::receive>(listener);

    dispatcher.enqueue<Emission>(1);
    dispatcher.update();
}

int main(int ac, char *av[]) {
    testing::InitGoogleTest(&ac, av);
    return RUN_ALL_TESTS();
}