#include <armap.h>

class MyApp: public App {
    public:
        void init();
        void draw();
        void handleEvent(SDL_Event event);
    private:
        int mode;
        uint8_t depth8[640*480];
        uint32_t depth32[640*480];
        uint8_t grid[640*480*3];
};

Kinect *kinect;
MyApp *app;

void MyApp::init()
{
    App::init(640, 480, false);

    mode = 0;

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
    memset(depth32, 0, sizeof(depth32));
}

void MyApp::draw()
{
    static const GLfloat verts[4][2] = {
        { -(GLfloat)width/2,  (GLfloat)height/2 },
        {  (GLfloat)width/2,  (GLfloat)height/2 },
        {  (GLfloat)width/2, -(GLfloat)height/2 },
        { -(GLfloat)width/2, -(GLfloat)height/2 }
    };
    static const GLfloat texcoords[4][2] = {
        {0, 0},
        {1, 0},
        {1, 1},
        {0, 1}
    };

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
                    depth32[i] = ((d[i]<<16)*4 + depth32[i]*12) / 16;
                }
                unsigned char c = (depth32[i]>>8)/5000;
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
}

void MyApp::handleEvent(SDL_Event event)
{
    switch (event.type) {
        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
                case SDLK_1:  // grid
                    kinect->stopVideo();
                    kinect->stopDepth();
                    mode = 0;
                    break;
                case SDLK_2:  // video
                    kinect->stopDepth();
                    kinect->startVideo();
                    mode = 1;
                    break;
                case SDLK_3:  // depth
                    kinect->stopVideo();
                    kinect->startDepth();
                    mode = 2;
                    break;
                case SDLK_4:  // depth avg
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
    }
}

int main(int argc, char *argv[])
{

    kinect = Kinect::create();
    if (!kinect) return 1;

    app = new MyApp();
    app->init();
    app->loop();

    delete app;
    delete kinect;
    return 0;
}

