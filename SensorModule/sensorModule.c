#include "sensorModule.h"

AstraContext_t* context;

void sensorModule(void){
    printf("Initializing Astra...\n");
    context = InitializeAstraObj();

    connectAstra(context);
}