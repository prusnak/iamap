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

#include <SDL.h>
#include "armap.h"

#define RESX 640
#define RESY 480
#define BPP 32
#define FRAMESLEEP 200
#define FULLSCREEN 0

Kinect *kinect;
SDL_Surface *screen;
volatile int quit = 0;
int mode = 0;

void draw()
{
   // SDL_LockSurface(screen);
   uint32_t *p = (uint32_t *)screen->pixels;

   if (mode == 0) {
       uint8_t *v = kinect->getVideo();
       for (int i = 0; i < 640*480; i++) {
           p[i] = v[i*3+2] | (v[i*3+1]<<8) | (v[i*3]<<16);
       }
   }

   if (mode == 1) {
       uint16_t *d = kinect->getDepth();
       for (int i = 0; i < 640*480; i++) {
           unsigned char c = d[i]*255/5000;
           if (c) c = 255-c;
           p[i] = c | (c<<8) | (c<<16);
       }
   }

   // SDL_UnlockSurface(screen);
}

int main(int argc, char* argv[])
{

    if (SDL_Init(SDL_INIT_VIDEO) > 0) {
        throw "Could not initialize SDL.";
    }
    if (!(screen = SDL_SetVideoMode(RESX, RESY, BPP, SDL_HWSURFACE | SDL_FULLSCREEN*FULLSCREEN))) {
        throw "Could not initialize video.";
    }
    SDL_WM_SetCaption("ARMap SandBox", NULL);

    kinect = new Kinect();
    kinect->loop();

    SDL_Event event;
    while (!quit) {
        draw();
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = 1; break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym) {
                        case SDLK_ESCAPE: quit = 1; break;
                        case SDLK_q: mode = 0; break;
                        case SDLK_w: mode = 1; break;
                        default: break;
                    }
                    break;
            }
        }
        SDL_Flip(screen);
#ifdef FRAMESLEEP
        SDL_Delay(FRAMESLEEP);
#endif
    }
    SDL_Quit();
    delete kinect;
    return 0;
}
