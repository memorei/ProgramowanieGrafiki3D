cmake_minimum_required(VERSION 3.15)
project(Engine CXX)

add_compile_definitions(PROJECT_NAME="${PROJECT_NAME}" PROJECT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_library(Engine_Meshes
    Mesh.h Mesh.cpp
    Material.h Material.cpp
)

target_include_directories(Engine_Meshes PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)