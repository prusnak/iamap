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
#include "armap.h"

const int GLWIDTH = 640;
const int GLHEIGHT = 480;
Kinect *kinect;
GLuint tex, window;
int mode = 0; // 0 - video, 1 - depth
uint8_t depth8[640*480];

void GLDisplay()
{
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
    glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
    glTexCoord2f(1, 0); glVertex3f(GLWIDTH, 0, 0);
    glTexCoord2f(1, 1); glVertex3f(GLWIDTH, GLHEIGHT, 0);
    glTexCoord2f(0, 1); glVertex3f(0, GLHEIGHT, 0);

    glEnd();
    glutSwapBuffers();
}

void GLKeyboard(unsigned char key, int x, int y)
{
    if (key == 27) {
        exit(0);
    }
    if (key == 'q') {
        mode = 0;
        kinect->stopDepth();
        kinect->startVideo();
    }
    if (key == 'w') {
        mode = 1;
        kinect->stopVideo();
        kinect->startDepth();
    }
}

void GLReshape(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, GLWIDTH, GLHEIGHT, 0, -1.0f, 1.0f);
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
    GLReshape(width, height);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
    glutInitWindowSize(GLWIDTH, GLHEIGHT);
    glutInitWindowPosition(0, 0);
    window = glutCreateWindow("ARMap SandBox");
    glutDisplayFunc(&GLDisplay);
    glutIdleFunc(&GLDisplay);
    glutReshapeFunc(&GLReshape);
    glutKeyboardFunc(&GLKeyboard);
    GLInit(GLWIDTH, GLHEIGHT);
    kinect = new Kinect();
    kinect->startVideo();
    glutMainLoop();
    return 0;
}
