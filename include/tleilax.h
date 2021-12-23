//
// Created by geoco on 17.12.2021.
//

#ifndef TLEILAX_CPP_TLEILAX_H
#define TLEILAX_CPP_TLEILAX_H
#include <Galaxy.h>
#include <concepts>
#include <cstdint>
#include <functional>
#include <raylib.h>
#include <string_view>

struct RendererBase {
    virtual std::function<void()> get_render_func() = delete;
};

namespace renderables {
    struct Intro : RendererBase {
        [[nodiscard]] std::function<void()> get_render_func() const;
    };
    struct GalaxyView : RendererBase {
        int amount_stars;
        [[nodiscard]] std::function<void()> get_render_func() const;
    };
}// namespace renderables

namespace tleilax {
    struct Config {
        static constexpr std::string_view title = "Tleilax";
        static constexpr struct {
            uint32_t width;
            uint32_t height;
        } window {800, 600};
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
