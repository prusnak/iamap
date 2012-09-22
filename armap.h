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

#include <libfreenect.h>

class Kinect {
    public:
        Kinect(int index = 0);
        ~Kinect();
        void loop();
        uint16_t *getDepth();
        uint8_t *getVideo();
    // public because of callback
        volatile bool die;
        freenect_context *f_ctx;
    private:
        freenect_device *f_dev;
        uint8_t *buffer_video, *buffer_video_int;
        uint16_t *buffer_depth, *buffer_depth_int;
};
