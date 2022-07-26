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

namespace tleilax {
    struct Config {
        static constexpr std::string_view title = "Tleilax";
        static constexpr struct {
            uint32_t width;
            uint32_t height;
        } window{1280, 720};
    };
    class Application {
    public:
        explicit Application() noexcept
            : _assets{files::ship_components, files::ships},
              _core(std::make_shared<Core>(Config::window.width, Config::window.height)){};
        Application(const Application &) noexcept = delete;
        Application(Application &) noexcept = delete;
        Application(Application &&) noexcept = delete;
        Application &operator=(const Application &) noexcept = delete;
        Application &operator=(const Application &&) noexcept = delete;

        void run(const Config &);

    private:
        Assets _assets;
        std::shared_ptr<Core> _core;
        std::shared_ptr<UIView> _ui_view;
        void _setup_imgui();
        void _toggle_fullscreen();
    };
}// namespace tleilax

#endif//TLEILAX_CPP_TLEILAX_H
