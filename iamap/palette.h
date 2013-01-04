#ifndef __PALETTE_H__
#define __PALETTE_H__

#include <vector>

class Palette {

    public:
        Palette();
        ~Palette();
        bool load(const char *filename);
        inline int getColor(int val) { if (val >= 0 && val < PALETTE_LEN) return hash[val]; else return 0; }
        void rehash(int minval, int maxval);
        static const int PALETTE_LEN = 6000;

    private:
        int minval, maxval;
        int locolor, hicolor;
        std::vector<float> gradi;
        std::vector<int> gradc;
        int hash[PALETTE_LEN];
};

#endif
