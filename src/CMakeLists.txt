set(INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/extern
        )

set(SOURCES
        Tleilax.cpp
        Fleet.cpp
        galaxy/Galaxy.cpp
        galaxy/UIGalaxy.cpp
        galaxy/RenderGalaxy.cpp
        starsystem/StarSystem.cpp
        starsystem/RenderStarSystem.cpp
        Assets.cpp
        Core.cpp
        battle/Battle.cpp
        battle/RenderBattle.cpp
        )

add_library(tleilax-lib ${SOURCES})

target_link_libraries(tleilax-lib EnTT raylib nlohmann_json::nlohmann_json imgui rlimgui)
target_include_directories(tleilax-lib PUBLIC ${INCLUDE_DIRS})