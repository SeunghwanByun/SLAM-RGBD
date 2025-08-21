cmake_minimum_required(VERSION 3.16)
project(Youth LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SR_BUILD_EXAMPLES "Build examples (viewer app)" ON)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(src)

message(STATUS "=== Youth Configuration ===")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "OpenGL: ${OPENGL_gl_LIBRARY}")
message(STATUS "GLEW: ${GLEW_LIBRARIES}")
message(STATUS "GLFW: ${glfw3_LIBRARIES}")