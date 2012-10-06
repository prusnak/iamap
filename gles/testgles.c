#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <SDL.h>
#include <SDL_opengles2.h>

#include "Matrix.hpp"

#define GLWIDTH  640
#define GLHEIGHT 480

SDL_Window *window = NULL;
SDL_GLContext context = NULL;

int mousebutton = 0;
int mousestart[2] = {0, 0};

struct vec {
    GLfloat x, y, z;
} mov = {0, 0, 600}, rot = {0, 0, 0}, movstart, rotstart;

void quit(int rc)
{
    if (context) SDL_GL_DeleteContext(context);
    if (window) SDL_DestroyWindow(window);
    exit(rc);
}

float view_rot = 0.0;
int u_Projection = -1, u_ModelView = -1;
int attr_pos = 0, attr_color = 1;

void GLdraw()
{
    static const GLfloat verts[4][2] = {
        { -GLWIDTH/2,  GLHEIGHT/2 },
        {  GLWIDTH/2,  GLHEIGHT/2 },
        {  GLWIDTH/2, -GLHEIGHT/2 },
        { -GLWIDTH/2, -GLHEIGHT/2 }
    };
    static const GLfloat colors[4][3] = {
        { 1, 0, 0 },
        { 0, 1, 0 },
        { 0, 0, 1 },
        { 1, 1, 1 }
    };

    mat4 matProjection, matModelView;
    const float fH = tan(45*M_PI/360) * 0.1;
    const float fW = fH * GLWIDTH / GLHEIGHT;
    matProjection = mat4().Frustum(-fW, fW, -fH, fH, 0.1, 5000.0);
    glUniformMatrix4fv(u_Projection, 1, GL_FALSE, matProjection.Pointer());

    matModelView =
        mat4().Rotate(-rot.x, vec3(0.0, 1.0, 0.0)) *
        mat4().Translate(mov.x, -mov.y, 0.0) *
        mat4().Rotate(-rot.y, vec3(1.0, 0.0, 0.0)) *
        mat4().Translate(0.0, 0.0, -mov.z);
    glUniformMatrix4fv(u_ModelView, 1, GL_FALSE, matModelView.Pointer());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(attr_color, 3, GL_FLOAT, GL_FALSE, 0, colors);
    glEnableVertexAttribArray(attr_pos);
    glEnableVertexAttribArray(attr_color);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableVertexAttribArray(attr_pos);
    glDisableVertexAttribArray(attr_color);

    view_rot += 0.1;
}

void GLreshape(int width, int height)
{
    glViewport(0, 0, (GLint) width, (GLint) height);
}


void GLinit()
{
    const char *fragShaderText = " \
varying vec4 v_color; \
void main() { \
   gl_FragColor = v_color; \
} \
";

    const char *vertShaderText = " \
uniform mat4 ModelView; \
uniform mat4 Projection; \
attribute vec4 pos; \
attribute vec4 color; \
varying vec4 v_color; \
void main() { \
   mat4 mvp = Projection * ModelView; \
   gl_Position = mvp * pos; \
   v_color = color; \
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
    attr_color = glGetAttribLocation(program, "color");
    u_Projection = glGetUniformLocation(program, "Projection");
    u_ModelView = glGetUniformLocation(program, "ModelView");
}


int main(int argc, char *argv[])
{
    int done;
    SDL_Event event;

    if (SDL_VideoInit(0) < 0) {
        fprintf(stderr, "Couldn't initialize video driver: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("ARMap Sandbox", 100, 100, GLWIDTH, GLHEIGHT, SDL_WINDOW_OPENGL);

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

    GLinit();
    GLreshape(GLWIDTH, GLHEIGHT);

    done = 0;
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

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
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
                    mousebutton = event.button.button;
                    mousestart[0] = event.button.x;
                    mousestart[1] = event.button.y;
                    memcpy(&movstart, &mov, sizeof(mov));
                    memcpy(&rotstart, &rot, sizeof(rot));
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
            }
        }
        GLdraw();
        SDL_GL_SwapWindow(window);
    }

    quit(0);
    return 0;
}
