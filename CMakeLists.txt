cmake_minimum_required(VERSION 3.1)
include(Sugoi)
cmake_policy(SET CMP0063 NEW)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Coverage

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")

add_subdirectory(lib/jsoncpp)


# Serial

file(GLOB serial_srcs
    src/*.cpp
)

add_library(serial
    ${serial_srcs}
)

target_link_libraries(serial
    PUBLIC jsoncpp
)

target_include_directories(serial PUBLIC include lib/boost/include)
target_include_directories(serial PRIVATE src)

# Tests

file(GLOB serial_test_srcs
    test/*.cpp
)

add_executable(test-serial
    ${serial_test_srcs}
)

target_link_libraries(test-serial
    PUBLIC serial jsoncpp gtest_main
)

target_include_directories(test-serial
    PUBLIC test
)


add_executable(example
    # examples/example.cpp
    examples/small-example.cpp
)

target_link_libraries(example
    PUBLIC serial jsoncpp
)

add_executable(sandbox
    sandbox.cpp
)

target_include_directories(sandbox
    PRIVATE include
)
