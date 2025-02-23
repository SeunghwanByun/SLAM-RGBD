# CMakeLists.txt of SensorModule

add_library(SensorModuleLib
    # astra_wrapper.cpp
    # astra_wrapper.h
    connectAstra.c
    connectAstra.h
    sensorModule.c
    sensorModule.h
)

# Astra 라이브러리 포함 및 링크
target_include_directories(SensorModuleLib PUBLIC /astra/x86_64/include)
target_link_directories(SensorModuleLib PUBLIC /astra/x86_64/lib)

target_link_libraries(SensorModuleLib
    astra
    astra_core
    stdc++
    pthread
)
