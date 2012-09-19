/*
    ARMap SandBox - Augmented Reality Sandbox Toy
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

#include <stdio.h>
#include <SDL.h>
#include <SDL_thread.h>
#include <libfreenect.h>

#define RESX 640
#define RESY 480
#define BPP 32
#define FRAMESLEEP 1
#define FULLSCREEN 0

SDL_Surface *screen;
freenect_device *f_dev;
freenect_context *f_ctx;
volatile int quit = 0;

void depth_cb(freenect_device *dev, void *v_depth, uint32_t timestamp)
{
    int i;
    uint16_t *depth = (uint16_t *)v_depth;
    uint32_t *pixels = (uint32_t *)screen->pixels;
    SDL_LockSurface(screen);
    for (i = 0; i< 640*480; i++) {
        pixels[i] = depth[i]/5000.0*255;
    }
    SDL_UnlockSurface(screen);
}

void video_cb(freenect_device *dev, void *v_video, uint32_t timestamp)
{
    uint8_t *video = (uint8_t *)video;
}

int freenect_threadfunc(void *unused)
{
    freenect_set_depth_callback(f_dev, depth_cb);
    freenect_set_video_callback(f_dev, video_cb);
    freenect_set_depth_mode(f_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_REGISTERED));
    freenect_set_video_mode(f_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB));
    freenect_start_depth(f_dev);
    freenect_start_video(f_dev);
    while (!quit && freenect_process_events(f_ctx) >= 0) ;
    freenect_stop_depth(f_dev);
    freenect_stop_video(f_dev);
    freenect_close_device(f_dev);
    freenect_shutdown(f_ctx);
    return 0;
}

int main(int argc, char* argv[])
{
    SDL_Event event;
    SDL_Thread *thread;

    if (freenect_init(&f_ctx, NULL) < 0) {
        fprintf(stderr, "Could not initialize Freenect.\n");
        return 1;
    }
    freenect_select_subdevices(f_ctx, FREENECT_DEVICE_CAMERA);
    if (freenect_open_device(f_ctx, &f_dev, 0) < 0) {
        fprintf(stderr, "Could not find Kinect device.\n");
        freenect_shutdown(f_ctx);
        return 2;
    }
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not initialize SDL.\n");
        freenect_shutdown(f_ctx);
        return 3;
    }
    if (!(screen = SDL_SetVideoMode(RESX, RESY, BPP, SDL_HWSURFACE | SDL_FULLSCREEN*FULLSCREEN))) {
        fprintf(stderr, "Could not initialize video.\n");
        freenect_shutdown(f_ctx);
        SDL_Quit();
        return 4;
    }

    thread = SDL_CreateThread(freenect_threadfunc, NULL);

    while (!quit) {
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = 1; break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym) {
                        case SDLK_ESCAPE: quit = 1; break;
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
    SDL_WaitThread(thread, NULL);
    SDL_Quit();
    return 0;
}
