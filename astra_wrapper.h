// astra_wrapper.h

#ifndef ASTRA_WRAPPER_H
#define ASTRA_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct MyAstraContext MyAstraContext;

MyAstraContext* my_astra_initialize();
void my_astra_terminate(MyAstraContext* context);
const int16_t* my_astra_get_depth_data(MyAstraContext* context, int* width, int* height);
const uint8_t* my_astra_get_color_data(MyAstraContext* context, int* width, int* height);

#ifdef __cplusplus
}
#endif

#endif // ASTRA_WRAPPER_H
