add_library(ViewerModuleLib
    astra_wrapper.cpp
    astra_wrapper.h
    viewerModule.c
    viewerModule.h
)

# Astra 라이브러리 포함 및 링크
target_include_directories(ViewerModuleLib PUBLIC /astra/x86_64/include)
target_link_directories(ViewerModuleLib PUBLIC /astra/x86_64/lib)

# Find link libraries
target_link_libraries(ViewerModuleLib
    astra
    astra_core
    X11
    stdc++
    pthread
)
