// main.c

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "SensorModule/sensorModule.h"
#include "ViewerModule/viewerModule.h"
#include "main.h"

// 종료 시그널 핸들링
volatile int keepRunning = 1;

void intHandler(int dummy){
  keepRunning = 0;
}

int main(int argc, char** argv)
{
  printf("Starting Youth\n");
  
  // 종료 시그널 핸들러 등록
  signal(SIGINT, intHandler);

  /* Connect sensor and get sensor data. */
  initSensorModule();
  // pthread_t sensor_thread_id;
  // pthread_create(&sensor_thread_id, NULL, sensorModule, NULL);

  /* Logging system will be here. */
  // pthread_t logging_thread_id;
  // pthread_create(&logging_thread_id, NULL, loggingModule, NULL);

  /* Algorithm module. */
  // pthread_t algorithm_thread_id;
  // pthread_create(&algorithm_thread_id, NULL, algorithmModule, NULL);

  /* Generate 3D Viewer. */
  // glutInit(&argc, argv);
  // glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  // pthread_t viewer_thread_id;
  // pthread_create(&viewer_thread_id, NULL, viewerModule, NULL);
  
  // 약간의 지연 후 뷰어 모듈 초기화 (센서가 준비되도록)
  usleep(500000);

  initViewerModule();

  // 프로그램이 종료될 때까지 대기
  printf("Application running. Press Ctrl+C to exit.\n");
  while(keepRunning){
    sleep(1);
  }

  printf("Shutting down application...\n");

  // 모듈 종료
  stopSensorModule();
  stopViewerModule();
  
  printf("Application terminated successfully\n");
  // pthread_join(sensor_thread_id, NULL);
  // pthread_join(viewer_thread_id, NULL);
  // pthread_join(logging_thread_id, NULL);
  // pthread_join(algorithm_thread_id, NULL);
    
  return 0;
}
