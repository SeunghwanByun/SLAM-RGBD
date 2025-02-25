#ifndef LOGGING_MODULE_H
#define LOGGING_MODULE_h

// #include "../main.h"

#include <pthread.h>
#include <stdio.h>
#include <stdint.h>

void receive_data();
void* loggingModule(void* id);

#endif // LOGGING_MODULE_H
