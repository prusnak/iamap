/*
    iaMap - InterActive Mapping
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

#ifndef __KINECT_H__
#define __KINECT_H__

#include <libfreenect.h>

class Kinect {
    public:
        static Kinect *create(int index = 0);
        static Kinect *createFake();
        ~Kinect();
        uint16_t *getDepth();
        uint8_t *getVideo();
        void startVideo();
        void startDepth();
        void stopVideo();
        void stopDepth();
    private:
        Kinect(freenect_context *f_ctx, freenect_device *f_dev); // use create method instead
        freenect_device *f_dev;
        freenect_context *f_ctx;
        volatile bool die;
        uint8_t *buffer_video, *buffer_video_int;
        uint16_t *buffer_depth, *buffer_depth_int;
        bool running_video;
        bool running_depth;
        bool fake;
        static void depth_cb(freenect_device *dev, void *depth, uint32_t timestamp);
        static void video_cb(freenect_device *dev, void *video, uint32_t timestamp);
        static void *thread_func(void *arg);
};

#endif
