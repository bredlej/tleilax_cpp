//
// Created by geoco on 17.12.2021.
//
#include <tleilax.h>
#include <chrono>
#include <cstdio>

void tleilax::Application::run(const Config &config) {

    InitWindow(config.window.width, config.window.height, config.title.data());
    SetTargetFPS(60);

    //render_func = renderables::Intro().get_render_func();
    Galaxy g;
    g.populate();
    render_func = [&g](){g.render();};

    while (!WindowShouldClose()) {
        render_func();
    }
    CloseWindow();
}

