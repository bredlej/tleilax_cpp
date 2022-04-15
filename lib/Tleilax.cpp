//
// Created by geoco on 17.12.2021.
//
#include <tleilax.h>

void tleilax::Application::run(const Config &config) {

    InitWindow(tleilax::Config::window.width, tleilax::Config::window.height, tleilax::Config::title.data());
    SetTargetFPS(144);

    std::shared_ptr<Galaxy> g = std::make_shared<Galaxy>(_core, _assets);
    g->populate();

    _ui_view = g;

    while (!WindowShouldClose()) {
        _ui_view->render();
        _ui_view->update();
    }
    CloseWindow();
}

