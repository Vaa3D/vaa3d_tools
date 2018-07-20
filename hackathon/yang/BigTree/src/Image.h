// Image.h

//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <set>
#include <time.h>
#include <errno.h>
#include <queue>
#include <dirent.h>
#include <cstring>
#include <math.h>
#include <chrono>
#include <map>
#include <iterator>

#include <omp.h>

#include "tiffio.h"
#include "tiffio.hxx"

using namespace std;

//
#define HALVE_BY_MEAN 1
#define HALVE_BY_MAX  2
#define TMITREE_MAX_HEIGHT 10

// Baseline TIFF Tag Compression
//COMPRESSION_NONE = 1;
//COMPRESSION_CCITTRLE = 2;
//COMPRESSION_LZW = 5;
//COMPRESSION_PACKBITS = 32773;
//COMPRESSION_ADOBE_DEFLATE = 8;
//COMPRESSION_DEFLATE = 32946;
//COMPRESSION_JP2000 = 34712;
//#define COMPPRESSION_METHOD COMPRESSION_JP2000
//#define COMPPRESSION_METHOD COMPRESSION_CCITTRLE

#define COMPPRESSION_METHOD COMPRESSION_LZW
//#define COMPPRESSION_METHOD COMPRESSION_PACKBITS
//#define COMPPRESSION_METHOD COMPRESSION_ADOBE_DEFLATE


enum  axis { vertical=1, inv_vertical=-1, horizontal=2, inv_horizontal=-2, depth=3, inv_depth=-3, axis_invalid=0 };

//
template<class T>
void del1dp(T *&p)
{
    if(p) {delete []p; p=NULL;}
    return;
}

//
void ssclear(stringstream *stringStreamInMemory);

//
int makeDir(const char *filepath);

//
char *copyFile(const char *srcFile, const char *dstFile);

//
float fastmax(const std::vector<float>& v);

//
void halveSample(uint8* img, int height, int width, int depth, int method, int bytes_chan);

//
void halveSample2D(uint8* img, int height, int width, int depth, int method, int bytes_chan);

//
void copydata(unsigned char *psrc, uint32 stride_src, unsigned char *pdst, uint32 stride_dst, uint32 width, uint32 len);

//
void swap2bytes(void *targetp);

//
void swap4bytes(void *targetp);

//
char *loadTiffMetaInfo(char* filename, uint32 &sz0, uint32  &sz1, uint32  &sz2, uint32  &sz3, uint16 &datatype);

//
void readTiff(stringstream *dataStreamInMemory, unsigned char *&img, unsigned int img_width, unsigned int img_height, unsigned int first, unsigned int last, int starti = -1, int endi = -1, int startj = -1, int endj = -1);

//
char *initTiff3DFile(char *filename, int sz0, int  sz1, int  sz2, int  sz3, int datatype);

//
int openTiff3DFile(char *filename, char *mode, void *&fhandle, bool reopen);

//
char *appendSlice2Tiff3DFile(void *fhandler, int slice, unsigned char *img, unsigned int  img_width, unsigned int  img_height, int spp, int bpp, int NPages);

//
int writeTiff3DFile(char* filename, uint8 *img, int x, int y, int z, int c, int datatype);

//
int tiffIOTest(char* inputFileName, char *outputFileName, int compressionMethod);

//
int compareString(const char *ptr0, const char *ptr1, int len);
