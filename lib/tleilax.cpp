//
// Created by geoco on 17.12.2021.
//
#include <tleilax.h>

void tleilax::Application::run(const Config &config) {

    SetConfigFlags(FLAG_MSAA_4X_HINT);// Enable Multi Sampling Anti Aliasing 4x (if available)
    InitWindow(config.window.width, config.window.height, config.title.data());
    SetTargetFPS(60);

    Galaxy g;
    g.populate();
    render_func = [&g](){g.render();};

    while (!WindowShouldClose()) {
        render_func();
        g.update();
    }

    CloseWindow();
}

