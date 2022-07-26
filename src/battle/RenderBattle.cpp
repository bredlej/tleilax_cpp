//
// Created by geoco on 26.07.2022.
//
#include <battle.hpp>
void battle::Battle::render() {
    BeginDrawing();
    ClearBackground(Colors::col_0);
    //_render_visible();
    DrawText("Battle", 50, 50, 50, Colors::col_10);
    DrawFPS(1200, 10);
    //_draw_ui();
    EndDrawing();

}

void battle::Battle::update() {
    if (IsKeyDown(KEY_BACKSPACE)) {
        _core->dispatcher.enqueue<PlayerBattleEndEvent>();
    }
    _core->dispatcher.update();
}