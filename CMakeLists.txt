cmake_minimum_required(VERSION 3.16)

project(ProjectYouth)

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

# Set source file
set(SOURCES
    main.c
    astra_wrapper.cpp
    viewer.c
)

# Find Library
# OpenGL
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)

# Astra
# include_directories(${ASTRA_SDK_INCLUDE})
# link_directories(${ASTRA_SDK_LIB})
include_directories(/astra/x86_64/include)
link_directories(/astra/x86_64/lib)

# Generate execution file
add_executable(Youth ${SOURCES})

# Find link libraries
target_link_libraries(Youth
    OpenGL::GL
    GLEW::GLEW
    GLUT::GLUT
    GLU
    X11
    astra
    astra_core
    stdc++
    pthread
)