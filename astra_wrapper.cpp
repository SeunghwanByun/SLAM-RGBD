// astra_wrapper.cpp

#include "astra_wrapper.h"
#include <astra/astra.hpp>
#include <iostream>

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

const int16_t* GetDepthDataAstra(AstraContext_t* context, int* width, int* height)
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

const uint8_t* GetColorDataAstra(AstraContext_t* context, int* width, int* height)
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

} // extern "C"
