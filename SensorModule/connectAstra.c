#include "connectAstra.h"

AstraData_t connectAstra(AstraContext_t* pstContext, AstraData_t* ){
    AstraData_t stAstraData = {0};

    int width, height;
    const int16_t* depthData = GetDepthDataAstraOpenGL(pstContext, &width, &height);
    const uint8_t* colorData = GetColorDataAstraOpenGL(pstContext, &width, &height);

    if(depthData && colorData){
        for(int y = 0; y < height; ++y){
            for(int x = 0; x < width; ++x){
                int index = y * width + x;
                int depthValue = depthData[index];

                if(depthValue > 0){
                    // Calculate 3D Coordinate (Using Simple Camera Model)
                    float z = depthValue / 1000.0f; // from mm to m
                    float x_pos = (x - width / 2) * z / 570.3f; // 570.3f is focal distance of Astra camera
                    float y_pos = (y - height / 2) * z / 570.3f;

                    // Set color using color data (RGB order)
                    int colorIndex = index * 3; // RGB consist of 3 values.
                    float r = colorData[colorIndex] / 255.0f;
                    float g = colorData[colorIndex + 1] / 255.0f;
                    float b = colorData[colorIndex + 2] / 255.0f;
                }
            }
        }
    }
}