//
// Created by geoco on 18.12.2021.
//
#include <tleilax.h>

std::function<void(void)> renderables::Intro::get_render_func(void) const {
    return []() {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("T L E I L A X", 120, 180, 100, RAYWHITE);
        DrawText("Press <Tab>", 30, 580, 30, RAYWHITE);
        EndDrawing();
    };
}