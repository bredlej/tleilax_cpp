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

enable_testing()

add_subdirectory(test)
add_subdirectory(lib)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} tleilax-lib)

#if (EMSCRIPTEN)
#    # 32MB should be enough to run all the examples, in debug mode.
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s NO_DISABLE_EXCEPTION_CATCHING")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s NO_DISABLE_EXCEPTION_CATCHING")
#    set(CMAKE_EXECUTABLE_SUFFIX ".html")
#    target_link_options(${PROJECT_NAME} PUBLIC "SHELL: -s TOTAL_MEMORY=131072000")
#    foreach(file
#            "./index.html"
#            "./run_webassembly.py")
#        configure_file(${file} ${file})
#    endforeach(file)
#endif()

