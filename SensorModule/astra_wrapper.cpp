// astra_wrapper.cpp

#include "astra_wrapper.h"
#include <astra/astra.hpp>
#include <iostream>

// For WebGL
#include <nlohmann/json.hpp>

extern "C" {

struct AstraContext {
    astra::StreamSet* streamSet;
    astra::StreamReader* reader;
};

AstraContext_t* InitializeAstraObj()
{
    // Initial Astra SDK
    astra::initialize();
    AstraContext_t* context = new AstraContext_t;
    context->streamSet = new astra::StreamSet();
    context->reader = new astra::StreamReader(context->streamSet->create_reader());
    context->reader->stream<astra::DepthStream>().start();
    context->reader->stream<astra::ColorStream>().start();
    return context;
}

void TerminateAstraObj(AstraContext_t* context)
{
    delete context->reader;
    delete context->streamSet;
    delete context;
    astra::terminate();
}

// For OpenGL - Depth
const int16_t* GetDepthDataAstraOpenGL(AstraContext_t* context, int* width, int* height)
{
    astra::Frame frame = context->reader->get_latest_frame();
    auto depthFrame = frame.get<astra::DepthFrame>();

    if (depthFrame.is_valid())
    {
        *width = depthFrame.width();
        *height = depthFrame.height();
        return depthFrame.data();
    }else{
        std::cout << "Get Depth Frame Failed...!" << std::endl;
    }
    return NULL;
}

// For OpenGL - Color
const uint8_t* GetColorDataAstraOpenGL(AstraContext_t* context, int* width, int* height)
{
    astra::Frame frame = context->reader->get_latest_frame();
    auto colorFrame = frame.get<astra::ColorFrame>();

    if (colorFrame.is_valid())
    {
        *width = colorFrame.width();
        *height = colorFrame.height();
        const astra::RgbPixel* rgbData = colorFrame.data();

        // RgbPixel 포인터를 uint8_t 포인터로 변환
        return reinterpret_cast<const uint8_t*>(rgbData);
    }else{
        std::cout << "Get Color Frame Failed...!" << std::endl;
    } 
    return NULL;
}

// For WebGL - Depth data to JSON format
const char* GetDepthDataAstraWebGL(AstraContext_t* context)
{
    int width, height;
    const int16_t* depthData = GetDepthDataAstraOpenGL(context, &width, &height);
    
    if (depthData)
    {
        nlohmann::json jsonData;
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int index = y * width + x;
                int depthValue = depthData[index];

                if (depthValue > 0)
                {
                    float z = depthValue / 1000.0f;
                    float x_pos = (x - width / 2) * z / 570.3f;
                    float y_pos = (y - height / 2) * z / 570.3f;

                    jsonData.push_back({{"x", x_pos}, {"y", -y_pos}, {"z", -z}});
                }
            }
        }
        
        // 생성된 JSON 데이터를 콘솔에 출력
        std::string jsonString = jsonData.dump();
        std::cout << "Generated JSON Data: " << jsonString << std::endl;
        char* jsonDataCStr = new char[jsonString.length() + 1];
        std::strcpy(jsonDataCStr, jsonString.c_str());
        return jsonDataCStr;  // 웹 서버에서 JSON 문자열로 반환
    }
    return nullptr;
}

// For WebGL - Color data to JSON format
const char* GetColorDataAstraWebGL(AstraContext_t* context)
{
    int width, height;
    const uint8_t* colorData = GetColorDataAstraOpenGL(context, &width, &height);
    
    if (colorData)
    {
        nlohmann::json jsonData;
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int index = (y * width + x) * 3;
                float r = colorData[index] / 255.0f;
                float g = colorData[index + 1] / 255.0f;
                float b = colorData[index + 2] / 255.0f;

                jsonData.push_back({{"r", r}, {"g", g}, {"b", b}});
            }
        }
        // 생성된 JSON 데이터를 콘솔에 출력
        std::string jsonString = jsonData.dump();
        std::cout << "Generated JSON Data: " << jsonString << std::endl;
        char* jsonDataCStr = new char[jsonString.length() + 1];
        std::strcpy(jsonDataCStr, jsonString.c_str());
        return jsonDataCStr;  // 웹 서버에서 JSON 문자열로 반환
    }
    return nullptr;
}
} // extern "C"
