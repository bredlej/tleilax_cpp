//
// Created by geoco on 20.02.2022.
//
#include <tleilax.h>
#include <gtest/gtest.h>

class TleilaxShipTest : public testing::Test {
protected:
    TleilaxShipTest() = default;
};

TEST_F(TleilaxShipTest, ShipCreationTestDefaultConstructor) {
    Ship<Engine> ship;
    ASSERT_TRUE(entt::null == ship.get());
}

TEST_F(TleilaxShipTest, ShipCreationTestWithParameterizedConstructor) {
    entt::registry registry;
    Engine engine1 {0.0f, 0.0f};
    Engine engine2 {1.0f, 1.0f};
    Hull hull1 {0.0f, 0.0f};
    Hull hull2 {1.0f, 1.0f};

    Weapon weapon1 {1.0f};

    Ship<Engine, Hull> ship1{registry, engine1, hull1};
    Ship<Engine, Hull, Weapon> ship2{registry, engine2, hull2, weapon1};

    ASSERT_EQ(0.0f, registry.get<Engine>(ship1.get()).speed);
    ASSERT_EQ(0.0f, registry.get<Engine>(ship1.get()).max_speed);
    ASSERT_EQ(0.0f, registry.get<Hull>(ship1.get()).max_health);
    ASSERT_EQ(0.0f, registry.get<Hull>(ship1.get()).health);

    ASSERT_EQ(1.0f, registry.get<Engine>(ship2.get()).speed);
    ASSERT_EQ(1.0f, registry.get<Engine>(ship2.get()).max_speed);
    ASSERT_EQ(1.0f, registry.get<Hull>(ship2.get()).max_health);
    ASSERT_EQ(1.0f, registry.get<Hull>(ship2.get()).health);
    ASSERT_EQ(1.0f, registry.get<Weapon>(ship2.get()).damage);
}

TEST_F(TleilaxShipTest, TestCreationWithStaticFunction) {
    entt::registry registry;
    Ship<Engine>::create_with_components(registry, {1.0f, 1.0f});

    ASSERT_EQ(1, registry.size());
    registry.each([&](entt::entity entity){
        ASSERT_EQ(1.0f, registry.get<Engine>(entity).speed);
        ASSERT_EQ(1.0f, registry.get<Engine>(entity).max_speed);
    });
}

int main(int ac, char *av[]) {
    testing::InitGoogleTest(&ac, av);
    return RUN_ALL_TESTS();
}