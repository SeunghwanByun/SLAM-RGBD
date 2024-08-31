// main.c

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "astra_wrapper.h"  // 헤더 파일 포함

GLuint depthTexture;
GLuint colorTexture;

float angleX = 0.0f;
float angleY = 0.0f;
float distance = 2.0f;

MyAstraContext* context;

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
    const int16_t* depthData = my_astra_get_depth_data(context, &width, &height);

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
    const int16_t* depthData = my_astra_get_depth_data(context, &width, &height);

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
    const int16_t* depthData = my_astra_get_depth_data(context, &width, &height);

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
    const uint8_t* colorData = my_astra_get_color_data(context, &width, &height);

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

void display_3d()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int width, height;
    const int16_t* depthData = my_astra_get_depth_data(context, &width, &height);

    if (depthData)
    {
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -distance);
        glRotatef(angleX, 1.0f, 0.0f, 0.0f);
        glRotatef(angleY, 0.0f, 1.0f, 0.0f);

        glBegin(GL_POINTS);

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int index = y * width + x;
                int depthValue = depthData[index];

                if (depthValue > 0)
                {
                    // 3D 좌표 계산 (단순화된 카메라 모델 사용)
                    float z = depthValue / 1000.0f;  // 밀리미터에서 미터로 변환
                    float x_pos = (x - width / 2) * z / 570.3f;  // 570.3f는 Astra 카메라의 초점 거리 (단순화)
                    float y_pos = (y - height / 2) * z / 570.3f;

                    // 깊이에 따라 색상 변화
                    float intensity = 1.0f - z / distance;
                    glColor3f(intensity, intensity, intensity);

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
            my_astra_terminate(context);
            exit(0);
            break;
    }
}

void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        angleY += 5.0f;
    }
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        angleX += 5.0f;
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

int main(int argc, char** argv)
{
    // printf("Initializing OpenGL...\n");
    // glutInit(&argc, argv);
    // glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    // glutInitWindowSize(640, 480);
    // glutCreateWindow("Astra Camera Depth Viewer");

    // glewInit();
    // initOpenGL();

    // printf("Initializing Astra...\n");
    // context = my_astra_initialize();

    // // glutDisplayFunc(display);
    // // glutDisplayFunc(display_depth);
    // // glutDisplayFunc(display_point_cloud);
    // // glutDisplayFunc(display_color);
    // glutIdleFunc(idle);
    // glutMainLoop();

    // my_astra_terminate(context);


    printf("Initializing OpenGL...\n");
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(640, 480);
    glutCreateWindow("Astra Camera 3D Viewer");

    glewInit();
    initOpenGL();

    printf("Initializing Astra...\n");
    context = my_astra_initialize();

    glutDisplayFunc(display_3d);
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);

    glutMainLoop();

    my_astra_terminate(context);
    return 0;
}
