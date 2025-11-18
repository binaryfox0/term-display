cmake_minimum_required(VERSION 3.10)

project(example)

option(TD_EXAMPLE_ASAN "Enable AddressSanitizer in examples")

if(TD_EXAMPLE_ASAN)
    if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "AddressSanitizer support was found")
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    else()
        message(WARNING "AddressSanitizer support was not found")
    endif()
endif()

set(EXTRA "")
add_subdirectory(deps/aparse)

function(example_projects name)
    add_executable(${name} ${name}.c example_utils.c)
    target_link_libraries(${name} PRIVATE term-display ${EXTRA} aparse)
endfunction()

if(UNIX)
    set(EXTRA "m")
endif()
example_projects(rgb_scrolling)

example_projects(image_display)
target_include_directories(image_display PRIVATE deps/include/)

if(WIN32)
    set(EXTRA "BCrypt")
endif()
example_projects(noise)
set(EXTRA "")

example_projects(multiline_text)
target_link_libraries(multiline_text PRIVATE m)

set(EXTRA "cglm")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/cglm/include")
    example_projects(cube)
    add_subdirectory(deps/cglm)
    target_link_libraries(cube PRIVATE cglm m)
endif()
