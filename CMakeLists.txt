cmake_minimum_required(VERSION 3.16)

project(ProjectYouth)

# Set source file
set(SOURCES
    main.c
)

add_executable(Youth ${SOURCES})

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)

target_link_libraries(Youth
OpenGL::GL
GLEW::GLEW
GLUT::GLUT
)

