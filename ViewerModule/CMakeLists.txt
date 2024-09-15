add_library(ViewerModuleLib
    viewer.c
    viewer.h
)

# Find link libraries
target_link_libraries(ViewerModuleLib
    X11
    stdc++
    pthread
)