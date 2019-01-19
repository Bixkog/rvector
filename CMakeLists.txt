cmake_minimum_required(VERSION 3.5)
project(rvector)
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++17 -ggdb -g -Wall -Wextra")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "-std=c++17 -O3 -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")

find_package(Boost COMPONENTS container)
# build google benchmark (target: benchmark)
# do not build tests of benchmarking lib
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Suppressing benchmark's tests" FORCE)
add_subdirectory(vendor/google/benchmark)

# build tests (targets: gmock_main, gtest)
add_subdirectory(vendor/google/googletest/googletest)

add_executable(runUnitTests
    src/test.cpp
    src/rvector.h
    src/allocator.h
    src/test_type.h
    src/test_type.cpp)

add_executable(runBenchmarks
	src/benchmark.cpp
    src/rvector.h
    src/allocator.h
    src/test_type.h
    src/test_type.cpp)

add_executable(runProfile
    src/profile.cpp
    src/rvector.h
    src/allocator.h
    src/test_type.h
    src/test_type.cpp)

target_link_libraries(runUnitTests gtest gtest_main pthread)
target_link_libraries(runBenchmarks benchmark pthread)
target_link_libraries(runProfile ${Boost_LIBRARIES} EASTL)

add_test(
    NAME runUnitTests
    COMMAND runUnitTests --gtest_color=yes)
