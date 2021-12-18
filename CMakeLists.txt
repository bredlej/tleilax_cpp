cmake_minimum_required(VERSION 3.20)
project(tleilax_cpp)

set(CMAKE_CXX_STANDARD 20)
include(FetchContent)
include(CTest)

FetchContent_Declare(
        EnTT
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG        v3.8.1
)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.11.0
)
FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 4.0.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_Populate(EnTT)
add_subdirectory(${EnTT_SOURCE_DIR} ${EnTT_BINARY_DIR})

FetchContent_GetProperties(raylib)
if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib)

    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples

    # build raylib
    add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})

endif ()

FetchContent_MakeAvailable(EnTT)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_subdirectory(test)
add_subdirectory(lib)

add_executable(tleilax_cpp main.cpp)
target_link_libraries(tleilax_cpp tleilax-lib)

