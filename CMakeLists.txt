cmake_minimum_required(VERSION 3.5)
project(rvector)
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++17 -ggdb -g -Wall -Wextra -static")
set(CMAKE_CXX_FLAGS_RELEASE "-std=c++17 -O3 -Wall -Wextra -static")

add_subdirectory(/home/wojtek/Downloads/googletest binary_dir EXCLUDE_FROM_ALL)

enable_testing()

add_executable(runUnitTests
    test.cpp)

add_executable(runBenchmarks
	benchmark.cpp)

add_executable(runProfile
    profile.cpp)
set_target_properties(runProfile PROPERTIES COMPILE_FLAGS "-g" )

target_link_libraries(runUnitTests gtest gmock_main pthread)
target_link_libraries(runBenchmarks benchmark pthread)


add_test(
    NAME runUnitTests
    COMMAND runUnitTests --gtest_color=yes)
