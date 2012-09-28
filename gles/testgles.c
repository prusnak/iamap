#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <SDL.h>
#include <SDL_opengles2.h>

#define GLWIDTH  640
#define GLHEIGHT 480

SDL_Window *window = NULL;
SDL_GLContext context = NULL;

void quit(int rc)
{
    if (context) SDL_GL_DeleteContext(context);
    if (window) SDL_DestroyWindow(window);
    exit(rc);
}

float view_rot = 0.0;
int u_matrix = -1;
int attr_pos = 0, attr_color = 1;

void make_z_rot_matrix(GLfloat angle, GLfloat *m)
{
    float c = cos(angle * M_PI / 180.0);
    float s = sin(angle * M_PI / 180.0);
    int i;
    for (i = 0; i < 16; i++) {
        m[i] = 0.0;
    }
    m[0] = m[5] = m[10] = m[15] = 1.0;

    m[0] = c;
    m[1] = s;
    m[4] = -s;
    m[5] = c;
}

void make_scale_matrix(GLfloat xs, GLfloat ys, GLfloat zs, GLfloat *m)
{
    int i;
    for (i = 0; i < 16; i++) {
        m[i] = 0.0;
    }
    m[0] = xs;
    m[5] = ys;
    m[10] = zs;
    m[15] = 1.0;
}


void mul_matrix(GLfloat *prod, const GLfloat *a, const GLfloat *b)
{
#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  p[(col<<2)+row]
    GLfloat p[16];
    GLint i;
    for (i = 0; i < 4; i++) {
        const GLfloat ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
        P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
        P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
        P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
        P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
    }
    memcpy(prod, p, sizeof(p));
#undef A
#undef B
#undef PROD
}

void GLdraw()
{
    static const GLfloat verts[3][2] = {
        { -1, -1 },
        {  1, -1 },
        {  0,  1 }
    };
    static const GLfloat colors[3][3] = {
        { 1, 0, 0 },
        { 0, 1, 0 },
        { 0, 0, 1 }
    };
    GLfloat mat[16], rot[16], scale[16];

    /* Set modelview/projection matrix */
    make_z_rot_matrix(view_rot, rot);
    make_scale_matrix(0.5, 0.5, 0.5, scale);
    mul_matrix(mat, rot, scale);
    glUniformMatrix4fv(u_matrix, 1, GL_FALSE, mat);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(attr_color, 3, GL_FLOAT, GL_FALSE, 0, colors);
    glEnableVertexAttribArray(attr_pos);
    glEnableVertexAttribArray(attr_color);

    glDrawArrays(GL_TRIANGLES, 0, 3);

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
uniform mat4 modelviewProjection; \
attribute vec4 pos; \
attribute vec4 color; \
varying vec4 v_color; \
void main() { \
   gl_Position = modelviewProjection * pos; \
   v_color = color; \
} \
";

    GLuint fragShader, vertShader, program;
    GLint stat;

    glClearColor(0.4, 0.4, 0.4, 0.0);

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
    u_matrix = glGetUniformLocation(program, "modelviewProjection");
}


int main(int argc, char *argv[])
{
    int done;
    SDL_Event event;

    if (SDL_VideoInit(0) < 0) {
        fprintf(stderr, "Couldn't initialize video driver: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("ARMap SandBox", 100, 100, GLWIDTH, GLHEIGHT, SDL_WINDOW_OPENGL);

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
