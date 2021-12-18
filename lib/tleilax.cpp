//
// Created by geoco on 17.12.2021.
//
#include <tleilax.h>

std::function<void(void)> RendererBase::get_render_func() const { return [] () {}; };

void tleilax::Application::run(const Config &config) {

    InitWindow(config.window.width, config.window.height, config.title.data());

    render_func = renderables::Intro().get_render_func();

    while (!WindowShouldClose()) {
        render_func();
    }
    CloseWindow();
}

