set(SOURCES
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_demo.cpp
        imgui/rlImGui.cpp
        Tleilax.cpp
        renderables/intro.cpp
        renderables/galaxy.cpp
        Fleet.cpp
        Galaxy.cpp
        Assets.cpp
        )
set(INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/extern
        )
add_library(tleilax-lib ${SOURCES})
target_link_libraries(tleilax-lib EnTT raylib nlohmann_json::nlohmann_json)
target_include_directories(tleilax-lib PUBLIC ${INCLUDE_DIRS})