#include "palette.h"
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

Palette::Palette()
{
    minval = 0;
    maxval = PALETTE_LEN - 1;
    locolor = 0;
    hicolor = 0;
    memset(hash, sizeof(hash), 0);
}

Palette::~Palette()
{
}

bool Palette::load(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    char buf[4096];
    if (!f) return false;
    gradi.clear();
    gradc.clear();
    while ( fgets(buf, sizeof(buf), f) ) {
        float i;
        int r, g, b;
        if (buf[0] == '-') {
            sscanf(buf+2, "%d %d %d", &r, &g, &b);
            locolor = ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
            continue;
        }
        if (buf[0] == '+') {
            sscanf(buf+2, "%d %d %d", &r, &g, &b);
            hicolor = ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
            continue;
        }
        sscanf(buf, "%f %d %d %d", &i, &r, &g, &b);
        gradi.push_back(i);
        gradc.push_back( ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF) );
    }
    fclose(f);
    printf("Palette loaded from %s\n", filename);
    return true;
}

void Palette::rehash(int minval, int maxval)
{
    if (minval >= maxval) return;
    if (minval < 0) minval = 0;
    if (maxval >= PALETTE_LEN) maxval = PALETTE_LEN - 1;
    this->minval = minval;
    this->maxval = maxval;

    memset(hash, sizeof(hash), 0);
    for (int v = 0; v < PALETTE_LEN; v++) {
        float vv = ((float)v - minval) / (maxval - minval);
        if (v < minval) {
            hash[v] = locolor;
            continue;
        }
        if (v > maxval) {
            hash[v] = hicolor;
            continue;
        }
        for (size_t i = 0; i < gradi.size()-1; i++) {
            if (vv >= gradi[i] && vv <= gradi[i+1]) {
                float a = (vv-gradi[i]) / (gradi[i+1]-gradi[i]);
                int r1 = (gradc[i] >> 16) & 0xFF;
                int g1 = (gradc[i] >> 8) & 0xFF;
                int b1 = gradc[i] & 0xFF;
                int r2 = (gradc[i+1] >> 16) & 0xFF;
                int g2 = (gradc[i+1] >> 8) & 0xFF;
                int b2 = gradc[i+1] & 0xFF;
                int r = r1*(1-a) + r2*a;
                int g = g1*(1-a) + g2*a;
                int b = b1*(1-a) + b2*a;
                hash[v] = ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
            }
        }
    }
}
