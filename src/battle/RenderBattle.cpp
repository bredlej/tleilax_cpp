//
// Created by geoco on 26.07.2022.
//
#include <battle.hpp>
void battle::Battle::render() {
    BeginDrawing();
    ClearBackground(Colors::col_0);
    //_render_visible();
    DrawText("Battle", 50, 50, 50, Colors::col_10);
    char attacker[10], opponent[10];
    std::sprintf(attacker, "%u", _attacker);
    std::sprintf(opponent, "%u", _opponent);
    DrawText(attacker, 50, 100, 50, Colors::col_12);
    DrawText(opponent, 50, 150, 50, Colors::col_12);

    DrawFPS(1200, 10);
    _draw_ui();
    EndDrawing();

}

void battle::Battle::update() {
    if (IsKeyDown(KEY_BACKSPACE)) {
        _core->dispatcher.enqueue<PlayerBattleEndEvent>();
    }
    if (IsKeyPressed(KEY_D)) {
        _open_demo = !_open_demo;
    }
    _core->dispatcher.update();
}

void battle::Battle::_draw_ui() {
    rlImGuiBegin();
    bool open = true;
    static ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;
    if (_open_demo) {
        ImGui::ShowDemoWindow(&_open_demo);
    } else {
        if (ImGui::Begin("Battle control", &open, flags)) {
            ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
            if (ImGui::IsMousePosValid())
                if (ImGui::BeginTabBar("Controls", tab_bar_flags)) {

                    ImGui::EndTabBar();
                }
        }
        ImGui::End();
    }
    rlImGuiEnd();
}
