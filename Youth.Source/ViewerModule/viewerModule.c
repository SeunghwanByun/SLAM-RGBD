#include "viewerModule.h"
#include <pthread.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

// Monitor Resolution Information
#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <X11/Xlib.h>
#endif

// Message Queue Name
#define MQ_NAME "/sensor_viewer_queue"
#define MAX_MSG_SIZE 8192 // 메세지 최대 크기, 필요에 따라 조정 가능

// 메세지 타입 정의
#define MSG_TYPE_METADATA 1
#define MSG_TYPE_DEPTH_DATA 2
#define MSG_TYPE_COLOR_DATA 3

// viewerModule.c 에 구현 추가
static ExitCallbackFunc exitCallback = NULL;

// Data Structure for receiving from Message Queue
typedef struct{
  int msgType; // 메세지 타입
  int width;  // 이미지 너비
  int height; // 이미지 높이
  int chunkIndex; // 청크 인덱스
  int totalChunks; // 전체 청크 수
  int dataSize; // 이 메세제의 데이터 크기
  int frameId;  // 프레임 식별자
  // 실제 데이터는 메세지 큐에서 별도로 수신
} MessageHeader;

// Viewer Status Variables
GLuint depthTexture;
GLuint colorTexture;

float angleX = 0.0f;
float angleY = 0.0f;
float distance = 2.0f;
int lastMouseX = 0, lastMouseY = 0;
float zoom = 1.0f;
int isDragging = 0;

// Message Queue Handle
mqd_t mqReceive = -1;

// GLFW Window
GLFWwindow* window;

// Data Buffer
int16_t* depthDataBuffer = NULL;
uint8_t* colorDataBuffer = NULL;
int currentWidth = 0;
int currentHeight = 0;
int hasNewData = 0;
int currentFrameId = 0;

// Thread Control Variable
int viewerIsRunning = 1;
pthread_mutex_t dataMutex = PTHREAD_MUTEX_INITIALIZER;

// 청크 수신 상태 구조체
typedef struct{
  int frameId;
  int width;
  int height;
  int receivedDepthChunks;
  int totalDepthChunks;
  int receivedColorChunks;
  int totalColorChunks;
  char* depthDataBuffer;
  char* colorDataBuffer;
  int isComplete;
} FrameReceiveStatus;

FrameReceiveStatus receiveStatus;

// Initialize OpenGL
void initOpenGL()
{
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Frame Receive Status Initialization
void initFrameReceiveStatus(FrameReceiveStatus* status, int frameId, int width, int height){
  status->frameId = frameId;
  status->width = width;
  status->height = height;
  status->receivedDepthChunks = 0;
  status->totalDepthChunks = 0;
  status->receivedColorChunks = 0;
  status->totalColorChunks = 0;

  // 이전 버퍼 해제
  if(status->depthDataBuffer){
    free(status->depthDataBuffer);
  }

  if(status->colorDataBuffer){
    free(status->colorDataBuffer);
  }

  // Allocate new buffer
  int depthDataSize = width * height * sizeof(int16_t);
  int colorDataSize = width * height * 3 * sizeof(uint8_t);

  status->depthDataBuffer = (char*)malloc(depthDataSize);
  status->colorDataBuffer = (char*)malloc(colorDataSize);

  if(!status->depthDataBuffer || !status->colorDataBuffer){
    perror("malloc frame buffers");
    if(status->depthDataBuffer) free(status->depthDataBuffer);
    if(status->colorDataBuffer) free(status->colorDataBuffer);
    status->depthDataBuffer = NULL;
    status->colorDataBuffer = NULL;
  }
}

// Data Receiving Thread
void* dataReceiveThread(void* arg){
  printf("Data receive thread started...\n");

  // Message Queue Feature Setting
  struct mq_attr attr;
  attr.mq_flags = 0;
  attr.mq_maxmsg = 10;
  attr.mq_msgsize = MAX_MSG_SIZE;
  attr.mq_curmsgs = 0;

  // Open Message Queue
  mqReceive = mq_open(MQ_NAME, O_RDONLY, 0644, &attr);
  if(mqReceive == (mqd_t) - 1){
    perror("mq_open receive");
    return NULL;
  }

  // Message Buffer
  char* msgBuffer = (char*)malloc(MAX_MSG_SIZE);
  if(!msgBuffer){
    perror("malloc");
    mq_close(mqReceive);
    return NULL;
  }

  // Initialize Receive status
  memset(&receiveStatus, 0, sizeof(FrameReceiveStatus));

  // Main Loop
  while(viewerIsRunning){
    ssize_t byteRead = mq_receive(mqReceive, msgBuffer, MAX_MSG_SIZE, NULL);

    if(byteRead > 0){
      MessageHeader* header = (MessageHeader*)msgBuffer;

      // 메세지 타입에 따라 처리
      switch(header->msgType){
        case MSG_TYPE_METADATA:
          // 새 프레임 정보 수신
          pthread_mutex_lock(&dataMutex);
          initFrameReceiveStatus(&receiveStatus, header->frameId, header->width, header->height);
          pthread_mutex_unlock(&dataMutex);
          break;

        case MSG_TYPE_DEPTH_DATA:
          // 깊이 데이터 청크 수신
          pthread_mutex_lock(&dataMutex);

          if(header->frameId == receiveStatus.frameId && receiveStatus.depthDataBuffer){
            int maxDataPerMsg = MAX_MSG_SIZE - sizeof(MessageHeader);
            int offset = header->chunkIndex * maxDataPerMsg;

            memcpy(receiveStatus.depthDataBuffer + offset, msgBuffer + sizeof(MessageHeader), header->dataSize);

            receiveStatus.receivedDepthChunks++;
            receiveStatus.totalDepthChunks = header->totalChunks;
          }

          pthread_mutex_unlock(&dataMutex);
          break;

        case MSG_TYPE_COLOR_DATA:
          // 색상 데이터 청크 수신
          pthread_mutex_lock(&dataMutex);

          if(header->frameId == receiveStatus.frameId && receiveStatus.colorDataBuffer){
            int maxDataPerMsg = MAX_MSG_SIZE - sizeof(MessageHeader);
            int offset = header->chunkIndex * maxDataPerMsg;

            memcpy(receiveStatus.colorDataBuffer + offset, msgBuffer + sizeof(MessageHeader), header->dataSize);

            receiveStatus.receivedColorChunks++;
            receiveStatus.totalColorChunks = header->totalChunks;
          }

          pthread_mutex_unlock(&dataMutex);
          break;
      }

      // 프레임 완성 여부 확인
      pthread_mutex_lock(&dataMutex);

      if(receiveStatus.totalDepthChunks > 0 && receiveStatus.totalColorChunks > 0 && receiveStatus.receivedDepthChunks == receiveStatus.totalDepthChunks && receiveStatus.receivedColorChunks == receiveStatus.totalColorChunks){
        // 프레임이 완성됨 - 메인 버퍼로 복사
        if(currentWidth != receiveStatus.width || currentHeight != receiveStatus.height){
          // 버퍼 크기 변경 필요
          free(depthDataBuffer);
          free(colorDataBuffer);

          depthDataBuffer = (int16_t*)malloc(receiveStatus.width * receiveStatus.height * sizeof(int16_t));
          colorDataBuffer = (uint8_t*)malloc(receiveStatus.width * receiveStatus.height * 3 * sizeof(uint8_t));

          if(!depthDataBuffer || !colorDataBuffer){
            perror("malloc display buffers");
            free(depthDataBuffer);
            free(colorDataBuffer);
            depthDataBuffer = NULL;
            colorDataBuffer = NULL;
            pthread_mutex_unlock(&dataMutex);
            continue;
          }

          currentWidth = receiveStatus.width;
          currentHeight = receiveStatus.height;
        }

        if(depthDataBuffer && colorDataBuffer){
          // 데이터 복사
          memcpy(depthDataBuffer, receiveStatus.depthDataBuffer, currentWidth * currentHeight * sizeof(int16_t));
          memcpy(colorDataBuffer, receiveStatus.colorDataBuffer, currentWidth * currentHeight * 3 * sizeof(uint8_t));

          currentFrameId = receiveStatus.frameId;
          hasNewData = 1;
        }

        // 상태 초기화를 위한 플래그
        receiveStatus.isComplete = 1;
      }

      pthread_mutex_unlock(&dataMutex);
    }
  }

  // 정리
  free(msgBuffer);

  if(receiveStatus.depthDataBuffer){
    free(receiveStatus.depthDataBuffer);
  }
  if(receiveStatus.colorDataBuffer){
    free(receiveStatus.colorDataBuffer);
  }

  mq_close(mqReceive);

  printf("Data receive thread terminated\n");
  return NULL;
}

// 3D Rendring Function
void display_3d_color(){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity();
  glTranslatef(0.0f, 0.0f, -distance);
  glRotatef(angleX, 1.0f, 0.0f, 0.0f);
  glRotatef(angleY, 0.0f, 1.0f, 0.0f);

  glScalef(zoom, zoom, zoom);

  pthread_mutex_lock(&dataMutex);

  if(hasNewData && depthDataBuffer && colorDataBuffer){
    glBegin(GL_POINTS);

    for(int y = 0; y < currentHeight; ++y){
      for(int x = 0; x < currentWidth; ++x){
        int index = y * currentWidth + x;
        int depthValue = depthDataBuffer[index];

        if(depthValue > 0){
          // Calculate 3D Coordinate (Using Simple Camera Model)
          float z_pos = depthValue / 1000.0f; // from mm to m
          float x_pos = (x - currentWidth / 2) * z_pos  / 570.3f; // 570.3f is focal distance of Astra camera
          float y_pos = (y - currentHeight / 2) * z_pos / 570.3f;

          // Set color using color data (RGB order)
          int colorIndex = index * 3; // RGB consist of 3 values.
          float r = colorDataBuffer[colorIndex] / 255.0f;
          float g = colorDataBuffer[colorIndex + 1] / 255.0f;
          float b = colorDataBuffer[colorIndex + 2] / 255.0f;

          glColor3f(r, g, b);
          glVertex3f(x_pos, -y_pos, -z_pos);
        }
      }
    }

    glEnd();
  }

  pthread_mutex_unlock(&dataMutex);
  glfwSwapBuffers(window);
}

// GLFW Error Callback
void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

// Keyboard Callback
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }

  // 외부 프로그램에 종료 신호 보내기 (전역변수나 신호 사용)
  if(exitCallback){
    exitCallback(); // 메인 프로그램에 등록된 콜백 호출
  }
}


void setExitCallback(ExitCallbackFunc callback){
  exitCallback = callback;
}

// Mouse Button Callback
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      isDragging = 1;
      glfwGetCursorPos(window, (double*)&lastMouseX, (double*)&lastMouseY);
    } else if (action == GLFW_RELEASE) {
      isDragging = 0;
    }
  }
}

// Mouse Moving Callback
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (isDragging) {
        int dx = xpos - lastMouseX;
        int dy = ypos - lastMouseY;

        angleX += dy * 0.05f;
        angleY += dx * 0.05f;

        lastMouseX = xpos;
        lastMouseY = ypos;
    }
}

// Scroll Callback
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (yoffset > 0) {
        // Zoom in
        zoom *= 1.1f;
    } else {
        // Zoom out
        zoom *= 0.9f;
    }
}

// Window Size Change Callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, (double)width / (double)height, 0.1, 100.0);
  glMatrixMode(GL_MODELVIEW);
}

void window_close_callback(GLFWwindow* window){
  viewerIsRunning = 0;

  // 외부 프로그램에 종료 신호 보내기
  if(exitCallback){
    exitCallback();
  }
}

// Viewer Thread Function
void* viewerModule(void* id){
  printf("Initializing GLFW...\n");

  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return NULL;
  }
    
  glfwSetErrorCallback(error_callback);
    
  // Get Monitor Resolution
  int screenWidth, screenHeight;
#ifdef _WIN32
  screenWidth = GetSystemMetrics(SM_CXSCREEN);
  screenHeight = GetSystemMetrics(SM_CYSCREEN);
#elif __linux__
  Display* d = XOpenDisplay(NULL);
  Screen* s = DefaultScreenOfDisplay(d);
  screenWidth = s->width;
  screenHeight = s->height;
  XCloseDisplay(d);
#endif

  // GLFW window creation
  window = glfwCreateWindow(screenWidth, screenHeight, "3D Viewer", NULL, NULL);

  glfwSetWindowCloseCallback(window, window_close_callback);
  
  if (!window) {
      fprintf(stderr, "Failed to create GLFW window\n");
      glfwTerminate();
      return NULL;
  }
    
  glfwMakeContextCurrent(window);
  glfwSetKeyCallback(window, key_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
  // Initialize GLEW
  GLenum err = glewInit();
  if (err != GLEW_OK) {
      fprintf(stderr, "GLEW initialization error: %s\n", glewGetErrorString(err));
      glfwTerminate();
      return NULL;
  }
    
  initOpenGL();

  // Set up initial perspective
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  framebuffer_size_callback(window, width, height);

  // Start Data Receiving Thread
  pthread_t receiveThreadId;
  pthread_create(&receiveThreadId, NULL, dataReceiveThread, NULL);

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    display_3d_color();
    glfwPollEvents();
  }

  // 정리
  viewerIsRunning = 0;
  pthread_join(receiveThreadId, NULL);

  free(depthDataBuffer);
  free(colorDataBuffer);

  glfwDestroyWindow(window);
  glfwTerminate();

  printf("Viewer thread terminated\n");
  return NULL;
}

pthread_t viewer_thread_id;
void initViewerModule(){
  viewerIsRunning = 1;
  pthread_create(&viewer_thread_id, NULL, viewerModule, NULL);
}

void stopViewerModule(){
  viewerIsRunning = 0;
  pthread_join(viewer_thread_id, NULL);
}

int isViewerModuleRunning(void){
  return viewerIsRunning;
}

void requestStopViewerModule(){
  viewerIsRunning = 0;
}
