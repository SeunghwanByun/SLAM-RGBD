add_library(ViewerModuleLib
    viewerModule.c
    viewerModule.h
)

# Find link libraries
target_link_libraries(ViewerModuleLib
    X11
    stdc++
    pthread
)