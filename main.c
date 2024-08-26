#include "stdio.h"

// OpenGL Header
#include <GL/glew.h>
#include <GL/glut.h>

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_POINT);
    glColor3f(1.0, 1.0, 1.0);
    glPointSize(5.0);
    glVertex2f(250.f, 250.f);
    glEnd();

    glFlush();
}

int main(int argc, char** argv)
{
    printf("Hello World\n");

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(500, 500);
    glutCreateWindow("OpenGL - First Window");

    glewInit();

    glutDisplayFunc(display);

    glutMainLoop();
    
    return 0;
}