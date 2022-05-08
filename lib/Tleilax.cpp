//
// Created by geoco on 17.12.2021.
//
#include <tleilax.h>

void tleilax::Application::run(const Config &config) {

    _core->game_log.debug("Welcome commander!\n");
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(tleilax::Config::window.width, tleilax::Config::window.height, tleilax::Config::title.data());
    SetTargetFPS(144);
    _setup_imgui();
    auto g = std::make_shared<Galaxy>(_core, _assets);
    g->populate();

    _ui_view = g;

    while (!WindowShouldClose()) {
        _toggle_fullscreen();
        _ui_view->update();
        _ui_view->render();
    }
    rlImGuiShutdown();
    CloseWindow();
}

void tleilax::Application::_toggle_fullscreen() {
    if (IsKeyPressed(KEY_ENTER) && (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)))
    {
        if (IsWindowFullscreen())
        {
            ToggleFullscreen();
            SetWindowSize(tleilax::Config::window.width, tleilax::Config::window.height);
        }
        else
        {
            SetWindowSize(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor()));
            ToggleFullscreen();
        }
    }
}

void tleilax::Application::_setup_imgui() {
    rlImGuiSetup(true);
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;
    colors[ImGuiCol_Text] = ImVec4(static_cast<float>(Colors::col_16.r)/255.0f, static_cast<float>(Colors::col_16.g)/255.0f, static_cast<float>(Colors::col_16.b)/255.0f, 1);
    colors[ImGuiCol_WindowBg] = ImVec4(static_cast<float>(Colors::col_1.r)/255.0f, static_cast<float>(Colors::col_1.g)/255.0f, static_cast<float>(Colors::col_1.b)/255.0f, 1);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(static_cast<float>(Colors::col_1.r)/255.0f, static_cast<float>(Colors::col_1.g)/255.0f, static_cast<float>(Colors::col_1.b)/255.0f, 1);
    colors[ImGuiCol_PopupBg]                = ImVec4(static_cast<float>(Colors::col_1.r)/255.0f, static_cast<float>(Colors::col_1.g)/255.0f, static_cast<float>(Colors::col_1.b)/255.0f, 1);
    colors[ImGuiCol_Border]                 = ImVec4(static_cast<float>(Colors::col_3.r)/255.0f, static_cast<float>(Colors::col_3.g)/255.0f, static_cast<float>(Colors::col_3.b)/255.0f, 1);
    colors[ImGuiCol_BorderShadow]           = ImVec4(static_cast<float>(Colors::col_2.r)/255.0f, static_cast<float>(Colors::col_2.g)/255.0f, static_cast<float>(Colors::col_2.b)/255.0f, 1);
    colors[ImGuiCol_FrameBg]                = ImVec4(static_cast<float>(Colors::col_1.r)/255.0f, static_cast<float>(Colors::col_1.g)/255.0f, static_cast<float>(Colors::col_1.b)/255.0f, 1);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(static_cast<float>(Colors::col_2.r)/255.0f, static_cast<float>(Colors::col_2.g)/255.0f, static_cast<float>(Colors::col_2.b)/255.0f, 1);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(static_cast<float>(Colors::col_1.r)/255.0f, static_cast<float>(Colors::col_1.g)/255.0f, static_cast<float>(Colors::col_1.b)/255.0f, 1);
    colors[ImGuiCol_TitleBg]                = ImVec4(static_cast<float>(Colors::col_2.r)/255.0f, static_cast<float>(Colors::col_2.g)/255.0f, static_cast<float>(Colors::col_2.b)/255.0f, 1);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(static_cast<float>(Colors::col_2.r)/255.0f, static_cast<float>(Colors::col_2.g)/255.0f, static_cast<float>(Colors::col_2.b)/255.0f, 1);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(static_cast<float>(Colors::col_2.r)/255.0f, static_cast<float>(Colors::col_2.g)/255.0f, static_cast<float>(Colors::col_2.b)/255.0f, 1);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(static_cast<float>(Colors::col_3.r)/255.0f, static_cast<float>(Colors::col_3.g)/255.0f, static_cast<float>(Colors::col_3.b)/255.0f, 1);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(static_cast<float>(Colors::col_2.r)/255.0f, static_cast<float>(Colors::col_2.g)/255.0f, static_cast<float>(Colors::col_2.b)/255.0f, 1);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(static_cast<float>(Colors::col_3.r)/255.0f, static_cast<float>(Colors::col_3.g)/255.0f, static_cast<float>(Colors::col_3.b)/255.0f, 1);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(static_cast<float>(Colors::col_4.r)/255.0f, static_cast<float>(Colors::col_4.g)/255.0f, static_cast<float>(Colors::col_4.b)/255.0f, 1);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(static_cast<float>(Colors::col_2.r)/255.0f, static_cast<float>(Colors::col_2.g)/255.0f, static_cast<float>(Colors::col_2.b)/255.0f, 1);
    colors[ImGuiCol_CheckMark]              = ImVec4(static_cast<float>(Colors::col_5.r)/255.0f, static_cast<float>(Colors::col_5.g)/255.0f, static_cast<float>(Colors::col_5.b)/255.0f, 1);
    colors[ImGuiCol_SliderGrab]             = ImVec4(static_cast<float>(Colors::col_5.r)/255.0f, static_cast<float>(Colors::col_5.g)/255.0f, static_cast<float>(Colors::col_5.b)/255.0f, 1);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(static_cast<float>(Colors::col_4.r)/255.0f, static_cast<float>(Colors::col_4.g)/255.0f, static_cast<float>(Colors::col_4.b)/255.0f, 1);
    colors[ImGuiCol_Button]                 = ImVec4(static_cast<float>(Colors::col_3.r)/255.0f, static_cast<float>(Colors::col_3.g)/255.0f, static_cast<float>(Colors::col_3.b)/255.0f, 1);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(static_cast<float>(Colors::col_5.r)/255.0f, static_cast<float>(Colors::col_5.g)/255.0f, static_cast<float>(Colors::col_5.b)/255.0f, 1);
    colors[ImGuiCol_ButtonActive]           = ImVec4(static_cast<float>(Colors::col_4.r)/255.0f, static_cast<float>(Colors::col_4.g)/255.0f, static_cast<float>(Colors::col_4.b)/255.0f, 1);
    colors[ImGuiCol_Header]                 = ImVec4(static_cast<float>(Colors::col_3.r)/255.0f, static_cast<float>(Colors::col_3.g)/255.0f, static_cast<float>(Colors::col_3.b)/255.0f, 1);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(static_cast<float>(Colors::col_4.r)/255.0f, static_cast<float>(Colors::col_4.g)/255.0f, static_cast<float>(Colors::col_4.b)/255.0f, 1);
    colors[ImGuiCol_HeaderActive]           = ImVec4(static_cast<float>(Colors::col_5.r)/255.0f, static_cast<float>(Colors::col_5.g)/255.0f, static_cast<float>(Colors::col_5.b)/255.0f, 1);
    colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImVec4(static_cast<float>(Colors::col_1.r)/255.0f, static_cast<float>(Colors::col_1.g)/255.0f, static_cast<float>(Colors::col_1.b)/255.0f, 1);
    colors[ImGuiCol_TabHovered]             = ImVec4(static_cast<float>(Colors::col_5.r)/255.0f, static_cast<float>(Colors::col_5.g)/255.0f, static_cast<float>(Colors::col_5.b)/255.0f, 1);
    colors[ImGuiCol_TabActive]              = ImVec4(static_cast<float>(Colors::col_2.r)/255.0f, static_cast<float>(Colors::col_2.g)/255.0f, static_cast<float>(Colors::col_2.b)/255.0f, 1);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(static_cast<float>(Colors::col_2.r)/255.0f, static_cast<float>(Colors::col_2.g)/255.0f, static_cast<float>(Colors::col_2.b)/255.0f, 1);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(static_cast<float>(Colors::col_2.r)/255.0f, static_cast<float>(Colors::col_2.g)/255.0f, static_cast<float>(Colors::col_2.b)/255.0f, 1);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}
