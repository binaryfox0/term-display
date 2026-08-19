cmake_minimum_required(VERSION 3.15)
project(lodepng)
add_library(lodepng src/lodepng.c)
target_include_directories(lodepng PUBLIC include)
