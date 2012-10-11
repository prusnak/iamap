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

#ifdef RPI
#include <bcm_host.h>
#endif
#include "app.h"
#include "matrix.h"

#define GLreshape(x, y) glViewport(0, 0, (GLint)(x), (GLint)(y))

App::App()
{
    window = NULL;
    context = NULL;
    mousebutton = 0;
    mousestart[0] = 0; mousestart[1] = 0;
    mov.x = 0; mov.y = 0; mov.z = 579;
    rot.x = 0; rot.y = 0; rot.z = 0;
    u_Projection = -1;
    u_ModelView = -1;
    attr_pos = 0;
    attr_texcoord = 1;
}

void App::init(int width, int height, bool fullscreen)
{
    this->width = width;
    this->height = height;

#ifdef RPI
    bcm_host_init();
#endif

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Unable to initialize SDL:  %s\n", SDL_GetError());
        return;
    }

    if (!fullscreen) {
        window = SDL_CreateWindow("ARMap Sandbox", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
    } else {
        window = SDL_CreateWindow("ARMap Sandbox", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS);
    }

    context = SDL_GL_CreateContext(window);
    if (!context) {
        fprintf(stderr, "SDL_GL_CreateContext(): %s\n", SDL_GetError());
        quit(2);
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    const char *fragShaderText = " \
varying vec2 v_texcoord; \
uniform sampler2D tex; \
void main() { \
   gl_FragColor = texture2D(tex, v_texcoord); \
} \
";

    const char *vertShaderText = " \
uniform mat4 m_modelview; \
uniform mat4 m_projection; \
attribute vec4 pos; \
attribute vec2 texcoord; \
varying vec2 v_texcoord; \
void main() { \
   gl_Position = m_projection * m_modelview * pos; \
   v_texcoord = texcoord; \
} \
";

    GLuint fragShader, vertShader, program;
    GLint stat;

    glClearColor(0.0, 0.0, 0.0, 1.0);

    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, (const char **) &fragShaderText, NULL);
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &stat);
    if (!stat) {
       fprintf(stderr, "Error: fragment shader did not compile!\n");
       exit(1);
    }

    vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, (const char **) &vertShaderText, NULL);
    glCompileShader(vertShader);
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &stat);
    if (!stat) {
       fprintf(stderr, "Error: vertex shader did not compile!\n");
       exit(1);
    }

    program = glCreateProgram();
    glAttachShader(program, fragShader);
    glAttachShader(program, vertShader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &stat);
    if (!stat) {
       fprintf(stderr, "Error: shaders did not link!");
       exit(1);
    }

    glUseProgram(program);
    attr_pos = glGetAttribLocation(program, "pos");
    attr_texcoord = glGetAttribLocation(program, "texcoord");
    u_Projection = glGetUniformLocation(program, "m_projection");
    u_ModelView = glGetUniformLocation(program, "m_modelview");

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(16, texs);
    for (int i = 0; i < 16; i++) {
        glBindTexture(GL_TEXTURE_2D, texs[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}

void App::loop()
{
    GLreshape(width, height);
    done = 0;
    FILE *f;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {

                case SDL_WINDOWEVENT:
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_RESIZED:
                            GLreshape(event.window.data1, event.window.data2);
                            break;
                        case SDL_WINDOWEVENT_CLOSE:
                            done = 1;
                            break;
                    }
                    break;

                case SDL_MOUSEMOTION:
                    if (mousebutton == 1) {
                        rot.x = rotstart.x + (event.motion.x-mousestart[0]) * 0.1;
                        rot.y = rotstart.y + (event.motion.y-mousestart[1]) * 0.1;
                    }
                    if (mousebutton == 3) {
                        mov.x = movstart.x + (event.motion.x-mousestart[0]) * 0.2;
                        mov.y = movstart.y + (event.motion.y-mousestart[1]) * 0.2;
                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == 1 || event.button.button == 3) {
                        mousebutton = event.button.button;
                        mousestart[0] = event.button.x;
                        mousestart[1] = event.button.y;
                        memcpy(&movstart, &mov, sizeof(mov));
                        memcpy(&rotstart, &rot, sizeof(rot));
                    }
                    if (event.button.button == 2) {
                        printf("%d %d\n", event.button.x, event.button.y);
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    mousebutton = 0;
                    mousestart[0] = 0;
                    mousestart[1] = 0;
                    break;

                case SDL_MOUSEWHEEL:
                    mov.z -= event.wheel.y*10;
                    break;

                case SDL_QUIT:
                    done = 1;
                    break;

                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            done = 1;
                            break;
                        case SDLK_s:
                            f = fopen("armap-coords.dat", "wb");
                            if (!f) break;
                            fwrite(&mov, sizeof(mov), 1, f);
                            fwrite(&rot, sizeof(rot), 1, f);
                            fclose(f);
                            printf("Coords written to armap-coords.dat\n");
                            break;
                        case SDLK_l:
                            f = fopen("armap-coords.dat", "rb");
                            if (!f) break;
                            fread(&mov, sizeof(mov), 1, f);
                            fread(&rot, sizeof(rot), 1, f);
                            fclose(f);
                            printf("Coords read from armap-coords.dat\n");
                            break;
                    }
                    break;
            }
            handleEvent(event);
        }
        calc();
        GLdraw();
        SDL_GL_SwapWindow(window);
    }
}

void App::GLdraw()
{
    mat4 matProjection, matModelView;
    const float fH = tan(45*M_PI/360) * 0.1;
    const float fW = fH * width / height;
    matProjection = mat4().Frustum(-fW, fW, -fH, fH, 0.1, 5000.0);
    glUniformMatrix4fv(u_Projection, 1, GL_FALSE, matProjection.Pointer());

    matModelView =
        mat4().Rotate(-rot.x, vec3(0.0, 1.0, 0.0)) *
        mat4().Translate(mov.x, -mov.y, 0.0) *
        mat4().Rotate(-rot.y, vec3(1.0, 0.0, 0.0)) *
        mat4().Translate(0.0, 0.0, -mov.z);
    glUniformMatrix4fv(u_ModelView, 1, GL_FALSE, matModelView.Pointer());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw();
}

void App::quit(int rc)
{
    if (context) SDL_GL_DeleteContext(context);
    if (window) SDL_DestroyWindow(window);
#ifdef RPI
    bcm_host_deinit();
#endif
    SDL_Quit();
    exit(rc);
}

App::~App()
{
    quit(0);
}

