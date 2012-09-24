/*
    ARMap SandBox - Augmented Reality Sandbox Toy
    Copyright (c) 2010 OpenKinect Project
    Copyright (c) 2012 Pavol Rusnak

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#if defined(__APPLE__)
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include <cstdio>
#include <cstring>
#include "armap.h"

const int GLWIDTH = 640;
const int GLHEIGHT = 480;
Kinect *kinect;
GLuint tex, window;
int mode = 0; // 0 - video, 1 - depth
uint8_t depth8[640*480];

struct vec {
    GLfloat x, y, z;
} mov = {0, 0, 600}, rot = {0, 0, 0}, movstart, rotstart;

int mousebutton = -1;
int mousestart[2] = {0, 0};

void GLDisplay()
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -mov.z);
    glRotatef(rot.y, 1.0, 0.0, 0.0);
    glTranslatef(mov.x, -mov.y, 0.0);
    glRotatef(rot.x, 0.0, 1.0, 0.0);

    glBindTexture(GL_TEXTURE_2D, tex);
    if (mode == 0) {
        glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, kinect->getVideo());
    }
    if (mode == 1) {
        uint16_t *d = kinect->getDepth();
        for (int i = 0; i < 640*480; i++) {
            unsigned char c = d[i]*255/5000;
            if (c) c = 255 - c;
            depth8[i] = c;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, 1, 640, 480, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, depth8);
    }

    glBegin(GL_TRIANGLE_FAN);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glTexCoord2f(0, 0); glVertex3f(-GLWIDTH/2, GLHEIGHT/2, 0);
        glTexCoord2f(1, 0); glVertex3f(GLWIDTH/2, GLHEIGHT/2, 0);
        glTexCoord2f(1, 1); glVertex3f(GLWIDTH/2, -GLHEIGHT/2, 0);
        glTexCoord2f(0, 1); glVertex3f(-GLWIDTH/2, -GLHEIGHT/2, 0);
    glEnd();

    glutSwapBuffers();
}

void GLKeyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case 27:
            glutDestroyWindow(window);
            return;
        case 'q':
            mode = 0;
            kinect->stopDepth();
            kinect->startVideo();
            return;
        case 'w':
            mode = 1;
            kinect->stopVideo();
            kinect->startDepth();
            return;
    }
}

void GLMouse(int button, int state, int x, int y)
{
    if (!state) {
        switch (button) {
            case 0:
            case 1:
            case 2:
                mousebutton = button;
                mousestart[0] = x;
                mousestart[1] = y;
                memcpy(&movstart, &mov, sizeof(mov));
                memcpy(&rotstart, &rot, sizeof(rot));
                break;
            case 3:
                mov.z -= 10;
                break;
            case 4:
                mov.z += 10;
                break;
        }
    } else {
        switch (button) {
            case 1:
                break;
            case 2:
                break;
        }
        mousebutton = -1;
        mousestart[0] = 0;
        mousestart[1] = 0;
    }
}

void GLMotion(int x, int y)
{
    if (mousebutton == 0) {
        rot.x = rotstart.x + (x-mousestart[0]) * 0.1;
        rot.y = rotstart.y + (y-mousestart[1]) * 0.1;
    }
    if (mousebutton == 2) {
        mov.x = movstart.x + (x-mousestart[0]) * 0.2;
        mov.y = movstart.y + (y-mousestart[1]) * 0.2;
    }
}

void GLReshape(int width, int height)
{
    if (!height) height = 1;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, float(width)/height, 0.1, 5000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void GLInit(int width, int height)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_TEXTURE_2D);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_FLAT);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GLReshape(width, height);
}

int main(int argc, char **argv)
{
    kinect = Kinect::create();
    if (!kinect) return 1;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
    glutInitWindowSize(GLWIDTH, GLHEIGHT);
    glutInitWindowPosition(0, 0);
    window = glutCreateWindow("ARMap SandBox");
    glutDisplayFunc(&GLDisplay);
    glutIdleFunc(&GLDisplay);
    glutReshapeFunc(&GLReshape);
    glutKeyboardFunc(&GLKeyboard);
    glutMouseFunc(&GLMouse);
    glutMotionFunc(&GLMotion);
    GLInit(GLWIDTH, GLHEIGHT);
    kinect->startVideo();
    glutMainLoop();
    delete kinect;
    return 0;
}
