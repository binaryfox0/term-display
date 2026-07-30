cmake_minimum_required(VERSION 3.10)

project(term-display C)

option(TD_BUILD_ASAN "Build every binary with AddressSanitizer" OFF)
option(TD_BUILD_TEST "Build term-display tests" OFF)
option(TD_BUILD_EXAMPLE "Build term-display examples" OFF)

set(TD_BUILD_DEBUG FALSE)

if(CMAKE_BUILD_TYPE MATCHES "[Dd]ebug"
        OR CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    set(TD_BUILD_DEBUG TRUE)
endif()

if(TD_BUILD_ASAN)
    if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        if(NOT TD_BUILD_DEBUG)
            message(FATAL_ERROR "TD_BUILD_ASAN MUST be built with Debug or RelWithDebInfo")
        endif()
    else()
        message(FATAL_ERROR "AddressSanitizer was not supported by '${CMAKE_C_COMPILER_ID}'")
    endif()
endif()

function(target_add_asan target_name)
    if(NOT TD_BUILD_ASAN)
        return()
    endif()

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${target_name} PRIVATE
            -fsanitize=address
            -fno-omit-frame-pointer
        )

        target_link_options(${target_name} PRIVATE
            -fsanitize=address
        )
    endif()
endfunction()

add_subdirectory(softrast)

set(TD_SOURCES
    #    src/td_dynarr.c
    #    src/td_priv.c
    #    src/td_texture.c
    src/td_utils.c
    src/td_context.c
    src/td_error.c
    src/td_misc.c
    src/td_timer.c
    src/td_main.c
    src/td_window.c
    src/td_rbuf.c
    src/td_event.c
    src/td_texture.c
    src/td_rasterizer.c
    src/td_renderer.c
    #    src/td_font.c
)

if(WIN32)
    list(APPEND TD_SOURCES 
        src/win32/td_timer.c
        src/win32/td_term.c
    )
elseif(UNIX)
    list(APPEND TD_SOURCES 
        src/unix/td_timer.c
        src/unix/td_term.c
        src/unix/td_input.c
    )
else()
    message(FATAL_ERROR "Unsupported platform. Aborted.")
endif()

list(APPEND TD_SOURCES $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:src/td_debug.c>)

add_library(term-display STATIC ${TD_SOURCES})
target_include_directories(term-display PUBLIC include PRIVATE src)
set_target_properties(term-display PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON)
target_compile_definitions(term-display PRIVATE
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:TD_BUILD_DEBUG>
    $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:TD_BUILD_RELEASE>
)

if(CMAKE_BUILD_TYPE MATCHES "[Dd]ebug")
    if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(term-display PRIVATE
            $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:
                -Werror
                -Wall
                -Wextra
                -Wpedantic
                -Wconversion
                -Wcast-align
                -Wcast-qual
                -Wwrite-strings
                -Wstrict-prototypes
                -Wmissing-prototypes
                -Wshadow
                -Wswitch
                -Wundef
                -Wnull-dereference
                -Wformat=2
                -fdiagnostics-show-option
                -Winit-self
                -Wmissing-include-dirs
                -Wdisabled-optimization
                -Wno-unused-parameter
                -Wno-parentheses
                -Wno-variadic-macro-arguments-omitted
            >
        )
    endif()
endif()
target_add_asan(term-display)

if(TD_BUILD_EXAMPLE OR TD_BUILD_TEST)
    if(NOT EXISTS example/deps/aparse)
        message(FATAL_ERROR "Dependency 'aparse' is missing. Run 'git submodule update --init --recursive' to fetch it.")
    endif()

    add_subdirectory(example/deps/aparse)
    if(TD_BUILD_EXAMPLE)
        add_subdirectory(example)
    endif()

    if(TD_BUILD_TEST)
        if(TD_BUILD_DEBUG)
            add_subdirectory(tests)
        else() 
            message(FATAL_ERROR "Tests required to be built with Debug or RelWithDebInfo")
        endif()
    endif()
endif()
