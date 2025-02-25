#include "sensorModule.h"
#include "astra_wrapper.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

// Message Queue Name
#define MQ_NAME "/sensor_viewer_queue"
#define MAX_MSG_SIZE 8192 // 메세지 최대 크기, 필요에 따라 조정 가능

// Sensor Context
AstraContext_t* sensorContext = NULL;

// Message Queue Handle
mqd_t mqSend = -1;

#define MSG_TYPE_METADATA 1
#define MSG_TYPE_DEPTH_DATA 2
#define MSG_TYPE_COLOR_DATA 3

// Sensor Data Structure
typedef struct {
  int msgType; // 메세지 타입
  int width;  // 이미지 너비
  int height; // 이미지 높이
  int chunkIndex; // 청크 인덱스
  int totalChunks; // 전체 청크 수
  int dataSize; // 이 메세지의 데이터 크기
  int frameId; // 프레임 식별자
}MessageHeader;

// Thread Control Variable;
static int sensorIsRunning = 1;

void* sensorLoop(void* arg){
  printf("Sensor thread started...\n");

  // Message Queue Feature Setting
  struct mq_attr attr;
  attr.mq_flags = 0;
  attr.mq_maxmsg = 10;
  attr.mq_msgsize = MAX_MSG_SIZE;
  attr.mq_curmsgs = 0;

  // Generate Message Queue
  mqSend = mq_open(MQ_NAME, O_CREAT | O_WRONLY, 0644, &attr);
  if(mqSend == (mqd_t) - 1){
    perror("mq_open send");
    return NULL;
  }

  // Initialize Sensor
  printf("Initializing Astra sensor ...\n");
  sensorContext = InitializeAstraObj();
  
  if(!sensorContext){
    printf("Failed to initialize Astra sensor\n");
    mq_close(mqSend);
    mq_unlink(MQ_NAME);
    return NULL;
  }

  // 데이터 청크를 위한 버퍼 할당
  char* msgBuffer = (char*)malloc(MAX_MSG_SIZE);
  if(!msgBuffer){
    perror("malloc message buffer");
    TerminateAstraObj(sensorContext);
    mq_close(mqSend);
    mq_unlink(MQ_NAME);
    return NULL;
  }

  int frameId = 0;

  // Main Loop
  while(sensorIsRunning){
    int width, height;

    // Get Data from Sensor
    const int16_t* depthData = GetDepthDataAstraOpenGL(sensorContext, &width, &height);
    const uint8_t* colorData = GetColorDataAstraOpenGL(sensorContext, &width, &height);

    if(depthData && colorData){
      frameId++;

      // 1. 메타데이터 메세지 전송
      MessageHeader* header = (MessageHeader*)msgBuffer;
      header->msgType = MSG_TYPE_METADATA;
      header->width = width;
      header->height = height;
      header->chunkIndex = 0;
      header->totalChunks = 0; // 메타데이터에는 의미 없음
      header->dataSize = 0;
      header->frameId = frameId;

      if(mq_send(mqSend, msgBuffer, sizeof(MessageHeader), 0) == -1){
        perror("mq_send metadata");
        continue;
      }

      // 2. 깊이 데이터 청크 전송
      int depthDataSize = width * height * sizeof(int16_t);
      int maxDataPerMsg = MAX_MSG_SIZE - sizeof(MessageHeader);
      int totalDepthChunks = (depthDataSize + maxDataPerMsg - 1) / maxDataPerMsg;

      for(int i = 0; i < totalDepthChunks; i++){
        header->msgType = MSG_TYPE_DEPTH_DATA;
        header->chunkIndex = i;
        header->totalChunks = totalDepthChunks;
        header->frameId = frameId;

        int offset = i * maxDataPerMsg;
        int chunkSize = (i == totalDepthChunks - 1) ? (depthDataSize - offset) : maxDataPerMsg;

        header->dataSize = chunkSize;

        // 데이터 복사
        memcpy(msgBuffer + sizeof(MessageHeader), ((char*)depthData) + offset, chunkSize);

        if(mq_send(mqSend, msgBuffer, sizeof(MessageHeader) + chunkSize, 0) == -1){
          perror("mq_send depth chunk");
          break;
        }
      }

      // 3. 색상 데이터 청크 전송
      int colorDataSize = width * height * 3 * sizeof(uint8_t);
      int totalColorChunks = (colorDataSize + maxDataPerMsg - 1) / maxDataPerMsg;

      for(int i = 0; i < totalColorChunks; i++){
        header->msgType = MSG_TYPE_COLOR_DATA;
        header->chunkIndex = i;
        header->totalChunks = totalColorChunks;
        header->frameId = frameId;

        int offset = i * maxDataPerMsg;
        int chunkSize = (i == totalColorChunks - 1) ? (colorDataSize - offset) : maxDataPerMsg;

        header->dataSize = chunkSize;

        // 데이터 복사
        memcpy(msgBuffer + sizeof(MessageHeader), ((char*)colorData) + offset, chunkSize);

        if(mq_send(mqSend, msgBuffer, sizeof(MessageHeader) + chunkSize, 0) == -1){
          perror("mq_send color chunk");
          break;
        }
      }
    }
    //   // Prepare data for sending to Message Queue
    //   int depthDataSize = width * height * sizeof(int16_t);
    //   int colorDataSize = width * height * 3 * sizeof(uint8_t);
    //   int totalDataSize = sizeof(SensorDataMsg) + depthDataSize + colorDataSize;

    //   if(totalDataSize > MAX_MSG_SIZE){
    //     printf("Warning: Data size exceeds max message size. Consider reducing resolution or increasing MAX_MSG_SIZE.\n");
    //     // 필요한 경우 데이터를 여러 메시지로 분할하거나 크기 조정
    //     continue;
    //   }

    //   // Generate Message Buffer.
    //   char* msgBuffer = (char*)malloc(totalDataSize);
    //   if(!msgBuffer){
    //     printf("Memory allocation failed\n");
    //     continue;
    //   }

    //   // Header Information Setting
    //   SensorDataMsg* header = (SensorDataMsg*)msgBuffer;
    //   header->width = width;
    //   header->height = height;
    //   header->dataSize = depthDataSize + colorDataSize;

    //   // Copy Data
    //   char* dataPtr = msgBuffer + sizeof(SensorDataMsg);
    //   memcpy(dataPtr, depthData, depthDataSize);
    //   dataPtr += depthDataSize;
    //   memcpy(dataPtr, colorData, colorDataSize);

    //   // Send Data to Message Queue
    //   if(mq_send(mqSend, msgBuffer, totalDataSize, 0) == -1){
    //     perror("mq_send");
    //   }

    //   free(msgBuffer);
    // }

    // Data Collection Speed Control (About 30 FPS)
    usleep(33333);
  }

  // 메모리 해제
  free(msgBuffer);

  // Terminate Sensor
  TerminateAstraObj(sensorContext);

  // Close Message Queue
  mq_close(mqSend);

  printf("Sensor thread termination\n");
  return NULL;
}

pthread_t sensor_thread_id;

void initSensorModule(){
  sensorIsRunning = 1;
  pthread_create(&sensor_thread_id, NULL, sensorLoop, NULL);
}

void stopSensorModule(){
  sensorIsRunning = 0;
  pthread_join(sensor_thread_id, NULL);
  mq_unlink(MQ_NAME); // Eliminate Message Queue
}
