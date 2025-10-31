cmake_minimum_required(VERSION 3.10)

project(Examples)

        message(STATUS "AddressSanitizer supported — enabling for Debug build (C compiler)")
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)

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

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/stb/stb_image.h")
    example_projects(image_display)
    target_include_directories(image_display PRIVATE deps/stb/)
endif()
set(EXTRA "") # reset

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
