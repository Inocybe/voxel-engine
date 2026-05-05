cmake_minimum_required(VERSION 3.20)

project(VoxelEngine)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

# GLFW
add_subdirectory(external/glfw)
include_directories(external/glfw/include)

# GLAD
add_library(glad STATIC src/glad.c)
target_include_directories(glad PUBLIC include)

# GLM
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE include/glm)

# SIMPLEX
add_library(simplex INTERFACE) # header only library so is an interface
target_sources(simplex INTERFACE 
    ${PROJECT_SOURCE_DIR}/include/simplex/simplex.h)
target_include_directories(simplex INTERFACE include/simplex)

# ENGINE LIBRARIES
add_library(shader src/shader.cpp)
target_include_directories(shader PUBLIC include)
target_link_libraries(shader PUBLIC glm glad)

add_library(texture src/texture.cpp)
target_include_directories(texture PUBLIC include)
target_link_libraries(texture PUBLIC glm glad)

add_library(chunk src/chunk.cpp)
target_include_directories(chunk PUBLIC include)
target_link_libraries(chunk PUBLIC glm)

add_library(heightmap src/heightmap.cpp)
target_include_directories(heightmap PUBLIC include)
target_link_libraries(heightmap PUBLIC glm simplex)

add_library(thread_pool src/thread_pool.cpp)
target_include_directories(thread_pool PUBLIC include)

add_library(player src/player.cpp)
target_include_directories(player PUBLIC include)
target_link_libraries(player PUBLIC glm)

add_library(world src/world.cpp)
target_include_directories(world PUBLIC include)
target_link_libraries(world PUBLIC glm chunk heightmap player thread_pool)

add_library(engine src/engine.cpp)
target_include_directories(engine PUBLIC include)
target_link_libraries(engine PUBLIC glm glad glfw shader texture world player thread_pool)

#include_directories(external/glfw/include include include/glm)

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE engine ${CMAKE_DL_LIBS})


# TO RUN: cmake .. --preset mingw-clang

# or cmake -B build --preset mingw-clang && cmake --build build