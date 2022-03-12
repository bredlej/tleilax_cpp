//
// Created by geoco on 20.02.2022.
//
#include <tleilax.h>
#include <gtest/gtest.h>
using namespace components;

class TleilaxShipTest : public testing::Test {
protected:
    TleilaxShipTest() = default;
};

TEST_F(TleilaxShipTest, ShipCreationTestDefaultConstructor) {
    Ship<components::Engine> ship;
    ASSERT_TRUE(entt::null == ship.get());
}

TEST_F(TleilaxShipTest, ShipCreationTestWithParameterizedConstructor) {
    entt::registry registry;
    Engine engine1 {"e1", "e1", 0, 0};
    Engine engine2 {"e2", "e2", 1, 1};

    Hull hull1 {"h1", "h1", 0.0f, 0.0f};
    Hull hull2 {"h2", "h2", 1.0f, 1.0f};

    Weapon weapon1 {"w1", "w1", 1, {1, 4}, 1};

    Ship<Engine, Hull> ship1{registry, engine1, hull1};
    Ship<Engine, Hull, Weapon> ship2{registry, engine2, hull2, weapon1};

    ASSERT_EQ("e1", registry.get<Engine>(ship1.get()).id);
    ASSERT_EQ("e1", registry.get<Engine>(ship1.get()).name);
    ASSERT_EQ(0, registry.get<Engine>(ship1.get()).power);
    ASSERT_EQ(0, registry.get<Engine>(ship1.get()).weight);

    ASSERT_EQ("h1", registry.get<Hull>(ship1.get()).id);
    ASSERT_EQ("h1", registry.get<Hull>(ship1.get()).name);
    ASSERT_EQ(0.0f, registry.get<Hull>(ship1.get()).max_health);
    ASSERT_EQ(0.0f, registry.get<Hull>(ship1.get()).health);

    ASSERT_EQ("e2", registry.get<Engine>(ship2.get()).id);
    ASSERT_EQ("e2", registry.get<Engine>(ship2.get()).name);
    ASSERT_EQ(1, registry.get<Engine>(ship2.get()).power);
    ASSERT_EQ(1, registry.get<Engine>(ship2.get()).weight);

    ASSERT_EQ("h2", registry.get<Hull>(ship2.get()).id);
    ASSERT_EQ("h2", registry.get<Hull>(ship2.get()).name);
    ASSERT_EQ(1.0f, registry.get<Hull>(ship2.get()).max_health);
    ASSERT_EQ(1.0f, registry.get<Hull>(ship2.get()).health);

    ASSERT_EQ("w1", registry.get<Weapon>(ship2.get()).id);
    ASSERT_EQ("w1", registry.get<Weapon>(ship2.get()).name);
    ASSERT_EQ(1, registry.get<Weapon>(ship2.get()).power_usage);
    ASSERT_EQ(1, registry.get<Weapon>(ship2.get()).damage.amount);
    ASSERT_EQ(4, registry.get<Weapon>(ship2.get()).damage.sides);
    ASSERT_EQ(1, registry.get<Weapon>(ship2.get()).weight);
}

TEST_F(TleilaxShipTest, TestCreationWithStaticFunction) {
    entt::registry registry;
    Ship<Engine>::create_with_components(registry, {"e1", "e1", 1, 1});

    ASSERT_EQ(1, registry.size());
    registry.each([&](entt::entity entity){
        ASSERT_EQ("e1", registry.get<Engine>(entity).id);
        ASSERT_EQ("e1", registry.get<Engine>(entity).name);
        ASSERT_EQ(1, registry.get<Engine>(entity).power);
        ASSERT_EQ(1, registry.get<Engine>(entity).weight);
    });
}

int main(int ac, char *av[]) {
    testing::InitGoogleTest(&ac, av);
    return RUN_ALL_TESTS();
}