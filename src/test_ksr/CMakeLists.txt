enable_testing()

add_executable(test_type_util test_type_util.cpp)
add_test(test_type_util test_type_util)

target_link_libraries(test_type_util Qt5::Test)





