/*
    ARMap - Augmented Reality Toy
    Copyright (C) 2012 Pavol Rusnak

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

#ifndef __APP_H__
#define __APP_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <SDL.h>
#include <SDL_opengles2.h>

class App {
    public:
        App();
        virtual ~App();
        void init(int width, int height, bool fullscreen);
        void loop();

        virtual void handleEvent(SDL_Event event) { }
        virtual void calc() { }
        virtual void draw() = 0;

        int width, height;
        int attr_pos, attr_texcoord;
        GLuint tex;
        GLuint texs[16];

    private:
        void quit(int rc);
        void GLdraw();

        SDL_Window *window;
        SDL_GLContext context;
        int mousebutton;
        int mousestart[2];
        struct vec {
            GLfloat x, y, z;
        } mov, rot, movstart, rotstart;
        int u_Projection, u_ModelView;
        int done;
        SDL_Event event;
};

#endif
