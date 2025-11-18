cmake_minimum_required(VERSION 3.10)

if(UNIX)
    project(kstrokseq)
    add_executable(kstrokseq kstrokseq.c)
else()
    message(STATUS "Not unix-like OS, skipped building kstrokseq")
endif()
