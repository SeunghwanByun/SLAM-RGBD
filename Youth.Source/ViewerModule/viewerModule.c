#include "viewerModule.h"
#include "astra_wrapper.h"
#include <pthread.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

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
float zoom = 1.0f;
int isDragging = 0;

// extern AstraContext_t* context;
AstraContext_t* context;
GLFWwindow* window;

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
    
    int width, height;
    const int16_t* depthData = GetDepthDataAstraOpenGL(context, &width, &height);
    const uint8_t* colorData = GetColorDataAstraOpenGL(context, &width, &height);
    if(depthData && colorData){
        for(int y = 0; y < height; ++y){
            for(int x = 0; x < width; ++x){
                int index = y * width + x;
                int depthValue = depthData[index];

                if(depthValue > 0){
                    // Calculate 3D Coordinate (Using Simple Camera Model)
                    float z_pos = depthValue / 1000.0f; // from mm to m
                    float x_pos = (x - width / 2) * z_pos
                    / 570.3f; // 570.3f is focal distance of Astra camera
                    float y_pos = (y - height / 2) * z_pos
                    / 570.3f;

                    // Set color using color data (RGB order)
                    int colorIndex = index * 3; // RGB consist of 3 values.
                    float r = colorData[colorIndex] / 255.0f;
                    float g = colorData[colorIndex + 1] / 255.0f;
                    float b = colorData[colorIndex + 2] / 255.0f;

                    glColor3f(r, g, b);
                    glVertex3f(x_pos, -y_pos, -z_pos);
                }
            }
        }
    }

    glEnd();
    glfwSwapBuffers(window);
}

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        TerminateAstraObj(context);
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

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

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

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
    
    printf("Initializing Astra...\n");
    context = InitializeAstraObj();

#define EXEC_MODE 0 // 0 : OpenGL, 1 : WebGL
#if EXEC_MODE == 0
    // Set up initial perspective
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    framebuffer_size_callback(window, width, height);
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        display_3d_color();
        glfwPollEvents();
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
#elif EXEC_MODE == 1
    const char* colorData = GetColorDataAstraWebGL(context);

    if (colorData) {
        printf("%d\n", colorData);
    }
#endif

    return NULL;
}

pthread_t viewer_thread_id;
void initViewerModule(){
    pthread_create(&viewer_thread_id, NULL, viewerModule, NULL);
}
