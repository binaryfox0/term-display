cmake_minimum_required(VERSION 3.10)

set(EXTRA "")

function (example_projects name)
 add_executable(${name} ${name}.c example_utils.c)
 target_link_libraries(${name} PRIVATE term-display ${EXTRA})
endfunction()

if(UNIX)
 set(EXTRA "m")
endif()
example_projects(rgb_scrolling)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/stb")
 example_projects(image_display)
 target_include_directories(image_display PRIVATE deps/stb)
endif()
set(EXTRA "") # reset

if(WIN32)
 set(EXTRA "BCrypt")
endif()
example_projects(noise)
set(EXTRA "")

example_projects(multiline_text)
example_projects(kbinput)
example_projects(cube)
