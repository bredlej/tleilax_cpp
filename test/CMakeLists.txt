add_executable(test_tleilax-lib test_tleilax-lib.cpp)
target_link_libraries(test_tleilax-lib tleilax-lib gtest)

add_executable(test_ships test_ships.cpp)
target_link_libraries(test_ships tleilax-lib gtest)

add_executable(test_components test_components.cpp)
target_link_libraries(test_components tleilax-lib gtest)

add_executable(test_path test_path.cpp)
target_link_libraries(test_path tleilax-lib gtest)

add_executable(test_graph test_graph.cpp)
target_link_libraries(test_graph tleilax-lib gtest)

add_test(Tleilax-Test test_tleilax-lib)
add_test(Tleilax-Test test_ships)
add_test(Tleilax-Test test_components)
add_test(Tleilax-Test test_path)
add_test(Tleilax-Test test_graph)