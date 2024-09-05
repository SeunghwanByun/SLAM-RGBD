// main.c

#include <stdio.h>
#include <stdlib.h>

#include "LoggingModule/loggingModule.h"
#include "ViewerModule/viewer.h"

int main(int argc, char** argv)
{
    viewerModule(argc, argv);

    loggingModule();

    return 0;
}