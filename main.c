// main.c

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "main.h"

int main(int argc, char** argv)
{
    /* Connect sensor and get sensor data. */
    pthread_t sensor_thread_id;
    pthread_create(&sensor_thread_id, NULL, sensorModule, NULL);

    /* Generate 3D Viewer. */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    pthread_t viewer_thread_id;
    pthread_create(&viewer_thread_id, NULL, viewerModule, NULL);

    /* Logging system will be here. */
    pthread_t logging_thread_id;
    pthread_create(&logging_thread_id, NULL, loggingModule, NULL);

    /* Algorithm module. */
    pthread_t algorithm_thread_id;
    pthread_create(&algorithm_thread_id, NULL, algorithmModule, NULL);

    pthread_join(sensor_thread_id, NULL);
    pthread_join(viewer_thread_id, NULL);
    pthread_join(logging_thread_id, NULL);
    pthread_join(algorithm_thread_id, NULL);
    
    return 0;
}