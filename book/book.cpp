#include <armap.h>

enum PageState {
  PAGE_DOWN,
  PAGE_LEFT,
  PAGE_LEFTRIGHT,
  PAGE_RIGHT,
  PAGE_RIGHTLEFT
};

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
        int pageid;
        PageState pagestate;
        bool person;
};

Kinect *kinect;
MyApp *app;

void MyApp::init()
{
    App::init(1024, 768, false);

    alpha1 = 0;
    alpha2 = 1;
    alpha1diff = 0;
    alpha2diff = 0;
    pageid = 0;
    pagestate = PAGE_DOWN;
    person = false;

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

#define BOOK_L 240
#define BOOK_M 320
#define BOOK_R 400
#define BOOK_T 275
#define BOOK_B 350
#define BOOK_DMIN 1400
#define BOOK_DMAX 1590

#define PERSON_L 150
#define PERSON_R 470
#define PERSON_T 0
#define PERSON_B 220
#define PERSON_DMIN 900
#define PERSON_DMAX 2500

#define MAXPAGES 84 // has to be even!

void MyApp::loadPages()
{
    char buf[100];
    snprintf(buf, sizeof(buf), "book1/%03d.jpg", pageid);
    loadTexture(buf, texs[0]);
    snprintf(buf, sizeof(buf), "book1/%03d.jpg", pageid + 1);
    loadTexture(buf, texs[1]);
    printf("pages %d-%d loaded\n", pageid, pageid + 1);
}

void MyApp::calc()
{
    if (mode == 1 || mode == 2) {
        int cntp = 0, cnt1 = 0, cnt2 = 0;
        static int avgp = 0, avg1 = 0, avg2 = 0;
        depth = kinect->getDepth();
        for (int y = PERSON_T; y <= PERSON_B; y++) {
            for (int x = PERSON_L; x < PERSON_R; x++) {
                if (depth[x+y*640] > PERSON_DMIN && depth[x+y*640] < PERSON_DMAX) {
                    cntp++;
                }
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
        avgp = (avgp*9 + cntp) / 10;
        avg1 = (avg1*9 + cnt1) / 10;
        avg2 = (avg2*9 + cnt2) / 10;

        if (avgp >= 2000) {
            if (person == false) {
                printf("person in\n");
                person = true;
                pageid = (rand() % (MAXPAGES/2))*2;
                pagestate = PAGE_DOWN;
                loadPages();
                alpha1diff = 1;
            }
        } else
        if (avgp <= 1700) {
            if (person == true) {
                printf("person out\n");
                person = false;
                alpha1diff = -1;
            }
        }

        if (avg2 >= 500 && avg1 < 500) { // left side up
           alpha2diff = -1;
           if (pagestate == PAGE_RIGHT) {
               pagestate = PAGE_RIGHTLEFT;
               printf("RIGHT -> RIGHTLEFT\n");
           } else
           if (pagestate == PAGE_DOWN) {
               pagestate = PAGE_LEFT;
               printf("DOWN -> LEFT\n");
           } else
           if (pagestate == PAGE_LEFTRIGHT) {
               pagestate = PAGE_LEFT;
               printf("LEFTRIGHT -> LEFT\n");
           }
        }

        if (avg1 >= 500 && avg2 < 500) { // right side up
           alpha2diff = -1;
           if (pagestate == PAGE_LEFT) {
               pagestate = PAGE_LEFTRIGHT;
               printf("LEFT -> LEFTRIGHT\n");
           } else
           if (pagestate == PAGE_DOWN) {
               pagestate = PAGE_RIGHT;
               printf("DOWN -> RIGHT\n");
           } else
           if (pagestate == PAGE_RIGHTLEFT) {
               pagestate = PAGE_RIGHT;
               printf("RIGHTLEFT -> RIGHT\n");
           }
        }

        if (avg1 < 500 && avg2 < 500) { // both sides down
            if (pagestate == PAGE_LEFTRIGHT && pageid > 1) {
                printf("LEFTRIGHT -> DOWN\n");
                pageid -= 2;
                loadPages();
            } else
            if (pagestate == PAGE_RIGHTLEFT && pageid < MAXPAGES - 2) {
                printf("RIGHTLEFT -> DOWN\n");
                pageid += 2;
                loadPages();
            }
            alpha2diff = 1;
            pagestate = PAGE_DOWN;
        }

//        printf("%d %d %d\n", avgp, avg1, avg2);
    }
    if (alpha1 < 1.0 && alpha1diff == 1) {
        alpha1 += 0.05;
    }
    if (alpha1 > 0.0 && alpha1diff == -1) {
        alpha1 -= 0.05;
    }
    if (alpha2 < 1.0 && alpha2diff == 1) {
        alpha2 += 0.05;
    }
    if (alpha2 > 0.0 && alpha2diff == -1) {
        alpha2 -= 0.05;
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
            if (depth[i] > BOOK_DMIN && depth[i] < BOOK_DMAX && (i%640) > BOOK_L && (i%640) < BOOK_R && (i/640) > BOOK_T && (i/640) < BOOK_B) { // book
                *p = 0; p++;
                *p = 128; p++;
                *p = 128; p++;
            } else
            if (depth[i] > PERSON_DMIN && depth[i] < PERSON_DMAX && (i%640) > PERSON_L && (i%640) < PERSON_R && (i/640) > PERSON_T && (i/640) < PERSON_B) { // person
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
        // rectangle for person area
        for (int i = PERSON_T; i <= PERSON_B; i++) {
            p[(i*640+PERSON_L)*3  ] = 0;
            p[(i*640+PERSON_L)*3+1] = 255;
            p[(i*640+PERSON_L)*3+2] = 0;
            p[(i*640+PERSON_R)*3  ] = 0;
            p[(i*640+PERSON_R)*3+1] = 255;
            p[(i*640+PERSON_R)*3+2] = 0;
        }
        for (int i = PERSON_L; i < PERSON_R; i++) {
            p[(PERSON_T*640+i)*3  ] = 0;
            p[(PERSON_T*640+i)*3+1] = 255;
            p[(PERSON_T*640+i)*3+2] = 0;
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
