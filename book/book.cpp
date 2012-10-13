#include <armap.h>

class MyApp: public App {
    public:
        void init();
        void calc();
        void draw();
        void handleEvent(SDL_Event event);
        void loadPages();
    private:
        int mode;
        uint8_t px[640*480*3];
        uint16_t *depth;
        float alpha1, alpha2;
        int alpha1diff, alpha2diff;
        int pageid, nextpageid;
};

Kinect *kinect;
MyApp *app;

void MyApp::init()
{
    App::init(640, 480, false);

    alpha1 = 0;
    alpha2 = 1;
    alpha1diff = 0;
    alpha2diff = 0;
    pageid = 0;
    nextpageid = 0;

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

#define BOOK_L 180
#define BOOK_M 320
#define BOOK_R 460
#define BOOK_T 220
#define BOOK_B 320
#define BOOK_DMIN 600
#define BOOK_DMAX 720

#define PERSON_B 120
#define PERSON_DMIN 850
#define PERSON_DMAX 1600

#define MAXPAGES 150

void MyApp::loadPages()
{
    // TODO: load correct textures
    if (rand()%2) {
        loadTexture("pageA.png", texs[0]);
    } else {
        loadTexture("pageB.png", texs[0]);
    }
    if (rand()%2) {
        loadTexture("pageA.png", texs[1]);
    } else {
        loadTexture("pageB.png", texs[1]);
    }
    printf("page %d loaded\n", pageid);
}

void MyApp::calc()
{
    if (mode == 1 || mode == 2) {
        int cntp = 0, cnt1 = 0, cnt2 = 0;
        static int avgp = 0, avg1 = 0, avg2 = 0;
        static int oldp = 0, old1 = 0, old2 = 0;
        depth = kinect->getDepth();
        for (int i = 0; i < 640*PERSON_B; i++) {
            if (depth[i] > PERSON_DMIN && depth[i] < PERSON_DMAX) {
                cntp++;
            }
        }
        for (int y = BOOK_T; y <= BOOK_B; y++) {
            for (int x = BOOK_L; x < BOOK_M; x++) {
                if (depth[x+y*640] > BOOK_DMIN && depth[x+y*640] < BOOK_DMAX) {
                    cnt1++;
                }
            }
            for (int x = BOOK_M; x <= BOOK_R; x++) {
                if (depth[x+y*640] > BOOK_DMIN && depth[x+y*640] < BOOK_DMAX) {
                    cnt2++;
                }
            }
        }
        avgp = (avgp*4 + cntp) / 5;
        avg1 = (avg1*4 + cnt1) / 5;
        avg2 = (avg2*4 + cnt2) / 5;
        if (avgp >= 1000 && oldp < 1000) {
            printf("person in\n");
            alpha1diff = 1;
            pageid = rand() % MAXPAGES;
            loadPages();
        }
        if (oldp >= 1000 && avgp < 1000) {
            printf("person out\n");
            alpha1diff = -1;
        }

        if (avg1 >= 500 && old1 < 500) {
            if (nextpageid % 2 == 0) {
                alpha2diff = -1;
                nextpageid = nextpageid + 1;
                printf("page +0\n");
            } else {
                alpha2diff = +1;
                nextpageid = nextpageid - 1;
                printf("page --\n");
            }
        }

        if (avg2 >= 500 && old2 < 500) {
            if (nextpageid % 2 == 0) {
                alpha2diff = -1;
                nextpageid = nextpageid - 1;
                printf("page -0\n");
            } else {
                alpha2diff = +1;
                nextpageid = nextpageid + 1;
                printf("page ++\n");
            }
        }
        oldp = avgp;
        old1 = avg1;
        old2 = avg2;
//        printf("%d %d %d %d %d\n", avgp, avg1, avg2, snc1, snc2);
    }
    if (alpha1 < 1.0 && alpha1diff == 1) {
        alpha1 += 0.02;
    }
    if (alpha1 > 0.0 && alpha1diff == -1) {
        alpha1 -= 0.02;
    }
    if (alpha2 < 1.0 && alpha2diff == 1) {
        alpha2 += 0.05;
    }
    if (alpha2diff == -1) {
        if (alpha2 > 0.0) {
            alpha2 -= 0.05;
        } else {
            alpha2diff = 1;
            pageid = nextpageid;
            loadPages();
        }
    }
}

void MyApp::draw()
{
    static const GLfloat verts[4][2] = {
        { -(GLfloat)width/2,  (GLfloat)height/2 },
        {  (GLfloat)width/2,  (GLfloat)height/2 },
        {  (GLfloat)width/2, -(GLfloat)height/2 },
        { -(GLfloat)width/2, -(GLfloat)height/2 }
    };
    static const GLfloat vertsA[4][2] = {
        { -(GLfloat)width/2-8,  (GLfloat)height/2 },
        {                  -8,  (GLfloat)height/2 },
        {                  -8, -(GLfloat)height/2 },
        { -(GLfloat)width/2-8, -(GLfloat)height/2 }
    };
    static const GLfloat vertsB[4][2] = {
        {                   8,  (GLfloat)height/2 },
        {  (GLfloat)width/2+8,  (GLfloat)height/2 },
        {  (GLfloat)width/2+8, -(GLfloat)height/2 },
        {                  +8, -(GLfloat)height/2 }
    };
    static const GLfloat texcoords[4][2] = {
        {0, 0},
        {1, 0},
        {1, 1},
        {0, 1}
    };
    static GLfloat colors[4][4] = {
        {1,1,1,1},
        {1,1,1,1},
        {1,1,1,1},
        {1,1,1,1}
    };

    uint8_t *p;
    if (mode == 0) {
        p = kinect->getVideo();
    } else
    if (mode == 1) {
        p = px;
        for (int i = 0; i < 640*480; i++) {
            if (depth[i] == 0) {
                *p = 0; p++;
                *p = 0; p++;
                *p = 0; p++;
            } else
            if (depth[i] > BOOK_DMIN && depth[i] < BOOK_DMAX) { // book
                *p = 0; p++;
                *p = 128; p++;
                *p = 128; p++;
            } else
            if (depth[i] > PERSON_DMIN && depth[i] < PERSON_DMAX) { // person
                *p = 128; p++;
                *p = 0; p++;
                *p = 128; p++;
            } else {
                *p = 0; p++;
                *p = 0; p++;
                *p = 0; p++;
            }
        }
        p = px;
    }

    if (mode == 0 || mode == 1) {
        // rectangles for book areas
        for (int i = BOOK_L; i <= BOOK_R; i++) {
            p[(BOOK_T*640+i)*3  ] = 255;
            p[(BOOK_T*640+i)*3+1] = 0;
            p[(BOOK_T*640+i)*3+2] = 0;
            p[(BOOK_B*640+i)*3  ] = 255;
            p[(BOOK_B*640+i)*3+1] = 0;
            p[(BOOK_B*640+i)*3+2] = 0;
        }
        for (int i = BOOK_T; i <= BOOK_B; i++) {
            p[(i*640+BOOK_L)*3  ] = 255;
            p[(i*640+BOOK_L)*3+1] = 0;
            p[(i*640+BOOK_L)*3+2] = 0;
            p[(i*640+BOOK_M)*3  ] = 255;
            p[(i*640+BOOK_M)*3+1] = 0;
            p[(i*640+BOOK_M)*3+2] = 0;
            p[(i*640+BOOK_R)*3  ] = 255;
            p[(i*640+BOOK_R)*3+1] = 0;
            p[(i*640+BOOK_R)*3+2] = 0;
        }
        // line for person area
        for (int i = 0; i < 640; i++) {
            p[(PERSON_B*640+i)*3  ] = 0;
            p[(PERSON_B*640+i)*3+1] = 255;
            p[(PERSON_B*640+i)*3+2] = 0;
        }
        colors[0][3] = colors[1][3] = colors[2][3] = colors[3][3] = 1;
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, p);
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

    if (mode == 2) {
        colors[0][3] = colors[1][3] = colors[2][3] = colors[3][3] = alpha1 * alpha2;
        glBindTexture(GL_TEXTURE_2D, texs[0]);
        glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, vertsA);
        glVertexAttribPointer(attr_col, 4, GL_FLOAT, GL_FALSE, 0, colors);
        glVertexAttribPointer(attr_tex, 2, GL_FLOAT, GL_FALSE, 0, texcoords);
        glEnableVertexAttribArray(attr_pos);
        glEnableVertexAttribArray(attr_col);
        glEnableVertexAttribArray(attr_tex);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glDisableVertexAttribArray(attr_pos);
        glDisableVertexAttribArray(attr_col);
        glDisableVertexAttribArray(attr_col);

        glBindTexture(GL_TEXTURE_2D, texs[1]);
        glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, vertsB);
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
}

void MyApp::handleEvent(SDL_Event event)
{
    switch (event.type) {
        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
                case SDLK_1:  // video
                    kinect->stopDepth();
                    kinect->startVideo();
                    mode = 0;
                    break;
                case SDLK_2:  // depth
                    kinect->stopVideo();
                    kinect->startDepth();
                    mode = 1;
                    break;
                case SDLK_3:  // real thing
                    kinect->stopVideo();
                    kinect->startDepth();
                    mode = 2;
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
    kinect->startVideo();
    app->loop();

    delete app;
    delete kinect;
    return 0;
}
