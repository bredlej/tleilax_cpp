set(INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/extern
        )

#add_library(raylib-extension RaylibExtension.cpp)
#target_link_libraries(raylib-extension raylib)
#target_include_directories(raylib-extension PUBLIC ${INCLUDE_DIRS})

set(SOURCES
        Tleilax.cpp
        Fleet.cpp
        galaxy/Galaxy.cpp
        galaxy/UIGalaxy.cpp
        galaxy/RenderGalaxy.cpp
        Assets.cpp
        )

add_library(tleilax-lib ${SOURCES})

target_link_libraries(tleilax-lib EnTT raylib nlohmann_json::nlohmann_json imgui rlimgui)
target_include_directories(tleilax-lib PUBLIC ${INCLUDE_DIRS})