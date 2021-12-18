cmake_minimum_required(VERSION 3.20)
project(Tleilax)

set(CMAKE_CXX_STANDARD 20)
include(FetchContent)
include(CTest)
include(cmake/CPM.cmake)
include(cmake/fetch-googletest.cmake)
include(cmake/fetch-entt.cmake)
include(cmake/fetch-raylib.cmake)

enable_testing()

add_subdirectory(test)
add_subdirectory(lib)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} tleilax-lib)

