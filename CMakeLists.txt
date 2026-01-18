cmake_minimum_required(VERSION 3.10)

project(term-display C)

set(TD_SOURCES
    src/td_dynarr.c
    src/td_main.c
    src/td_priv.c
    src/td_texture.c
    src/td_font.c
    src/td_rasterizer.c
    src/td_renderer.c
)

if(WIN32)
    list(APPEND TD_SOURCES src/td_priv_win32.c)
elseif(UNIX)
    list(APPEND TD_SOURCES src/td_priv_unix.c)
else()
    message(FATAL_ERROR "Unsupported platform. Aborted.")
endif()

list(APPEND TD_SOURCES $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:src/td_debug.c>)

add_library(term-display STATIC ${TD_SOURCES})
target_include_directories(term-display PUBLIC include)
set_target_properties(term-display PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
target_compile_definitions(term-display PRIVATE
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:TD_BUILD_DEBUG>
    $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:TD_BUILD_RELEASE>
)

if(CMAKE_BUILD_TYPE MATCHES "[Dd]ebug")
    if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(term-display PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:
            -Werror -Wall -Wextra -pedantic -Wcast-align
            -Wcast-qual -Wdisabled-optimization -Wformat=2
            -Winit-self -Wmissing-include-dirs
            -Wredundant-decls -Wshadow -Wundef -Wno-unused 
            -Wno-variadic-macros -Wno-parentheses 
            -fdiagnostics-show-option -Wconversion
    >)
    endif()
endif()

add_subdirectory(example)
add_subdirectory(tests)
