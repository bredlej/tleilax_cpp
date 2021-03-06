//
// Created by geoco on 15.04.2022.
//

#ifndef TLEILAX_CORE_HPP
#define TLEILAX_CORE_HPP

#include <colors.hpp>
#include <name_generator.hpp>
#include <pcg/pcg_random.hpp>
#include <entt/entt.hpp>
#include <graphics_base.hpp>
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
    virtual ~UIView() = default;
};

class Core {
public:
    explicit Core() noexcept : window{1200, 760} {};
    explicit Core(int width, int height) noexcept : window{width, height} {};
    Core(const Core&) noexcept = delete;
    Core(Core&&) noexcept = delete;
    Core& operator=(const Core&) noexcept = delete;
    Core& operator=(Core&&) noexcept = delete;

    uint32_t get_seed_of(const entt::entity) const ;
    entt::dispatcher dispatcher{};
    entt::registry registry{};
    pcg32 pcg;
    NameGenerator name_generator{};
    constexpr static Colors colors;
    struct {int width; int height;} window;
    TleilaxAppLog debug_log;
    TleilaxAppLog game_log;
};
#endif//TLEILAX_CORE_HPP
