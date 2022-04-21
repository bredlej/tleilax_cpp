cmake_minimum_required(VERSION 3.16.3)
project(Tleilax)

set(CMAKE_CXX_STANDARD 20)
include(FetchContent)
include(CTest)
include(cmake/CPM.cmake)
include(cmake/fetch-googletest.cmake)
include(cmake/fetch-entt.cmake)
include(cmake/fetch-json.cmake)
include(cmake/fetch-raylib.cmake)

if(APPLE)
    find_library(GLFW glfw3 PATHS glfw)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(OPENGL_LIBRARY OpenGL)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    set(EXTRA_LIBS
            ${COCOA_LIBRARY}
            ${OPENGL_LIBRARY}
            ${IOKIT_LIBRARY}
            ${COREVIDEO_LIBRARY}
            ${GLFW}
            )
endif(APPLE)

if (DEFINED ENV{BUILD_WASM})
    include(cmake/fetch-raylib.cmake)
    message("-- Building for WebAssembly")
    set(WASM ON)
else ()
    message("-- Building for Desktop")
    set(WASM OFF)
endif ()

enable_testing()

add_subdirectory(test)
add_subdirectory(lib)
add_executable(${PROJECT_NAME} main.cpp)

set(IMGUI_SOURCES
        extern/imgui/imgui.cpp
        extern/imgui/imgui_demo.cpp
        extern/imgui/imgui_draw.cpp
        extern/imgui/imgui_widgets.cpp
        extern/imgui/imgui_tables.cpp
        extern/imgui/imgui.h
        extern/imgui/imgui_internal.h
        extern/imgui/misc/cpp/imgui_stdlib.cpp
        extern/imgui/misc/cpp/imgui_stdlib.h
        )
add_library(imgui ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC extern/imgui)

add_library(rlimgui extern/rlimgui/rlImGui.cpp extern/rlimgui/rlImGui.h)
target_link_libraries(rlimgui PUBLIC imgui raylib)
target_include_directories(rlimgui PUBLIC raylib/src/external/glfw/include)

if (${WASM})
    target_link_libraries(${PROJECT_NAME} tleilax-lib glfw3)
else ()
    target_link_libraries(${PROJECT_NAME} tleilax-lib ${EXTRA_LIBS})
endif ()

add_custom_command(
        TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_CURRENT_BINARY_DIR}/assets)

if (EMSCRIPTEN AND ${WASM})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s TOTAL_MEMORY=32MB USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --preload-file assets -s TOTAL_MEMORY=32MB -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY ")
    set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so taht you can directly open it.
    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ../docs/generated)
    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_NAME}.js ../docs/generated/${PROJECT_NAME}.js)
    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_NAME}.wasm ../docs/generated/${PROJECT_NAME}.wasm)
    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_NAME}.html ../docs/generated/index.html)
    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_NAME}.data ../docs/generated/${PROJECT_NAME}.data)
endif ()

