//
// Created by geoco on 17.12.2021.
//

#ifndef TLEILAX_CPP_TLEILAX_H
#define TLEILAX_CPP_TLEILAX_H
#include <cstdint>
#include <entt/entt.hpp>
#include <raylib.h>
#include <string_view>
#include <concepts>
#include <functional>

struct RendererBase {
    virtual std::function<void(void)> get_render_func(void) const;
};

namespace renderables {
    struct Intro : RendererBase { std::function<void(void)> get_render_func(void) const override; };
}

namespace tleilax {
    struct Config {
        static constexpr std::string_view title = "Tleilax";
        struct {
            uint32_t width;
            uint32_t height;
        } window;
    };
    struct Application {
    public:
        Application() = default;
        void run(const Config &);
    private:
        std::function<void(void)> render_func;
    };
}// namespace tleilax
#endif//TLEILAX_CPP_TLEILAX_H
