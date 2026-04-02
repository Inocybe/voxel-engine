cmake_minimum_required(VERSION 3.10)


if(WIN32 AND NOT DEFINED CMAKE_GENERATOR)
    set(CMAKE_GENERATOR "MinGW Makefiles" CACHE STRING "CMake generator" FORCE)
endif()


if(NOT CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER clang)
endif()
if(NOT CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER clang++)
endif()


project(LearnOpenGL)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW
add_subdirectory(external/glfw)
include_directories(external/glfw/include)

# GLAD
add_library(glad src/glad.c)
target_include_directories(glad PUBLIC include)

# PERLIN
add_library(perlin INTERFACE) # header only library so is an interface
target_sources(perlin INTERFACE
    ${PROJECT_SOURCE_DIR}/include/perlin/seamless_noise_generator_2d.h
        ${PROJECT_SOURCE_DIR}/include/perlin/fractal_noise_generator.h
        ${PROJECT_SOURCE_DIR}/include/perlin/perlin_noise_generator.h
        ${PROJECT_SOURCE_DIR}/include/perlin/point.h
        ${PROJECT_SOURCE_DIR}/include/perlin/vector.h
        ${PROJECT_SOURCE_DIR}/include/perlin/math.h)
target_include_directories(perlin INTERFACE include)

# OTHER
add_library(shader src/shader.cpp)
target_include_directories(shader PUBLIC include include/glm)

add_library(texture src/texture.cpp)
target_include_directories(texture PUBLIC include include/glm)

add_library(engine src/engine.cpp)
target_include_directories(engine PUBLIC include include/glm)

add_library(chunk src/chunk.cpp)
target_include_directories(chunk PUBLIC include include/glm)


add_library(world src/world.cpp)
target_include_directories(world PUBLIC include include/glm)

#include_directories(external/glfw/include include include/glm)

add_executable(main src/main.cpp)
target_link_libraries(main
    engine
    shader
    texture
    chunk
    world
    glfw
    glad
    ${CMAKE_DL_LIBS})


# TO RUN: cmake .. --preset mingw-clang