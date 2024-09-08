// main.c

#include <stdio.h>
#include <stdlib.h>

#include "main.h"

int main(int argc, char** argv)
{
    viewerModule(argc, argv);
    sensorModule();
    loggingModule();

    return 0;
}