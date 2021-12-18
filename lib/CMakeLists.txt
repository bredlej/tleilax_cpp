add_library(tleilax-lib tleilax.cpp)
target_link_libraries(tleilax-lib EnTT raylib)
target_include_directories(tleilax-lib PUBLIC ${PROJECT_SOURCE_DIR}/include)