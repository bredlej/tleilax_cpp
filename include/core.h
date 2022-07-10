//
// Created by geoco on 15.04.2022.
//

#ifndef TLEILAX_CORE_H
#define TLEILAX_CORE_H

#include <entt/entt.hpp>
#include <pcg/pcg_random.hpp>
#include <name_generator.h>
#include <colors.h>
#include <graphics_base.h>
#include <memory>
#include <unordered_map>

enum class UIState {
    GalaxyView,
    StarSystemView
};

constexpr auto seed_function = [](const uint32_t x, const uint32_t y, const uint32_t z) {
    return ((x + y) >> 1) * (x + y + 1) + y * ((x + z) >> 1) * (x + z + 1) + z;
};

struct UIView {
    virtual void render() = 0;
    virtual void update() = 0;
};

class Core {
public:
    explicit Core(int width, int height) : window{width, height} {};
    entt::dispatcher dispatcher{};
    entt::registry registry{};
    pcg32 pcg;
    NameGenerator name_generator{};
    constexpr static Colors colors;
    struct {int width; int height;} window;
    TleilaxAppLog debug_log;
    TleilaxAppLog game_log;
};
#endif//TLEILAX_CORE_H
