/*=========================================================================

  Program:   Image Basic Processing Structures and Functions
  Module:    $RCSfile: y_img.h,v $
  Language:  C++
  Date:      $Date: 2012/02/08 $
  Version:   $Revision: 1.50 $

  Developed by Yang Yu (yuy@janelia.hhmi.org)

=========================================================================*/
#ifndef __Y_IMG_H__
#define __Y_IMG_H__

//
//  1) basic variable/vector/image functions
//  2) Matrix class
//  3) Image class
//  4) Marker class
//

#include <string>
#include <algorithm>
#include <vector>
using namespace std;

#include <cmath>
#include <ctime>

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <fstream>
using std::ifstream;
using std::ofstream;

#include <sstream>
using std::istringstream;
using std::ostringstream;

#ifndef PI
#define PI 3.14159265358979323846
#endif

#ifndef INF
#define INF 1E10
#endif

#ifndef EPS
#define EPS 1E-8
#endif

#ifndef MAXITERS
#define MAXITERS 100
#endif

#ifndef FUNCONVTOL
#define FUNCONVTOL 1E-4
#endif

#ifndef EMPTY
#define EMPTY 1
#endif

typedef float REAL; // precision switch
typedef enum { UNKNOWNDT=0, UINT8=1, UINT16=2, FLOAT32=4, FLOAT64=8 } DataType;
typedef enum { UNKNOWNDK, IMAGE, FIELD } DataKind;
typedef enum { UNKNOWNDIMTYPE, D1D, D2D, D3D, D4D, D5D, D6D, D7D, D8D, D9D, DND } DimType;
typedef enum { UNKNOWNAT, AX, AY, AZ, AC, AT } AxisType;

//
// 64bit long
#if defined(_MSC_VER) && (_WIN64)
#define LONG64 long long
#else
#define LONG64 long
#endif

//
#define foreach(count, iter) 	\
    for(LONG64 iter=0; iter<count; iter++)

//-------------------------------------------------------------------------------
/// basic functions
//-------------------------------------------------------------------------------

// max
template <class T> T y_max(T x, T y);

// min
template <class T> T y_min(T x, T y);

// abs
template <class T> T y_abs(T x);

// delete 1d pointer
template <class T> void y_del(T *&p);

// delete 2d pointers
template<class Tdata, class Tidx> void y_del2d(Tdata **&p, Tidx column);

// delete 2 1d pointers
template<class T1, class T2> void y_del2(T1 *&p1, T2 *&p2);

// new 1d pointer
template<class T, class Tidx> void y_new(T *&p, Tidx N);

// sign of 1 variable
template<class T> T y_sign(T x);

// sign of 2 variables
template<class T> T y_sign2v(T a, T b);

// quick sort
template <class Tdata, class Tidx> void quickSort(Tdata *a, Tidx l, Tidx r, Tidx N);

// dist
template<class T> T y_dist(T x1, T y1, T z1, T x2, T y2, T z2);


//-------------------------------------------------------------------------------
/// Image Processing Functions
//
//  gaussian filter, thresholding, normalizing, gradient magnitude, extract surfaces, ...
//
//-------------------------------------------------------------------------------

template<class Tdata, class Tidx>
class HistogramLUT
{
public:
    HistogramLUT(){bins=0; index=0; lut=NULL;}
    ~HistogramLUT(){}

public:
    void initLUT(Tdata *p, Tidx sz, Tidx nbins);
    Tidx getIndex(Tdata val);

public:
    Tidx bins, index;
    Tdata *lut; // look up table
    REAL minv, maxv;
};

//ANSI C codes from the article "Contrast Limited Adaptive Histogram Equalization"
// by Karel Zuiderveld (karel@cv.ruu.nl) in "Graphics Gems IV", Academic Press, 1994
// modified to templates by Yang Yu (yuy@janelia.hhmi.org) 05/02/2012
template <class Tdata, class Tidx>
int CLAHE2D (Tdata* pImage, Tidx uiXRes, Tidx uiYRes, Tdata Min, Tdata Max,
             Tidx uiNR_OF_GREY, Tidx uiNrX, Tidx uiNrY, Tidx uiNrBins, REAL fCliplimit);


template <class Tidx>
void ClipHistogram2D (Tidx* pulHistogram, Tidx uiNrGreylevels, Tidx ulClipLimit);

template <class Tdata, class Tidx>
void MakeHistogram2D (Tdata* pImage, Tidx uiXRes, Tidx uiSizeX, Tidx uiSizeY,
                      Tidx* &pulHistogram, Tidx uiNrGreylevels, Tdata* pLookupTable);

template <class Tdata, class Tidx>
void MapHistogram2D (Tidx* pulHistogram, Tdata Min, Tdata Max, Tidx uiNrGreylevels, Tidx ulNrOfPixels);

template <class Tdata, class Tidx>
void MakeLut2D (Tdata * pLUT, Tdata Min, Tdata Max, Tidx uiNrBins);

template <class Tdata, class Tidx>
void Interpolate2D (Tdata * pImage, Tidx uiXRes, Tidx * pulMapLU, Tidx * pulMapRU, Tidx * pulMapLB,
                    Tidx * pulMapRB, Tidx uiXSize, Tidx uiYSize, Tdata * pLUT);

// contrast enhancement using CLAHE
template <class Tdata, class Tidx>
void contrastEnhancing(Tdata *&p, Tidx *dims, Tidx datatype, Tidx szblock, Tidx histbins, REAL maxslope);

// gaussian filtering
template <class Tdata, class Tidx> void guassianFiltering(Tdata *&p, Tidx *dims, Tidx *w);

// thresholding
template <class Tdata, class Tidx> void thresholding(Tdata *&p, Tidx *dims);

// computing image's max and min values
template <class Tdata, class Tidx> void imaxmin(Tdata *p, Tidx totalplxs, Tdata &max_v, Tdata &min_v);

// rescaling
template <class Tdata, class Tidx> void rescaling(Tdata *&p, Tidx totalplxs, Tdata lv, Tdata rv);

// normalizing
template <class Tdata, class Tidx> void normalizing(Tdata *&p, Tidx totalplxs, Tdata lv, Tdata rv);

// gradient magnitude
template <class Tdata, class Tidx> void gradientMagnitude(Tdata *&p, Tidx *dims);

// surface extracting
template <class Tdata, class Tidx> void extractsurfaces(Tdata *&p, Tidx *dims);

// flip
template <class Tdata, class Tidx> void flip(Tdata *&pOut, Tdata *pIn, Tidx *dims, AxisType atype);

// image copy
template <class Tdata, class Tidx> void imgcopy(Tdata *&pOut, Tdata *pIn, Tidx totalplxs);

// tps basic function
template <class Tdata> Tdata tpsRBF(Tdata r);

// image intersection
template <class Tdata, class Tidx> void imgintersection(Tdata *&pOut, Tdata *p1, Tdata *p2, Tidx totalplxs, Tidx method, Tdata ceilingval);

// image blank plane cutting
template <class Tdata, class Tidx> void imgcutting(Tdata *&pOutput, Tidx* szOutput, Tdata *pInput, Tidx *szInput, REAL threshold, Tidx left_x, Tidx right_x, Tidx up_y, Tidx down_y, Tidx front_z, Tidx back_z);

//-------------------------------------------------------------------------------
/// Matrix class
//
//  Definition:
//
//  Mrxc
//  ----------> r
//  |
//  |
//  |
//  c
//
//  c++ code style: for(j in c) for(i in r) cout<<M.v[j][i]<<endl;
//
//  Anxm * Bdxn = Cdxm
//  c++ code example: B.prod(A); C.copy(B); A.clean(); B.clean();
//
//-------------------------------------------------------------------------------

template<class Tdata, class Tidx>
class Y_MAT
{
public:
    Y_MAT();
    Y_MAT(Tdata *pInput, Tidx dimx, Tidx dimy);

    ~Y_MAT();

public:
    void init(Tidx dimx, Tidx dimy, Tdata value);
    void clone(Y_MAT pIn, bool t);
    void copy(Y_MAT M);
    void clean();
    void zeros();
    void unit();

    void add(Y_MAT M);
    void prod(Y_MAT M);
    void transpose();
    void pseudoinverse();

    Tdata pythagorean(Tdata a, Tdata b);
    void svd(Y_MAT U, Y_MAT S, Y_MAT V);
    void lu(Y_MAT a, Y_MAT b);

    void vectorMul(REAL *in, REAL *&out);
    void normalize3DAffine(Y_MAT M);
    void getRigid(Y_MAT M1, Y_MAT M2);

    void assign(const Y_MAT M) const;
    void setVal(Tidx r, Tidx c, Tdata val);

    void sortEigenVectors(Y_MAT E);
    void sortEigenValues();

public:
    Tdata *p;
    Tdata **v;
    Tidx row, column;

};

//-------------------------------------------------------------------------------
/// nd image (XYZCT...) structure
//-------------------------------------------------------------------------------

// nD image class
template <class Tdata, class Tidx>
class Y_IMAGE
{
public:
    Y_IMAGE();
    ~Y_IMAGE();

public:
    void clean();
    void setImage(Tdata *p, Tidx *sz, Tidx imgdim);
    void initImage(Tidx *sz, Tidx imgdim); // init a zero image
    void initField(Tidx *sz, Tidx imgdim); // init a zero field
    void set4doffsets();
    void getMassCenter();
    void getMeanIntensityValue();
    void setRefChn(Tidx refcinput);
    void getBoundingBox();
    void zeros();
    Tdata val4d(Tidx z, Tidx y, Tidx x, Tidx v);
    Tidx idx4d(Tidx z, Tidx y, Tidx x, Tidx v);

public:
    Tdata *pImg;
    Tidx *dims;

    DataKind dk; // image [c][z][y][x] field [z][y][x][v]
    DimType dimt; // 1D/2D/3D/4D
    DataType dt;

    Tidx refc; // reference color channel, 0 by default

    Tidx totalplxs;
    Tidx of3, of2, of1; // efficient index
    REAL *centroid; // mass center
    REAL *means; // mean intensity values
    Tidx bbx, bex, bby, bey, bbz, bez; // bounding box

    Tidx m; // number of feature points
};

//-------------------------------------------------------------------------------
/// 3d feature point(s) structure
//-------------------------------------------------------------------------------

// 3d landmark class
template <class Tidx>
class Y_MARKER
{
public:
    Y_MARKER();
    ~Y_MARKER();

public:
    REAL len();

public:
    Tidx x, y, z;
    bool visited;
};

typedef vector<Y_MARKER<double> > Y_LANDMARK;
typedef vector<Y_MARKER<LONG64> > Y_ROI;

//-------------------------------------------------------------------------------
#include "y_img.hxx"
//-------------------------------------------------------------------------------

#endif //__Y_IMG_H__
