#include <iamap.h>

class MyApp: public App {
    public:
        void init();
        void draw();
        bool handleEvent(SDL_Event event);
    private:
        int mode;
        int lvlmode;
        int lvlmin;
        int lvlmax;
        uint8_t rgbtex[640*480*3];
        uint8_t grid[640*480*3];
        uint16_t depth[640*480];
};

Kinect *kinect;
MyApp *app;

void MyApp::init()
{
    App::init(640, 480, false);

    mode = 0;
    lvlmode = 0;
    palette->load("palette-calib.txt");
    lvlmin = config->getInt("lvlmin");
    lvlmax = config->getInt("lvlmax");
    if (!lvlmin) {
        lvlmin = 2000;
        config->setInt("lvlmin", lvlmin);
    }
    if (!lvlmax) {
        lvlmax = 3000;
        config->setInt("lvlmax", lvlmax);
    }
    palette->rehash(lvlmin, lvlmax);

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

    memset(depth, 0, sizeof(depth));
    memset(rgbtex, 0, sizeof(rgbtex));
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
    static const GLfloat colors[4][4] = {
        {1,1,1,1},
        {1,1,1,1},
        {1,1,1,1},
        {1,1,1,1}
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
        case 2:  // depth - no avg
        case 3:  // depth - short avg
        case 4:  // depth - long avg
            d = kinect->getDepth();
            if (mode == 3) {
                for (int i = 0; i < 640*480; i++) {
                    if (d[i]) {
                        int v = d[i] + depth[i]*3;
                        depth[i] = v/4;
                    }
                }
                d = depth;
            }
            if (mode == 4) {
                for (int i = 0; i < 640*480; i++) {
                    if (d[i]) {
                        int v = d[i] + depth[i]*31;
                        depth[i] = v/32;
                    }
                }
                d = depth;
            }
            if (!d || !palette) break;
            for (int i = 0; i < 640*480; i++) {
                if (d[i]) {
                    int c = palette->getColor(d[i]);
                    rgbtex[i*3  ] = (c >> 16) & 0xFF;
                    rgbtex[i*3+1] = (c >> 8) & 0xFF;
                    rgbtex[i*3+2] = c & 0xFF;
                } else {
                    rgbtex[i*3  ] = 0;
                    rgbtex[i*3+1] = 0;
                    rgbtex[i*3+2] = 0;
                }
            }
            if (lvlmode) {
                for (int i = 0; i < 640; i++) {
                    int c = palette->getColor(i*palette->PALETTE_LEN/640);
                    for (int j = 0; j < 10; j++) {
                        rgbtex[i*3+j*640*3  ] = (c >> 16) & 0xFF;
                        rgbtex[i*3+j*640*3+1] = (c >> 8) & 0xFF;
                        rgbtex[i*3+j*640*3+2] = c & 0xFF;
                    }
                }
            }
            glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbtex);
            break;
    }

    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(attr_col, 4, GL_FLOAT, GL_FALSE, 0, colors);
    glVertexAttribPointer(attr_tex, 2, GL_FLOAT, GL_FALSE, 0, texcoords);
    glEnableVertexAttribArray(attr_pos);
    glEnableVertexAttribArray(attr_col);
    glEnableVertexAttribArray(attr_tex);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableVertexAttribArray(attr_pos);
    glDisableVertexAttribArray(attr_col);
    glDisableVertexAttribArray(attr_tex);
}

bool MyApp::handleEvent(SDL_Event event)
{
    switch (event.type) {
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
                case SDLK_LEFTBRACKET:
                    lvlmode = 1;
                    return true;
                case SDLK_RIGHTBRACKET:
                    lvlmode = 2;
                    return true;
            }
            break;
        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
                case SDLK_1:  // grid
                    kinect->stopVideo();
                    kinect->stopDepth();
                    mode = 0;
                    return true;
                case SDLK_2:  // video
                    kinect->stopDepth();
                    kinect->startVideo();
                    mode = 1;
                    return true;
                case SDLK_3:  // depth - no avg
                    kinect->stopVideo();
                    kinect->startDepth();
                    mode = 2;
                    return true;
                case SDLK_4:  // depth - short avg
                    kinect->stopVideo();
                    kinect->startDepth();
                    mode = 3;
                    return true;
                case SDLK_5:  // depth - long avg
                    kinect->stopVideo();
                    kinect->startDepth();
                    mode = 4;
                    return true;
                case SDLK_z:  // screenshot
                    kinect->startVideo();
                    kinect->getVideo(); // dummy for now
                    kinect->startVideo();
                    return true;
                case SDLK_q:
                    palette->load("palette-calib.txt");
                    palette->rehash(lvlmin, lvlmax);
                    return true;
                case SDLK_w:
                    palette->load("palette-geo.txt");
                    palette->rehash(lvlmin, lvlmax);
                    return true;
                case SDLK_e:
                    palette->load("palette-fire.txt");
                    palette->rehash(lvlmin, lvlmax);
                    return true;
                case SDLK_r:
                    palette->load("palette-bluered.txt");
                    palette->rehash(lvlmin, lvlmax);
                    return true;
                case SDLK_LEFTBRACKET:
                case SDLK_RIGHTBRACKET:
                    lvlmode = 0;
                    return true;
            }
            break;
        case SDL_MOUSEWHEEL:
            if (lvlmode == 1) {
                lvlmin -= event.wheel.y*10;
                config->setInt("lvlmin", lvlmin);
                palette->rehash(lvlmin, lvlmax);
                return true;
            }
            if (lvlmode == 2) {
                lvlmax += event.wheel.y*10;
                config->setInt("lvlmax", lvlmax);
                palette->rehash(lvlmin, lvlmax);
                return true;
            }
            break;
    }
    return false;
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
