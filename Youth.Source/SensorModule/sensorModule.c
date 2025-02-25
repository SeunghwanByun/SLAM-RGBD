// #include "sensorModule.h"

// // AstraContext_t* context;

// void* sensorModule(void* id){
//     // printf("Initializing Astra...\n");
//     // context = InitializeAstraObj();

//     while(1){
//     printf("Test\n");
//         connectAstra(context);
//     }
// }
// Connect Astra
// Astra Orbbec RGB-D 카메라 데이터 수집 스레드 구현 (C 언어 버전)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <astra/capi/astra.h>

#define MAX_FRAME_QUEUE_SIZE 10

// RGB-D 프레임 구조체 정의
typedef struct {
    uint8_t* colorData;
    uint16_t* depthData;
    int width;
    int height;
} RGBDFrame;

// Thread Safe Queue
pthread_mutex_t frameQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t frameQueueCond = PTHREAD_COND_INITIALIZER;
RGBDFrame frameQueue[MAX_FRAME_QUEUE_SIZE];
int front = 0, rear = 0;

///////////////////
// Implement Queue

// Check Empty Queue
int is_queue_empty() {
    return front == rear;
}

// Check Full Queue
int is_queue_full() {
    return ((rear + 1) % MAX_FRAME_QUEUE_SIZE) == front;
}

// Enqueue Frame
void enqueue_frame(RGBDFrame frame) {
    pthread_mutex_lock(&frameQueueMutex);
    while (is_queue_full()) {
        pthread_cond_wait(&frameQueueCond, &frameQueueMutex);
    }
    frameQueue[rear] = frame;
    rear = (rear + 1) % MAX_FRAME_QUEUE_SIZE;
    pthread_cond_signal(&frameQueueCond);
    pthread_mutex_unlock(&frameQueueMutex);
}

// Dequeue Frame
int dequeue_frame(RGBDFrame* frame) {
    pthread_mutex_lock(&frameQueueMutex);
    while (is_queue_empty()) {
        pthread_cond_wait(&frameQueueCond, &frameQueueMutex);
    }
    *frame = frameQueue[front];
    front = (front + 1) % MAX_FRAME_QUEUE_SIZE;
    pthread_cond_signal(&frameQueueCond);
    pthread_mutex_unlock(&frameQueueMutex);
    return 1;
}

// Free Memory
void free_frame(RGBDFrame* frame) {
    if (frame->colorData) free(frame->colorData);
    if (frame->depthData) free(frame->depthData);
}

// Connect Astra Function
// 센서 스레드 함수
void* sensorModule(void* id) {
    astra_initialize();

    astra_streamsetconnection_t streamSet;
    astra_reader_t reader;
    astra_status_t status;

    status = astra_reader_create("device/default", &reader);
    if (status != ASTRA_STATUS_SUCCESS) {
        fprintf(stderr, "[Error] Failed to create Astra reader.\n");
        pthread_exit(NULL);
    }

    astra_depthstream_t depthStream;
    astra_colorstream_t colorStream;
    astra_reader_get_depthstream(reader, &depthStream);
    astra_reader_get_colorstream(reader, &colorStream);

    astra_stream_start(depthStream);
    astra_stream_start(colorStream);

    while (1) {
        astra_reader_frame_t frame;
        status = astra_reader_open_frame(reader, 0, &frame);
        if (status != ASTRA_STATUS_SUCCESS) {
            fprintf(stderr, "[Error] Failed to open Astra frame.\n");
            continue;
        }

        astra_depthframe_t depthFrame;
        astra_colorframe_t colorFrame;

        astra_frame_get_depthframe(frame, &depthFrame);
        astra_frame_get_colorframe(frame, &colorFrame);

        if (!depthFrame || !colorFrame) {
            fprintf(stderr, "[Warning] Invalid frame received.\n");
            astra_reader_close_frame(&frame);
            continue;
        }

        int width = 0, height = 0;

        const uint16_t* depthData;
        const uint8_t* colorData;
        astra_depthframe_get_data_ptr(depthFrame, &depthData, &width);
        astra_colorframe_get_data_ptr(colorFrame, &colorData, &width);

        RGBDFrame rgbdFrame;
        rgbdFrame.width = width;
        rgbdFrame.height = height;
        rgbdFrame.colorData = (uint8_t*)malloc(width * height * 3);
        rgbdFrame.depthData = (uint16_t*)malloc(width * height * sizeof(uint16_t));

        if (!rgbdFrame.colorData || !rgbdFrame.depthData) {
            fprintf(stderr, "[Error] Memory allocation failed.\n");
            free_frame(&rgbdFrame);
            astra_reader_close_frame(&frame);
            continue;
        }

        memcpy(rgbdFrame.colorData, colorData, width * height * 3);
        memcpy(rgbdFrame.depthData, depthData, width * height * sizeof(uint16_t));

        enqueue_frame(rgbdFrame);
        printf("[Sensor Thread] Frame captured: %dx%d\n", width, height);

        astra_reader_close_frame(&frame);
    }

    astra_terminate();
    pthread_exit(NULL);
}

pthread_t sensor_thread_id;
void initSensorModule(){
  pthread_create(&sensor_thread_id, NULL, sensorModule, NULL);
}
