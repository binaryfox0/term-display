cmake_minimum_required(VERSION 3.10)

project(term-display C)

set(CMAKE_C_STANDARD 99)

file(GLOB sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.c)

# Option for CGLM
option(BUILD_WITH_CGLM "Use glm for more optimized math operations" OFF)
if(BUILD_WITH_CGLM)
    find_package(cglm QUIET)
    if(cglm_FOUND)
        message(STATUS "Found cglm from current machine")
    else()
        message(STATUS "cglm not found from current machine, downloading automatically...")
        include(FetchContent)
        FetchContent_Declare(
            cglm
            GIT_REPOSITORY https://github.com/recp/cglm.git
            GIT_TAG master # Replace 'master' with a stable version if needed
        )
        FetchContent_MakeAvailable(cglm)
    endif()
    target_include_directories(term-display PUBLIC ${cglm_SOURCE_DIR}/include)
    target_link_libraries(term-display PRIVATE cglm m)
    target_compile_definitions(term-display PRIVATE BUILD_WITH_CGLM)
endif()

# Create the term-display library
add_library(term-display ${sources})
target_include_directories(term-display PUBLIC include)

# Add example subdirectory
add_subdirectory(example)
