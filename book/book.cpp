#include <armap.h>

class MyApp: public App {
    public:
        void init();
        void calc();
        void draw();
    private:
        uint8_t px[640*480*3];
};

Kinect *kinect;
MyApp *app;

void MyApp::init()
{
    App::init(640, 480, false);

}

void MyApp::calc()
{
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

    uint16_t *d = kinect->getDepth();
    uint8_t *p = px;
    for (int i = 0; i < 640*480; i++) {
        if (d[i] == 0) {
            *p = 0; p++;
            *p = 0; p++;
            *p = 0; p++;
        } else
        if (d[i] > 720 && d[i] < 750) { // book
            *p = 255; p++;
            *p = 0; p++;
            *p = 0; p++;
        } else
        if (d[i] > 900 && d[i] < 1600) { // person
            *p = 0; p++;
            *p = 255; p++;
            *p = 0; p++;
        } else {
            *p = 0; p++;
            *p = 0; p++;
            *p = 0; p++;
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, px);

    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(attr_texcoord, 2, GL_FLOAT, GL_FALSE, 0, texcoords);
    glEnableVertexAttribArray(attr_pos);
    glEnableVertexAttribArray(attr_texcoord);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableVertexAttribArray(attr_pos);
    glDisableVertexAttribArray(attr_texcoord);
}


int main(int argc, char *argv[])
{

    kinect = Kinect::create();
    if (!kinect) return 1;

    app = new MyApp();
    app->init();
    kinect->startDepth();
    app->loop();

    delete kinect;
    delete app;
    return 0;
}
