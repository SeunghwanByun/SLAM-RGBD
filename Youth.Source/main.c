// main.c

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "SensorModule/sensorModule.h"
#include "ViewerModule/viewerModule.h"

// 종료 시그널 핸들링
volatile int keepRunning = 1;

void intHandler(int dummy){
  keepRunning = 0;
  printf("Received shutdown signal. Closing application...\n");
}

// 뷰어에서 호출될 종료 콜백
void exitHandler(void){
  keepRunning = 0;
  printf("Received shutdown signal from viewer. Closing application...\n");
}

int main(int argc, char** argv)
{
  printf("Starting Youth\n");
  
  // 종료 시그널 핸들러 등록
  signal(SIGINT, intHandler);

  /* Connect sensor and get sensor data. */
  initSensorModule();

  /* Logging system will be here. */
  // pthread_t logging_thread_id;
  // pthread_create(&logging_thread_id, NULL, loggingModule, NULL);

  /* Algorithm module. */
  // pthread_t algorithm_thread_id;
  // pthread_create(&algorithm_thread_id, NULL, algorithmModule, NULL);

  /* Generate 3D Viewer. */
  // 약간의 지연 후 뷰어 모듈 초기화 (센서가 준비되도록)
  usleep(500000);
  initViewerModule();

  // 종료 콜백 설정
  setExitCallback(exitHandler);
  // 프로그램이 종료될 때까지 대기
  printf("Application running. Press Ctrl+C or close the window to exit.\n");
  while(keepRunning){
    sleep(1);

    // 정기적으로 센서와 뷰어가 살아있는지 확인
    if(!isSensorModuleRunning() || !isViewerModuleRunning()){
      keepRunning = 0;
      printf("Module terminated unexpectedly. Shutting down...\n");
    }
  }

  printf("Shutting down application...\n");

  // // 모듈 종료 타임아웃 설정
  // struct timespec timeout;
  // clock_gettime(CLOCK_REALTIME, &timeout);
  // timeout.tv_sec += 3; // 3초 타임 아웃
  //
  // // 모듈 정지 요청
  // requestStopViewerModule();
  // requestStopSensorModule();
  
  // // 스레드 종료 대기 (타임아웃 적용)
  // if(pthread_timedjoin_np(viewer_thread_id, NULL, &timeout) != 0){
  //   printf("Viewer module did not terminate gracefully, forsing shutdown...\n");
  //   // 강제 종료 로직 (예: pthread_cancel);
  // }
  //
  // if(pthread_timedjoin_np(sensor_thread_id, NULL, &timeout) != 0){
  //   printf("Sensor module did not terminate gracefully, forcing shutdown...\n");
  //   // 강제 종료 로직
  // }

  // 모듈 종료
  // stopSensorModule();
  // stopViewerModule();
  
  printf("Application terminated successfully\n");
  // pthread_join(logging_thread_id, NULL);
  // pthread_join(algorithm_thread_id, NULL);
    
  return 0;
}
