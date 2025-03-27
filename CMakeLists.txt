cmake_minimum_required(VERSION 3.10)

project(term-display C)

set(CMAKE_C_STANDARD 99)

file(GLOB sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.c)

# Create the term-display library
add_library(term-display ${sources})
target_include_directories(term-display PUBLIC include)

# Add example subdirectory
add_subdirectory(example)
