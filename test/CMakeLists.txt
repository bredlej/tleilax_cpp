add_executable(test_tleilax-lib test_tleilax-lib.cpp)
target_link_libraries(test_tleilax-lib tleilax-lib gtest)

add_executable(test_ships test_ships.cpp)
target_link_libraries(test_ships tleilax-lib gtest)

add_test(Tleilax-Test test_tleilax-lib)
add_test(Tleilax-Test test_ships)