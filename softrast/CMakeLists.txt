cmake_minimum_required(VERSION 3.15)

project(softrast C)

option(SWRZ_BUILD_WITH_ASAN "Build softrast with AddressSanitizer" OFF)
option(SWRZ_BUILD_BENCHMARK "Build benchmark for softrast" OFF)
option(SWRZ_BUILD_REFERENCE "Build reference implementation for softrast" OFF)

set(SWRZ_BUILD_DEBUG FALSE)

if(CMAKE_BUILD_TYPE MATCHES "[Dd]ebug"
        OR CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    set(SWRZ_BUILD_DEBUG TRUE)
endif()

if(SWRZ_BUILD_WITH_ASAN)
    if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        if(NOT SWRZ_BUILD_DEBUG)
            message(FATAL_ERROR "SWRZ_BUILD_WITH_ASAN MUST "
                "be built with Debug or RelWithDebInfo")
        endif()
    else()
        message(FATAL_ERROR "AddressSanitizer was not supported by "
            "'${CMAKE_C_COMPILER_ID}'")
    endif()
endif()

function(target_add_asan target)
    if(NOT SWRZ_BUILD_WITH_ASAN)
        return()
    endif()

    if (CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${target} PRIVATE
            -fsanitize=address,leak,undefined
            -fno-omit-frame-pointer
        )

        target_link_options(${target} PRIVATE
            -fsanitize=address
        )
    endif()
endfunction()

set(SWRZ_SOURCES
    src/swrz_alloc.c
    src/swrz_texture.c
    src/swrz_arena.c
    src/swrz_pool.c
    src/swrz_scene.c
    src/swrz_rasterizer.c
)

if(WIN32)
    list(APPEND SWRZ_SOURCES 
        src/win32/swrz_thread.c
        src/win32/swrz_mutex.c
        src/win32/swrz_condvar.c
    )
elseif(UNIX)
    list(APPEND SWRZ_SOURCES 
        src/posix/swrz_thread.c
        src/posix/swrz_mutex.c
        src/posix/swrz_condvar.c
    )
else()
    message(FATAL_ERROR "Unsupported platform. Aborted.")
endif()

add_library(softrast STATIC ${SWRZ_SOURCES})
target_include_directories(softrast PUBLIC include PRIVATE src)
set_target_properties(softrast PROPERTIES 
    C_STANDARD 99 C_STANDARD_REQUIRED ON)

function(target_add_strict_flags target)
    if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE
            $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:
                -Wall
                -Wextra
                -Wpedantic
                -Werror
                -Wconversion
                -Wsign-conversion
                -Wdouble-promotion
                -Wshadow
                -Wundef
                -Wcast-align
                -Wcast-qual
                -Wwrite-strings
                -Wstrict-prototypes
                -Wmissing-prototypes
                -Wold-style-definition
                -Wswitch-enum
                -Wswitch-default
                -Wimplicit-fallthrough
                -Wformat
                -Wformat-overflow
                -Wformat-truncation
                -Wformat-signedness
                -Warray-bounds
                -Wnull-dereference
                -Wvla
                
                -Wno-variadic-macro-arguments-omitted
                -Wno-gnu-zero-variadic-macro-arguments

                -fdiagnostics-show-option
                -fno-common
            >
        )
    endif()
endfunction()
target_add_strict_flags(softrast)
target_add_asan(softrast)

if(SWRZ_BUILD_BENCHMARK)
    # remove when softrast a separated repository
    if(NOT TARGET aparse)
        add_subdirectory(${CMAKE_SOURCE_DIR}/deps/aparse ${CMAKE_CURRENT_BINARY_DIR}/aparse)
    endif()
    
    if(NOT TARGET lodepng)
        add_subdirectory(deps/lodepng)
    endif()

    add_executable(benchmark 
        benchmark/str.c
        benchmark/cpu.c
        benchmark/sha256.c
        benchmark/main.c)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|i[3-6]86)$")
        if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR
        CMAKE_C_COMPILER_ID STREQUAL "Clang")
            set_source_files_properties(
                benchmark/cpu.c
                PROPERTIES
                    COMPILE_OPTIONS "-mxsave"
            )
        endif()
    endif()
    target_link_libraries(benchmark PRIVATE aparse softrast lodepng)
    target_compile_definitions(benchmark PRIVATE _GNU_SOURCE)
    target_add_strict_flags(benchmark)
    target_add_asan(benchmark)
    
endif()


if(SWRZ_BUILD_REFERENCE)
    find_package(OpenGL REQUIRED)
    find_package(glfw3 REQUIRED)
    add_subdirectory(deps/glad)

    # remove when softrast a separated repository
    if(NOT TARGET aparse)
        add_subdirectory(deps/aparse)
    endif()
    
    if(NOT TARGET lodepng)
        add_subdirectory(deps/lodepng)
    endif()

    add_executable(reference reference/main.c)
    target_link_libraries(reference PRIVATE 
        OpenGL::GL
        glfw
        glad
        aparse 
        softrast 
        lodepng)
    target_add_strict_flags(reference)
    target_add_asan(reference)

    add_executable(imgdiff reference/imgdiff.c)
    target_link_libraries(imgdiff PRIVATE 
        aparse
        lodepng)
    target_add_strict_flags(imgdiff)
    target_add_asan(imgdiff)
endif()
