cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Coverage

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")

add_subdirectory(lib)

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

target_include_directories(serial PUBLIC include)
target_include_directories(serial PRIVATE src)

# Tests

file(GLOB serial_test_srcs
    test/*.cpp
)

add_executable(test-serial
    ${serial_test_srcs}
)

target_link_libraries(test-serial
    PUBLIC serial jsoncpp
)

target_include_directories(test-serial
    PUBLIC test
)
