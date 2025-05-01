cmake_minimum_required(VERSION 3.10)

project(term-display C)

set(CMAKE_C_STANDARD 11)

file(GLOB sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.c)

if(WIN32)
    list(REMOVE_ITEM sources src/td_priv_unix.c)
elseif(UNIX)
    list(REMOVE_ITEM sources src/td_priv_win32.c)
else()
    message(FATAL_ERROR "Unsupported platform. Aborted.")
endif()

# Create term-display library
add_library(term-display STATIC ${sources})
target_include_directories(term-display PUBLIC include)

# Apply warnings **only** to term-display (not global)
if(CMAKE_BUILD_TYPE MATCHES "[Dd]ebug")
    if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(term-display PRIVATE
            -Werror -Wall -Wextra -pedantic -Wcast-align
            -Wcast-qual -Wdisabled-optimization -Wformat=2
            -Winit-self -Wlogical-op -Wmissing-include-dirs
            -Wredundant-decls -Wshadow -Wundef -Wno-unused 
            -Wno-variadic-macros -Wno-parentheses 
            -fdiagnostics-show-option -Wconversion
        )
    endif()
endif()

add_subdirectory(example)
