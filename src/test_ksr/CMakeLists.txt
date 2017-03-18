enable_testing()

add_executable(test_narrow_cast test_narrow_cast.cpp)
add_test(test_narrow_cast test_narrow_cast)

target_link_libraries(test_narrow_cast Qt5::Test)





