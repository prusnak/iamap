/*
    ARMap SandBox - Augmented Reality Sandbox Toy
    Copyright (c) 2010 OpenKinect Project
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

#include <cstdlib>
#include <cstring>
#include <SDL_thread.h>
#include "armap.h"

void depth_cb(freenect_device *dev, void *depth, uint32_t timestamp)
{
    uint16_t *b = (uint16_t *)depth;
    b[640*480] = 1; // dirty flag
}

void video_cb(freenect_device *dev, void *video, uint32_t timestamp)
{
    uint8_t *b = (uint8_t *)video;
    b[640*480*3] = 1; // dirty flag
}

int thread_func(void *arg)
{
    Kinect *kinect = (Kinect *)arg;
    while (!kinect->die && freenect_process_events(kinect->f_ctx) >= 0);
    return 0;
}

Kinect::Kinect(int index)
{
    if (freenect_init(&f_ctx, NULL) < 0) {
        throw "Could not init Freenect";
    }
    freenect_select_subdevices(f_ctx, (freenect_device_flags)FREENECT_DEVICE_CAMERA);
    if (freenect_open_device(f_ctx, &f_dev, index) < 0) {
        freenect_shutdown(f_ctx);
        throw "Could not find Kinect";
    }
    buffer_video_int = (uint8_t *)malloc(640*480*3+1);
    buffer_depth_int = (uint16_t *)malloc(640*480*2+1);
    buffer_video = (uint8_t *)malloc(640*480*3);
    buffer_depth = (uint16_t *)malloc(640*480*2);
    freenect_set_depth_callback(f_dev, depth_cb);
    freenect_set_video_callback(f_dev, video_cb);
    freenect_set_video_mode(f_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB));
    freenect_set_depth_mode(f_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_REGISTERED));
    freenect_set_video_buffer(f_dev, buffer_video_int);
    freenect_set_depth_buffer(f_dev, buffer_depth_int);
    freenect_start_depth(f_dev);
    freenect_start_video(f_dev);
    // freenect_set_led(f_dev, LED_RED);
}

void Kinect::loop()
{
    die = false;
    SDL_CreateThread(thread_func, this);
}

Kinect::~Kinect()
{
    die = true;
    // freenect_set_led(f_dev, LED_OFF);
    freenect_stop_depth(f_dev);
    freenect_stop_video(f_dev);
    free(buffer_video_int);
    free(buffer_depth_int);
    free(buffer_video);
    free(buffer_depth);
    freenect_close_device(f_dev);
    freenect_shutdown(f_ctx);
}

uint16_t *Kinect::getDepth()
{
    if (buffer_depth_int[640*480]) {
        memcpy(buffer_depth, buffer_depth_int, 640*480*2);
        buffer_depth_int[640*480] = 0;
    }
    return buffer_depth;
}

uint8_t *Kinect::getVideo()
{
    if (buffer_video_int[640*480*3]) {
        memcpy(buffer_video, buffer_video_int, 640*480*3);
        buffer_video_int[640*480*3] = 0;
    }
    return buffer_video;
}
