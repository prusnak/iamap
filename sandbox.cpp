#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <SDL.h>
#include <SDL_opengles2.h>
#ifdef RPI
#include <bcm_host.h>
#endif

#include "armap.h"
#include "Matrix.hpp"

#define GLWIDTH  640
#define GLHEIGHT 480

SDL_Window *window = NULL;
SDL_GLContext context = NULL;

Kinect *kinect;
GLuint tex;

int mode = 0;
int mousebutton = 0;
int mousestart[2] = {0, 0};

uint8_t depth8[640*480];
uint16_t depth16[640*480];
uint8_t grid[640*480*3];

struct vec {
    GLfloat x, y, z;
} mov = {0, 0, 579}, rot = {0, 0, 0}, movstart, rotstart;

void quit(int rc)
{
    if (context) SDL_GL_DeleteContext(context);
    if (window) SDL_DestroyWindow(window);
#ifdef RPI
    bcm_host_deinit();
#endif
    SDL_Quit();
    exit(rc);
}

float view_rot = 0.0;
int u_Projection = -1, u_ModelView = -1;
int attr_pos = 0, attr_texcoord = 1;

void GLdraw()
{
    static const GLfloat verts[4][2] = {
        { -GLWIDTH/2,  GLHEIGHT/2 },
        {  GLWIDTH/2,  GLHEIGHT/2 },
        {  GLWIDTH/2, -GLHEIGHT/2 },
        { -GLWIDTH/2, -GLHEIGHT/2 }
    };
    static const GLfloat texcoords[4][2] = {
        {0, 0},
        {1, 0},
        {1, 1},
        {0, 1}
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

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    uint16_t *d;
    switch (mode) {
        case 0:  // grid
            glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, grid);
            break;
        case 1:  // video
            glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, kinect->getVideo());
            break;
        case 2:  // depth
            d = kinect->getDepth();
            for (int i = 0; i < 640*480; i++) {
                unsigned char c = d[i]*255/5000;
                if (c) c = 255 - c;
                depth8[i] = c;
            }
            glTexImage2D(GL_TEXTURE_2D, 0, 1, 640, 480, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, depth8);
            break;
        case 3:  // depth avg
            d = kinect->getDepth();
            for (int i = 0; i < 640*480; i++) {
                if (d[i]) {
                    depth16[i] = (d[i]*3 + depth16[i]*17)/20;
                }
                unsigned char c = depth16[i]*255/5000;
                if (c) c = 255 - c;
                depth8[i] = c;
            }
            glTexImage2D(GL_TEXTURE_2D, 0, 1, 640, 480, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, depth8);
            break;
    }

    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(attr_texcoord, 2, GL_FLOAT, GL_FALSE, 0, texcoords);
    glEnableVertexAttribArray(attr_pos);
    glEnableVertexAttribArray(attr_texcoord);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableVertexAttribArray(attr_pos);
    glDisableVertexAttribArray(attr_texcoord);

    view_rot += 0.1;
}

void GLreshape(int width, int height)
{
    glViewport(0, 0, (GLint) width, (GLint) height);
}


void GLinit()
{
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
}


int main(int argc, char *argv[])
{
    int done;
    SDL_Event event;

    kinect = Kinect::create();
    if (!kinect) return 1;

#ifdef RPI
    bcm_host_init();
#endif

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Unable to initialize SDL:  %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("ARMap Sandbox", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, GLWIDTH, GLHEIGHT, SDL_WINDOW_OPENGL); // | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS

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

    // initialize green grid
    memset(grid, 0, sizeof(grid));
    for (int i=0; i<640; i++) {
        grid[(0*640+i)*3+1] = 255;
        grid[(80*640+i)*3+1] = 255;
        grid[(160*640+i)*3+1] = 255;
        grid[(240*640+i)*3+1] = 255;
        grid[(320*640+i)*3+1] = 255;
        grid[(400*640+i)*3+1] = 255;
        grid[(479*640+i)*3+1] = 255;
    }
    for (int i=0; i<480; i++) {
        grid[(i*640+0)*3+1] = 255;
        grid[(i*640+80)*3+1] = 255;
        grid[(i*640+160)*3+1] = 255;
        grid[(i*640+240)*3+1] = 255;
        grid[(i*640+320)*3+1] = 255;
        grid[(i*640+400)*3+1] = 255;
        grid[(i*640+480)*3+1] = 255;
        grid[(i*640+560)*3+1] = 255;
        grid[(i*640+639)*3+1] = 255;
    }

    memset(depth8, 0, sizeof(depth8));
    memset(depth16, 0, sizeof(depth16));

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
                        case SDLK_q:  // grid
                            kinect->stopVideo();
                            kinect->stopDepth();
                            mode = 0;
                            break;
                        case SDLK_w:  // video
                            kinect->stopDepth();
                            kinect->startVideo();
                            mode = 1;
                            break;
                        case SDLK_e:  // depth
                            kinect->stopVideo();
                            kinect->startDepth();
                            mode = 2;
                            break;
                        case SDLK_r:  // depth avg
                            kinect->stopVideo();
                            kinect->startDepth();
                            mode = 3;
                            break;
                        case SDLK_z:  // screenshot
                            kinect->startVideo();
                            kinect->getVideo(); // dummy for now
                            kinect->startVideo();
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
            }
        }
        GLdraw();
        SDL_GL_SwapWindow(window);
    }

    delete kinect;
    quit(0);
    return 0;
}
