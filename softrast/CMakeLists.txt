cmake_minimum_required(VERSION 3.15)

project(softrast C)

option(SWRZ_BUILD_WITH_ASAN 
    "Build softrast with AddressSanitizer" OFF)

set(SWRZ_BUILD_DEBUG FALSE)

if(CMAKE_BUILD_TYPE MATCHES "[Dd]ebug"
        OR CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    set(SWRZ_BUILD_DEBUG TRUE)
endif()

if(SWRZ_BUILD_WITH_ASAN)
    if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        if(NOT SWRZ_BUILD_DEBUG)
            message(FATAL_ERROR "SWRZ_BUILD_WITH_ASAN MUST be built with Debug or RelWithDebInfo")
        endif()
    else()
        message(FATAL_ERROR "AddressSanitizer was not supported by '${CMAKE_C_COMPILER_ID}'")
    endif()
endif()

function(target_add_asan target_name)
    if(NOT SWRZ_BUILD_WITH_ASAN)
        return()
    endif()

    if (CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${target_name} PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )

        target_link_options(${target_name} PRIVATE
            -fsanitize=address
        )
    endif()
endfunction()

set(SWRZ_SOURCES
    src/swrz_alloc.c
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

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(softrast PRIVATE
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

            -fdiagnostics-show-option
            -fno-common
        >
    )
endif()
target_add_asan(softrast)
