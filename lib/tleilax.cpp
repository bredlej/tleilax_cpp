//
// Created by geoco on 17.12.2021.
//
#include <tleilax.h>

void tleilax::Application::run(const Config &config) {

    InitWindow(config.window.width, config.window.height, config.title.data());
    SetTargetFPS(144);

    Assets assets {files::ship_components};
    auto ship_components = assets.get_ship_components();
    Galaxy g;
    g.populate();
    render_func = [&g](){g.render();};

    while (!WindowShouldClose()) {
        render_func();
        g.update();
    }
    CloseWindow();
}

