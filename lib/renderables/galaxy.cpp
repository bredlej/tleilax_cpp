//
// Created by geoco on 19.12.2021.
//
#include <tleilax.h>

std::function<void(void)> renderables::GalaxyView::get_render_func() const {
    return [] () {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText(" - - - - - - ", 120, 180, 100, RAYWHITE);
        DrawText("Press <Tab>", 30, 580, 30, RAYWHITE);
        EndDrawing();
    };
}