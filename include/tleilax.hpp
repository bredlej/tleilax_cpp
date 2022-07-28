//
// Created by geoco on 17.12.2021.
//

#ifndef TLEILAX_CPP_TLEILAX_H
#define TLEILAX_CPP_TLEILAX_H

#include <assets.hpp>
#include <concepts>
#include <cstdint>
#include <fleet.hpp>
#include <functional>
#include <galaxy.hpp>
#include <imgui/rlImGui.h>
#include <memory>
#include <raylib.h>
#include <ship.hpp>
#include <star_system.hpp>
#include <string_view>
#include <utility>
#include <unordered_map>
#include <battle.hpp>

namespace tleilax {
    struct Config {
        static constexpr std::string_view title = "Tleilax";
        static constexpr struct {
            uint32_t width;
            uint32_t height;
        } window{1280, 720};
    };
    enum class ViewMode {
        Galaxy, Battle
    };
    class Application {
    public:
        explicit Application() noexcept
            : _assets{files::ship_components, files::ships},
              _core(std::make_shared<Core>(Config::window.width, Config::window.height)),
              _view_mode{ViewMode::Galaxy} {};
        Application(const Application &) noexcept = delete;
        Application(Application &) noexcept = delete;
        Application(Application &&) noexcept = delete;
        Application &operator=(const Application &) noexcept = delete;
        Application &operator=(const Application &&) noexcept = delete;

        void run(const Config &);

    private:
        Assets _assets;
        std::shared_ptr<Core> _core;
        ViewMode _view_mode;
        std::unordered_map<ViewMode, std::unique_ptr<UIView>> _views;
        void _setup_imgui();
        void _register_events();
        void _toggle_fullscreen();

        void _on_start_battle(const PlayerBattleStartEvent &);
        void _on_end_battle(const PlayerBattleEndEvent &);
    };
}// namespace tleilax

#endif//TLEILAX_CPP_TLEILAX_H
