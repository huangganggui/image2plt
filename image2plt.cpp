#undef cimg_display
#define cimg_display 0
#include "CImg.h"
using namespace cimg_library;

void PutFileHeader(FILE* pf, int w, int h, int pixel_height)
{
    std::fprintf(pf, "-79260 -37200 8320740 5902800 34\n");
    std::fprintf(pf, "-8012 -10984 %d %d\n", w*pixel_height, h*pixel_height);
    std::fprintf(pf, "16\n");
    std::fprintf(pf, "6350 4060 1520 0 0.000000\n");
    std::fprintf(pf, "7870 5840 2030 0 0.000000\n");
    std::fprintf(pf, "12700 9650 3300 0 0.000000\n");
    std::fprintf(pf, "16000 11940 4060 0 0.000000\n");
    std::fprintf(pf, "19050 14220 4830 0 0.000000\n");
    std::fprintf(pf, "20320 15240 5080 0 0.000000\n");
    std::fprintf(pf, "23880 17530 5840 0 0.000000\n");
    std::fprintf(pf, "25400 19050 6350 0 0.000000\n");
    std::fprintf(pf, "31750 23620 7870 0 0.000000\n");
    std::fprintf(pf, "39620 29720 15750 0 0.000000\n");
    std::fprintf(pf, "44450 33270 11180 0 0.000000\n");
    std::fprintf(pf, "47750 35810 11940 0 0.000000\n");
    std::fprintf(pf, "50800 38100 12700 0 0.000000\n");
    std::fprintf(pf, "57150 42420 14220 0 0.000000\n");
    std::fprintf(pf, "63500 48010 16000 0 0.000000\n");
    std::fprintf(pf, "127000 95250 31750 0 0.000000\n");
}

void PutFileRec(FILE* pf, int h, int i, int j, int len, int pixel_height)
{
    std::fprintf(pf, "1026\n");
    std::fprintf(pf, "%d %d %d %d\n", i*pixel_height, (h-j-1)*pixel_height, (i+len)*pixel_height, (h-j)*pixel_height);
}

int main(int argc,char **argv)
{
    cimg_usage("A simple tool converts BMP image file to Cadence allegro IPX file.\n\nUsage : image2plt [options] image");
    const char *file_o      = cimg_option("-o","test.plt","Output PLT file.");
    const char *file_i      = cimg_option("-i","test.bmp","Input BMP file.");
    const int pixel_height  = cimg_option("-s", 3528,"Pixel height multiply 10000 in mm.");
    bool visu = false;

    visu = cimg::option("-h",argc,argv,(char*)0,(char*)0,false)!=0;
    visu |= cimg::option("-help",argc,argv,(char*)0,(char*)0,false)!=0;
    visu |= cimg::option("--help",argc,argv,(char*)0,(char*)0,false)!=0;
    visu |= cimg::option("-?",argc,argv,(char*)0,(char*)0,false)!=0;
    if (visu) {
        return 1;
    }
    
    CImg<float> img;
    
    // Enable quiet exception mode
    cimg::exception_mode(0);
    try {
        img  = CImg<>(file_i);
    }
    catch (CImgIOException &e) {
        std::fprintf(stderr,"Load file %s failed!\n", file_i);
        return 1;
    }
    std::fprintf(stderr,"Load image file %s successful!\n", file_i);
    FILE* p_plt_f = std::fopen(file_o, "w");
    int w = img.width();
    int h = img.height();
    int d = img.depth();
    int s = img.spectrum();
    int *pb_cnts = new int[256];
    
    int max1 = 0;
    int max2 = 0;
    int max1i = 0;
    int max2i = 0;
    int frame_size = w*h;
    int frame_size_2 = 2*w*h;
    PutFileHeader(p_plt_f, w, h, pixel_height);
    CImg<float> dest  = CImg<>(w, h, d, 1);
    int dest_size = dest.size();
    float *p_dest = dest.data();
    float *p_src = img.data();
    for(int i=0; i<w; i++) {
        for(int j=0; j<h; j++) {
            p_dest[w*j+i] = p_src[w*j+i]*0.299 + 
                            p_src[w*j+i+frame_size]*0.587 + 
                            p_src[w*j+i+frame_size_2]*0.114;
        }
    }
    std::fprintf(stderr,"Image  Width: %d\n", img.width());
    std::fprintf(stderr,"Image Height: %d\n", img.height());
    unsigned char t = 0;
    for(int i=0; i<256; i++) {
        pb_cnts[i] = 0;
    }
    for(int i=0; i<dest_size; i++) {
        t = p_dest[i];
        pb_cnts[t] ++;
    }
    for(int i=0; i<256; i++) {
        if((max2<pb_cnts[i]) && (max1>pb_cnts[i])) {
            max2 = pb_cnts[i];
            max2i = i;
        }
        if((max1<pb_cnts[i])) {
            max2 = max1;
            max2i = max1i;
            max1 = pb_cnts[i];
            max1i = i;
        }
    }
    int b_len = 0;
    int start_i = 0;
    float th = abs(max2i - max1i)/2;
    for(int j=0; j<h; j++) {
        for(int i=0; i<w; i++) {
            if((start_i==0) && (p_dest[w*j+i]<th)) {
               start_i = i; 
               b_len = 1;
            }
            else if((start_i!=0) && (p_dest[w*j+i]<th)) {
                b_len ++;
            }
            else if(start_i!=0) {
                PutFileRec(p_plt_f, h, start_i, j, b_len, pixel_height);
                b_len = 0;
                start_i = 0;
            }
        }
        if(start_i!=0) {
            PutFileRec(p_plt_f, h, start_i, j, b_len, pixel_height);
            b_len = 0;
            start_i = 0;
        }
    }
    std::fprintf(stderr,"Write %s completed!\n", file_o);
    delete pb_cnts;
    std::fclose(p_plt_f);
    return 0;
}

