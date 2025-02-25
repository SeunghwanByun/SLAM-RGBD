# CMakeLists.txt of SensorModule

add_library(SensorModuleLib
    # astra_wrapper.cpp
    # astra_wrapper.h
    connectAstra.c
    connectAstra.h
    sensorModule.c
    sensorModule.h
)

# Astra SDK 경로 설정
set(ASTRA_SDK_PATH "/astra/x86_64")

# Astra 라이브러리 포함 및 링크
target_include_directories(SensorModuleLib PUBLIC ${ASTRA_SDK_PATH}/include)
target_link_directories(SensorModuleLib PUBLIC ${ASTRA_SDK_PATH}/lib)

target_link_libraries(SensorModuleLib
    astra
    astra_core
    stdc++
    pthread
)

# Link Astra C API Library
find_library(ASTRA_CAPI_LIB astra SHARED PATHS ${ASTRA_SDK_PATH}/lib)
find_library(ASTRA_CORE_LIB astra_core SHARED PATHS ${ASTRA_SDK_PATH}/lib)

if(ASTRA_CAPI_LIB)
  message(STATUS "Find Astra C API Library : ${ASTRA_CAPI_LIB}")
  message(STATUS "Find Astra Core Library : ${ASTRA_CORE_LIB}")
  target_link_libraries(SensorModuleLib ${ASTRA_CAPI_LIB} ${ASTRA_CORE_LIB})
else()
  message(FATAL_ERROR "Cannot find Astra C API Library.")
endif()


