#include "viewerModule.h"
#include "../SensorModule/astra_wrapper.h"

// Monitor Resolution Information
#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <X11/Xlib.h>
#endif

GLuint depthTexture;
GLuint colorTexture;

float angleX = 0.0f;
float angleY = 0.0f;
float distance = 2.0f;
int lastMouseX = 0, lastMouseY = 0;
int zoom = 1.0f;
int isDragging = 0;

// extern AstraContext_t* context;
AstraContext_t* context;

void initOpenGL()
{
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}


void display_3d_color(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -distance);
    glRotatef(angleX, 1.0f, 0.0f, 0.0f);
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);

    glScalef(zoom, zoom, zoom);

    glBegin(GL_POINTS);

    // // Receive buffer from sensor module
    
    // int width, height;
    // static int iTestCnt = 0;
    // int16_t* depthData;
    // uint8_t* colorData;

    // pthread_mutex_lock(&mutex);
    // while(!iEndOfSavingData){
    //     pthread_cond_wait(&cond, &mutex);
    // }

    // memcpy(&width, pcSensorData, sizeof(int));
    // memcpy(&height, pcSensorData + sizeof(int), sizeof(int));

    // // depthData는 크기가 int16_t이므로 그 크기에 맞게 복사
    // depthData = (int16_t*)malloc(sizeof(int16_t) * width * height);
    // if (depthData == NULL) {
    //     printf("malloc error for depthData\n");
    //     pthread_mutex_unlock(&mutex);
    //     return;
    // }

    // memcpy(depthData, pcSensorData + sizeof(int) * 2, sizeof(int16_t) * width * height);

    // // colorData는 크기가 uint8_t이므로 그 크기에 맞게 복사
    // colorData = (uint8_t*)malloc(sizeof(uint8_t) * width * height * 3);
    // if (colorData == NULL) {
    //     printf("malloc error for colorData\n");
    //     free(depthData);  // 이미 할당된 depthData 메모리 해제
    //     pthread_mutex_unlock(&mutex);
    //     return;
    // }
    // memcpy(colorData, pcSensorData + sizeof(int) * 2 + sizeof(int16_t) * width * height, sizeof(uint8_t) * width * height * 3);

    // pthread_mutex_unlock(&mutex);
    
    // if(depthData && colorData){
    //     for(int y = 0; y < height; ++y){
    //         for(int x = 0; x < width; ++x){
    //             int index = y * width + x;
    //             int depthValue = depthData[index];

    //             if(depthValue > 0){
    //                 // Calculate 3D Coordinate (Using Simple Camera Model)
    //                 float z_pos = depthValue / 1000.0f; // from mm to m
    //                 float x_pos = (x - width / 2) * z_pos
    //                 / 570.3f; // 570.3f is focal distance of Astra camera
    //                 float y_pos = (y - height / 2) * z_pos
    //                 / 570.3f;

    //                 // Set color using color data (RGB order)
    //                 int colorIndex = index * 3; // RGB consist of 3 values.
    //                 float r = colorData[colorIndex] / 255.0f;
    //                 float g = colorData[colorIndex + 1] / 255.0f;
    //                 float b = colorData[colorIndex + 2] / 255.0f;

    //                 // pstAstraData[index].fX = x_pos;
    //                 // pstAstraData[index].fY = -y_pos;
    //                 // pstAstraData[index].fZ = -z;
    //                 // pstAstraData[index].fR = r;
    //                 // pstAstraData[index].fG = g;
    //                 // pstAstraData[index].fB = b;

    //                 glColor3f(r, g, b);
    //                 glVertex3f(x_pos, -y_pos, -z_pos);
    //                 // printf("%lf %lf %lf %lf %lf %lf\n", x_pos, -y_pos, -z, r, g, b);
    //             }
    //         }
    //     }
    // }
    
    // // int width, height;
    // for(int i = 0; i < iNumOfPoint; i++){
    //     float x_pos = pstAstraData[i].fX;
    //     float y_pos = pstAstraData[i].fY;
    //     float z_pos = pstAstraData[i].fZ;

    //     float r = pstAstraData[i].fR;
    //     float g = pstAstraData[i].fG;
    //     float b = pstAstraData[i].fB;

    //     glColor3f(r, g, b);
    //     glVertex3f(x_pos, y_pos, z_pos);
    // }

    glEnd();
    glutSwapBuffers();
}

void idle()
{
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27: // ESC 키
            TerminateAstraObj(context);
            exit(0);
            break;
    }
}

// Mouse click event
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            isDragging = 1;
            lastMouseX = x;
            lastMouseY = y;
        } else if (state == GLUT_UP) {
            isDragging = 0;
        }
    }
}

// Mouse moving event (drag)
void motion(int x, int y) {
    if (isDragging) {
        int dx = x - lastMouseX;
        int dy = y - lastMouseY;

        angleX += dy * 0.05f;
        angleY += dx * 0.05f;

        lastMouseX = x;
        lastMouseY = y;

        glutPostRedisplay();
    }
}

void mouseWheel(int button, int dir, int x, int y){
    if(dir > 0){
        /* To do */

    }else {
        /* To do */
    }
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void* viewerModule(void* id){
    printf("Initializing OpenGL...\n");
    // glutInit(&argc, argv);
    // glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Get Monitor Resolution
#ifdef _WIN32
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
#elif __linux__
    Display* d = XOpenDisplay(NULL);
    Screen* s = DefaultScreenOfDisplay(d);
    int32_t screenWidth = s->width;
    int32_t screenHeight = s->height;
#endif // #ifdef _WIN32

    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("3D Viewer");

    glewInit();
    initOpenGL();
  
    printf("Initializing Astra...\n");
    context = InitializeAstraObj();

#define EXEC_MODE 0 // 0 : OpenGL, 1 : WebGL
#if EXEC_MODE == 0
    while(1){
        /* Main Viewer Code. */
        glutDisplayFunc(display_3d_color);
        glutIdleFunc(idle);
        glutReshapeFunc(reshape);
        glutKeyboardFunc(keyboard);
        glutMouseFunc(mouse);
        glutMotionFunc(motion);
        glutMouseWheelFunc(mouseWheel);
        glutMainLoop();
    }
#elif EXEC_MODE == 1
        // 컬러 데이터와 깊이 데이터 모두를 생성하여 사용하거나 출력
        // const char* depthData = GetDepthDataAstraWebGL(context);
        const char* colorData = GetColorDataAstraWebGL(context);

        // 데이터를 필요한 대로 처리합니다 (예: 콘솔 출력 또는 반환)
        if (colorData) {
            printf("%d\n", colorData);
        }

        // if (depthData) {
        //     std::cout << depthData << std::endl;
        // }
    #endif

        // TerminateAstraObj(context);
}
