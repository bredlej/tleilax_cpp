//
// Created by geoco on 26.07.2022.
//
#include <battle.hpp>
void battle::Battle::render() {
    BeginDrawing();
    ClearBackground(Colors::col_0);
    _render_visible();
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

Camera battle::Battle::_initialize_camera(const Vector3 &cameraInitialPosition, const float cameraDistance,
                                  const float horizontalDistance, const float horizontalAngle,
                                  const float verticalAngle) {
    Camera camera;
    camera.target = cameraInitialPosition;
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    camera.position.x = horizontalDistance * std::cosh(horizontalAngle * PI / 180.0f);
    camera.position.y = horizontalDistance * std::sinh(horizontalAngle * PI / 180.0f);
    camera.position.z = cameraDistance * sinf(verticalAngle * PI / 180.0f);
    SetCameraMode(camera, CAMERA_ORBITAL);
    UpdateCamera(&camera);
    return camera;
}

void battle::Battle::_render_visible() {
    BeginMode3D(_camera);
    _render_grid();
    EndMode3D();
}

void battle::Battle::_render_grid() {
    DrawGrid(50, 10.0f);
}