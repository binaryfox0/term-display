cmake_minimum_required(VERSION 3.10)

project(term-display C)

set(CMAKE_C_STANDARD 11)

# Collect source files
file(GLOB sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.c)

# Create term-display library
add_library(term-display STATIC ${sources})
target_include_directories(term-display PUBLIC include)

# Apply warnings **only** to term-display (not global)
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    #target_compile_options(term-display PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(term-display PRIVATE /W4 /WX)
endif()

# Add example subdirectory
add_subdirectory(example)
