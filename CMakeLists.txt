cmake_minimum_required(VERSION 3.10)

project(term-display C)

set(CMAKE_CXX_STANDARD 90)

file(GLOB sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.c)
if(WIN32)
 list(REMOVE_ITEM sources src/term_priv_linux.c)
elseif(UNIX)
 list(REMOVE_ITEM sources src/term_priv_win32.c)
else()
 message(FATAL_ERROR "Unsupported OS found. Abort.")
endif()

add_library(term-display ${sources})
target_include_directories(term-display PUBLIC include)

add_subdirectory(example)
