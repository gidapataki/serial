cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Coverage

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")


# Serialize

file(GLOB serialize_srcs
    src/*.cpp
)

add_library(serialize
    ${serialize_srcs}
)

target_include_directories(serialize PUBLIC include)
target_include_directories(serialize PRIVATE src)


# Tests

file(GLOB serialize_test_srcs
    test/*.cpp
)

add_executable(test-serialize
    ${serialize_test_srcs}
)

target_link_libraries(test-serialize
    PUBLIC serialize
)

target_include_directories(test-serialize
    PUBLIC test
)
