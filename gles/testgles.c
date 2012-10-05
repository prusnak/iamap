#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <SDL.h>
#include <SDL_opengles2.h>

#define GLWIDTH  640
#define GLHEIGHT 480
#define GLASPECT 1.33333333333333333333

SDL_Window *window = NULL;
SDL_GLContext context = NULL;

struct vec {
    GLfloat x, y, z;
} mov = {0, 0, 250}, rot = {0, 0, 0}, movstart, rotstart;

void quit(int rc)
{
    if (context) SDL_GL_DeleteContext(context);
    if (window) SDL_DestroyWindow(window);
    exit(rc);
}

float view_rot = 0.0;
int u_matRotate = -1, u_matPerspective = -1, u_matMove = -1;
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
    static const float fov = 1.0; // ctg(45.0 deg)
    static const float aspect = GLASPECT;
    static const float zNear = 0.1;
    static const float zFar = 5000.0;
    GLfloat matPerspective[16] = { fov/aspect,   0,                         0,                           0,
                                            0, fov,                         0,                           0,
                                            0,   0, (zFar+zNear)/(zNear-zFar), 2*(zFar*zNear)/(zNear-zFar),
                                            0,   0,                        -1,                           0 };
    GLfloat matMove[16] = { 1, 0, 0, mov.x,
                            0, 1, 0, -mov.y,
                            0, 0, 1, -mov.z,
                            0, 0, 0,     1 };
    GLfloat matRotate[16] = { 1, 0, 0, 0,
                              0, 1, 0, 0,
                              0, 0, 1, 0,
                              0, 0, 0, 1 };

    glUniformMatrix4fv(u_matPerspective, 1, GL_FALSE, matPerspective);
    glUniformMatrix4fv(u_matMove, 1, GL_FALSE, matMove);
    glUniformMatrix4fv(u_matRotate, 1, GL_FALSE, matRotate);

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
uniform mat4 matMove; \
uniform mat4 matRotate; \
uniform mat4 matPerspective; \
attribute vec4 pos; \
attribute vec4 color; \
varying vec4 v_color; \
void main() { \
   gl_Position = matMove * matPerspective * pos; \
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
    u_matMove = glGetUniformLocation(program, "matMove");
    u_matRotate = glGetUniformLocation(program, "matRotate");
    u_matPerspective = glGetUniformLocation(program, "matPerspective");
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
