# ViewerModule 라이브러리 생성
add_library(ViewerModuleLib
    astra_wrapper.cpp
    astra_wrapper.h
    viewerModule.c
    viewerModule.h
)

# GLFW 찾기
find_package(glfw3 REQUIRED)

# GLEW 찾기
find_package(GLEW REQUIRED)

# OpenGL 찾기
find_package(OpenGL REQUIRED)

# Astra 라이브러리 포함 및 링크
target_include_directories(ViewerModuleLib PUBLIC
  /astra/x86_64/include
  ${GLFW_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIR}
)

target_link_directories(ViewerModuleLib PUBLIC
  /astra/x86_64/lib
)

# Find link libraries
target_link_libraries(ViewerModuleLib
    astra
    astra_core
    glfw
    GLEW::GLEW
    ${OPENGL_LIBRARIES}
    X11
    stdc++
    pthread
)

# 컴파일 옵션 설정
target_compile_options(ViewerModuleLib PRIVATE -Wall -Wextra)

# C 표준 설정
set_target_properties(ViewerModuleLib PROPERTIES C_STANDARD 11)
