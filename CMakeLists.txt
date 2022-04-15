cmake_minimum_required(VERSION 3.18.4)
project(Tleilax)

set(CMAKE_CXX_STANDARD 20)
include(FetchContent)
include(CTest)
include(cmake/CPM.cmake)
include(cmake/fetch-googletest.cmake)
include(cmake/fetch-entt.cmake)
include(cmake/fetch-raylib.cmake)
include(cmake/fetch-json.cmake)

if (DEFINED ENV{BUILD_WASM})
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

if (${WASM})
    target_link_libraries(${PROJECT_NAME} tleilax-lib glfw3)
else ()
    target_link_libraries(${PROJECT_NAME} tleilax-lib)
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

