#include "viewer.h"

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
int isDragging = 0;

AstraContext_t* context;

void initOpenGL()
{
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int width, height;
    const int16_t* depthData = GetDepthDataAstraOpenGL(context, &width, &height);

    if (depthData)
    {
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_SHORT, depthData);
        glBindTexture(GL_TEXTURE_2D, 0);

        glEnable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f,  1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f,  1.0f);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }

    glutSwapBuffers();
}

void display_depth()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int width, height;
    const int16_t* depthData = GetDepthDataAstraOpenGL(context, &width, &height);

    if (depthData)
    {
        // 데이터를 정규화하여 0.0에서 1.0 사이로 변환
        float* normalizedDepth = (float*)malloc(width * height * sizeof(float));
        for (int i = 0; i < width * height; ++i)
        {
            normalizedDepth[i] = depthData[i] / 1000.0f; // 0~1000mm의 값을 0.0~1.0 사이로 변환
        }

        // OpenGL 텍스처에 데이터를 매핑
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_FLOAT, normalizedDepth);
        glBindTexture(GL_TEXTURE_2D, 0);

        free(normalizedDepth);

        // OpenGL로 렌더링
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f,  1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f,  1.0f);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }

    glutSwapBuffers();
}

void display_point_cloud()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int width, height;
    const int16_t* depthData = GetDepthDataAstraOpenGL(context, &width, &height);

    if (depthData)
    {
        glPointSize(2.0f);
        glBegin(GL_POINTS);

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int index = y * width + x;
                int depthValue = depthData[index];

                if (depthValue > 0)
                {
                    // 기본적인 카메라 모델을 사용한 3D 좌표 계산 (단순화된 방식)
                    float z = depthValue / 1000.0f;  // 밀리미터에서 미터로 변환
                    float x_pos = (x - width / 2) * z / 570.3f;  // 570.3f는 Astra 카메라의 초점 거리 (단순화)
                    float y_pos = (y - height / 2) * z / 570.3f;

                    // 깊이에 따라 색상 변화
                    float intensity = 1.0f - z;
                    glColor3f(intensity, intensity, intensity);

                    glVertex3f(x_pos, -y_pos, -z);
                }
            }
        }

        glEnd();
    }

    glutSwapBuffers();
}

void display_color()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int width, height;
    const uint8_t* colorData = GetColorDataAstraOpenGL(context, &width, &height);

    if (colorData)
    {
        // OpenGL 텍스처에 RGB 데이터를 매핑
        glBindTexture(GL_TEXTURE_2D, colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, colorData);
        glBindTexture(GL_TEXTURE_2D, 0);

        // OpenGL로 렌더링
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f,  1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f,  1.0f);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }

    glutSwapBuffers();
}

void display_3d_color(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int width, height;
    const int16_t* depthData = GetDepthDataAstraOpenGL(context, &width, &height);
    const uint8_t* colorData = GetColorDataAstraOpenGL(context, &width, &height);

    if(depthData /*&& colorData*/){
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -distance);
        glRotatef(angleX, 1.0f, 0.0f, 0.0f);
        glRotatef(angleY, 0.0f, 1.0f, 0.0f);

        glBegin(GL_POINTS);

        for(int y = 0; y < height; ++y){
            for(int x = 0; x < width; ++x){
                int index = y * width + x;
                int depthValue = depthData[index];

                if(depthValue > 0){
                    // Calculate 3D Coordinate (Using Simple Camera Model)
                    float z = depthValue / 1000.0f; // from mm to m
                    float x_pos = (x - width / 2) * z / 570.3f; // 570.3f is focal distance of Astra camera
                    float y_pos = (y - height / 2) * z / 570.3f;

                    // Set color using color data (RGB order)
                    int colorIndex = index * 3; // RGB consist of 3 values.
                    float r = colorData[colorIndex] / 255.0f;
                    float g = colorData[colorIndex + 1] / 255.0f;
                    float b = colorData[colorIndex + 2] / 255.0f;

                    glColor3f(r, g, b);
                    glVertex3f(x_pos, -y_pos, -z);
                }
            }
        }

        glEnd();
    }

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

        angleX += dy * 0.5f;
        angleY += dx * 0.5f;

        lastMouseX = x;
        lastMouseY = y;

        glutPostRedisplay();
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

void InitViewer(int argc, char** argv){
    printf("Initializing OpenGL...\n");
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

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
    glutCreateWindow("Astra Camera 3D Viewer");

    glewInit();
    initOpenGL();
    
    printf("Initializing Astra...\n");
    context = InitializeAstraObj();

#define EXEC_MODE 1 // 0 : OpenGL, 1 : WebGL

#if EXEC_MODE == 0
    // glutDisplayFunc(display_3d);
    glutDisplayFunc(display_3d_color);
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

    glutMainLoop();
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


    TerminateAstraObj(context);
}