cmake_minimum_required(VERSION 3.16)

project(ProjectYouth)

# Set Debugging Symbols
# set(CMAKE_BUILD_TYPE Debug)

if(WIN32)
    message("Building on Windows")
    # Windows-specific settings
elseif(UNIX AND NOT APPLE)
    message("Building on Linux")
elseif(APPLE)
    message("Building on macOS")
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Sub directory addition
add_subdirectory(AlgorithmModule)
add_subdirectory(LoggingModule)
add_subdirectory(SensorModule)
add_subdirectory(ViewerModule)

# Find Library
# OpenGL
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)

# Generate execution file
add_executable(Youth main.c main.h)

# Find link libraries
target_link_libraries(Youth
    AlgorithmModuleLib
    LoggingModuleLib
    SensorModuleLib
    ViewerModuleLib
    OpenGL::GL
    GLEW::GLEW
    GLUT::GLUT
    GLU
    stdc++
    pthread
)
