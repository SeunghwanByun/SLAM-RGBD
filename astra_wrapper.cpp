// astra_wrapper.cpp

#include "astra_wrapper.h"
#include <astra/astra.hpp>
#include <iostream>

extern "C" {

struct MyAstraContext {
    astra::StreamSet* streamSet;
    astra::StreamReader* reader;
};

MyAstraContext* my_astra_initialize()
{
    astra::initialize();
    MyAstraContext* context = new MyAstraContext;
    context->streamSet = new astra::StreamSet();
    context->reader = new astra::StreamReader(context->streamSet->create_reader());
    context->reader->stream<astra::DepthStream>().start();
    return context;
}

void my_astra_terminate(MyAstraContext* context)
{
    delete context->reader;
    delete context->streamSet;
    delete context;
    astra::terminate();
}

const int16_t* my_astra_get_depth_data(MyAstraContext* context, int* width, int* height)
{
    astra::Frame frame = context->reader->get_latest_frame();
    auto depthFrame = frame.get<astra::DepthFrame>();

    if (depthFrame.is_valid())
    {
        *width = depthFrame.width();
        *height = depthFrame.height();
        return depthFrame.data();
    }
    return NULL;
}

const uint8_t* my_astra_get_color_data(MyAstraContext* context, int* width, int* height)
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
    }
    return NULL;
}

} // extern "C"
