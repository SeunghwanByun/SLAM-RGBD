#ifndef CONNECT_ASTRA_H
#define CONNECT_ASTRA_H

#include "astra_wrapper.h"

typedef struct AstraData{
    float fX;
    float fY;
    float fZ;
    float fR;
    float fG;
    float fB;
}AstraData_t;

AstraData_t connectAstra(AstraContext_t* pstContext, AstraData_t* pstSensorData);

#endif // CONNECT_ASTRA_H