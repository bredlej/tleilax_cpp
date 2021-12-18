cmake_minimum_required(VERSION 3.20)
project(tleilax_cpp)

set(CMAKE_CXX_STANDARD 20)
include(FetchContent)
FetchContent_Declare(
        EnTT
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG        v3.8.1
)
FetchContent_Populate(EnTT)
add_subdirectory(${EnTT_SOURCE_DIR} ${EnTT_BINARY_DIR})
FetchContent_MakeAvailable(EnTT)

add_subdirectory(lib)
add_executable(tleilax_cpp main.cpp)
target_link_libraries(tleilax_cpp tleilax-lib)

