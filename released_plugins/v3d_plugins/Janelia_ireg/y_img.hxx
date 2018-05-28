/*=========================================================================

  Program:   Image Basic Processing Structures and Functions
  Module:    $RCSfile: y_img.hxx,v $
  Language:  C++
  Date:      $Date: 2012/02/08 $
  Version:   $Revision: 1.50 $

  Developed by Yang Yu (yuy@janelia.hhmi.org)

=========================================================================*/
#ifndef __Y_IMG_HXX
#define __Y_IMG_HXX

#include "y_img.h"

//-------------------------------------------------------------------------------
/// basic functions
//-------------------------------------------------------------------------------

// max
template <class T>
T y_max(T x, T y)
{
    return (x>y)?x:y;
}

// min
template <class T>
T y_min(T x, T y)
{
    return (x<y)?x:y;
}

// abs
template <class T>
T y_abs(T x)
{
    return (x<(T)0)?-x:x;
}

// delete 1d pointer
template <class T>
void y_del(T *&p)
{
    if(p) {delete []p; p=NULL;}
    return;
}

// delete 2d pointers
template<class Tdata, class Tidx>
void y_del2d(Tdata **&p, Tidx column)
{
    if(p)
    {
        for(Tidx i=0; i<column; i++)
        {
            delete[] p[i];
        }
        delete []p; p=NULL;
    }
    return;
}

// delete 2 1d pointers
template<class T1, class T2>
void y_del2(T1 *&p1, T2 *&p2)
{
    y_del<T1>(p1);
    y_del<T2>(p2);
    return;
}

// new 1d pointer
template<class T, class Tidx>
void y_new(T *&p, Tidx N)
{
    //
    y_del<T>(p);

    //
    try
    {
        p = new T [N];
    }
    catch(...)
    {
        cout<<"Attempt to allocate memory failed!"<<endl;
        y_del<T>(p);
        return;
    }
    return;
}

// sign
template<class T>
T y_sign(T x)
{
    return ((x>=(T)0) ? (T)1 : (T)(-1));
}

template<class T>
T y_sign2v(T a, T b)
{
    return (y_sign<T>(b)*y_abs<T>(a));
}

// vector
template <class Tdata, class Tidx>
void y_new4dvec(Tdata **** & v, Tidx *sz)
{
    if(!sz)
    {
        cout<<"Error: the input pointer is invalid"<<endl;
        return;
    }

    try
    {
        v = new Tdata *** [ sz[3] ];

        for(Tidx i=0; i<sz[3]; i++)
        {
            v[i] = new Tdata ** [ sz[2] ];
            for(Tidx j=0; j<sz[2]; j++)
            {
                v[i][j] = new Tdata * [ sz[1] ];
                for(Tidx k=0; k<sz[1]; k++)
                {
                    v[i][j][k] = new Tdata [ sz[0] ];
                }
            }
        }
    }
    catch(...)
    {
        cout<<"Attempt to allocate memory failed!"<<endl;
        return;
    }

    return;
}

template <class Tdata, class Tidx>
void y_del4dvec(Tdata **** & v, Tidx *sz)
{
    if(!v) return;

    for(Tidx k = 0; k < sz[2]; k++)
    {
        for(Tidx j = 0; j < sz[1]; j++)
        {
            for(Tidx i = 0; i < sz[0]; i++)
            {
                delete [] v[k][j][i];
            }
            delete [] v[k][j];
        }
        delete [] v[k];
    }
    delete [] v;
    v=0;

    return;
}

template <class Tdata, class Tidx>
void y_ptr2vec(Tdata **** & v, Tdata *p, Tidx *sz)
{
    if(!p || !sz)
    {
        cout<<"Error: the input pointer is invalid"<<endl;
        return;
    }

    Tidx koffset = sz[2]*sz[1]*sz[0];
    Tidx joffset = sz[1]*sz[0];

    for(Tidx iz=0; iz<sz[3]; iz++)
    {
        Tidx offset_z = iz*koffset;
        for(Tidx iy=0; iy<sz[2]; iy++)
        {
            Tidx offset_y = offset_z + iy*joffset;
            for(Tidx ix=0; ix<sz[1]; ix++)
            {
                Tidx offset_x = offset_y + ix*sz[0];
                for(Tidx iv=0; iv<sz[0]; iv++)
                {
                    v[iz][iy][ix][iv] = p[offset_x+iv];
                }
            }
        }
    }

    return;
}

template <class Tdata, class Tidx>
void y_vec2ptr(Tdata **** v, Tdata * & p, Tidx *sz)
{
    if(!v || !sz)
    {
        cout<<"Error: the input pointer is invalid"<<endl;
        return;
    }

    Tidx koffset = sz[2]*sz[1]*sz[0];
    Tidx joffset = sz[1]*sz[0];

    for(Tidx iz=0; iz<sz[3]; iz++)
    {
        Tidx offset_z = iz*koffset;
        for(Tidx iy=0; iy<sz[2]; iy++)
        {
            Tidx offset_y = offset_z + iy*joffset;
            for(Tidx ix=0; ix<sz[1]; ix++)
            {
                Tidx offset_x = offset_y + ix*sz[0];
                for(Tidx iv=0; iv<sz[0]; iv++)
                {
                    p[offset_x+iv] = v[iz][iy][ix][iv];
                }
            }
        }
    }

    return;
}

template <class Tdata, class Tidx>
void y_vec4dproxy(Tdata **** & v, Tdata *p, Tidx *sz)
{
    if(!p || !sz)
    {
        cout<<"Error: the input pointer is invalid"<<endl;
        return;
    }

    if(v) { y_del4dvec<Tdata, Tidx>(v, sz); }

    //
    Tidx koffset = sz[2]*sz[1]*sz[0];
    Tidx joffset = sz[1]*sz[0];

    try
    {
        v = new Tdata *** [ sz[3] ];
        for(Tidx iz=0; iz<sz[3]; iz++)
        {
            Tidx offset_z = iz*koffset;

            v[iz] = new Tdata ** [ sz[2] ];
            for(Tidx iy=0; iy<sz[2]; iy++)
            {
                Tidx offset_y = offset_z + iy*joffset;

                v[iz][iy] = new Tdata * [ sz[1] ];
                for(Tidx ix=0; ix<sz[1]; ix++)
                {
                    Tidx offset_x = offset_y + ix*sz[0];

                    v[iz][iy][ix] = p + offset_x;
                }
            }
        }
    }
    catch(...)
    {
        cout<<"Fail to allocate memory"<<endl;
        return;
    }
}

template <class Tdata, class Tidx>
void y_del4dproxy(Tdata **** & v, Tidx *sz)
{
    if(!v) return;

    for(Tidx k = 0; k < sz[3]; k++)
    {
        for(Tidx j = 0; j < sz[2]; j++)
        {
            delete [] v[k][j];
        }
        delete [] v[k];
    }
    delete [] v;
    v=0;

    return;
}

template <class Tdata, class Tidx>
void quickSort(Tdata *a, Tidx l, Tidx r, Tidx N)
{
    //
    /// sort by descent order
    //

    if(l>=r) return;
    Tidx i=l;
    Tidx j=r+1;

    Tdata pivot = a[l];

    while(true)
    {
        do{ i = i+1; } while(a[i]>pivot);
        do{ j = j-1; } while(a[j]<pivot);

        if(i>=j) break;

        //swap
        Tdata tmp;
        tmp = a[i]; a[i] = a[j]; a[j] = tmp; //
    }

    //
    a[l] = a[j];
    a[j] = pivot;

    //
    quickSort(a, l, j-1, N);
    quickSort(a, j+1, r, N);
}

// dist
template<class T>
T y_dist(T x1, T y1, T z1, T x2, T y2, T z2)
{
    return (sqrt( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2) ));
}


//-------------------------------------------------------------------------------
/// Image Processing Functions
//-------------------------------------------------------------------------------

template<class Tdata, class Tidx>
void HistogramLUT<Tdata, Tidx> :: initLUT(Tdata *p, Tidx sz, Tidx nbins)
{
    //
    if(!p || nbins<=0)
    {
        cout<<"Invalid inputs"<<endl;
        return;
    }

    //
    bins = nbins;
    y_new<Tdata, Tidx>(lut, bins);

    // histogram bin #i [minv+i*stepv, minv+(i+1)*stepv)
    minv=INF;
    maxv=-INF;
    REAL stepv=0;
    for(Tidx i=0; i<sz; i++)
    {
        if(minv>p[i]) minv=p[i];
        if(maxv<p[i]) maxv=p[i];
    }
    stepv = (maxv - minv)/(REAL)bins;

    for(Tidx i=0; i<bins; i++)
    {
        lut[i] = minv + i*stepv; // only left values
    }

    return;
}

template<class Tdata, class Tidx>
Tidx HistogramLUT<Tdata, Tidx> :: getIndex(Tdata val)
{
    //
    Tidx min=0;
    Tidx max=bins-1;

    // binary search
    bool found = false;
    index=0;
    while(min<max && !found)
    {
        Tidx mid=(min+max)/2;

        if(val == lut[mid] || (val > lut[mid] && y_abs<Tdata>(val - lut[mid])<EPS) ) // =
        {
            found = true;
            index = mid;
        }
        else if(val < lut[mid]) // <
        {
            max = mid - 1;
        }
        else // >
        {
            if(mid+1>=max)
            {
                found = true;
                index = mid;
            }
            else
            {
                min = mid + 1;
            }
        }
    }

    //
    return index;
}

/*** start c codes for CLAHE ***/
//ANSI C codes from the article "Contrast Limited Adaptive Histogram Equalization"
// by Karel Zuiderveld (karel@cv.ruu.nl) in "Graphics Gems IV", Academic Press, 1994
// modified to templates by Yang Yu (yuy@janelia.hhmi.org) 05/02/2012
template <class Tdata, class Tidx>
int CLAHE2D (Tdata* pImage, Tidx uiXRes, Tidx uiYRes,
             Tdata Min, Tdata Max, Tidx uiNR_OF_GREY, Tidx uiNrX, Tidx uiNrY,
             Tidx uiNrBins, REAL fCliplimit)
/*   pImage - Pointer to the input/output image
 *   uiXRes - Image resolution in the X direction
 *   uiYRes - Image resolution in the Y direction
 *   Min - Minimum greyvalue of input image (also becomes minimum of output image)
 *   Max - Maximum greyvalue of input image (also becomes maximum of output image)
 *   uiNR_OF_GREY > Max - for 8-bit data is 256, 12-bit data is 4096
 *   uiNrX - Number of contextial regions in the X direction (min 2, max uiMAX_REG_X)
 *   uiNrY - Number of contextial regions in the Y direction (min 2, max uiMAX_REG_Y)
 *   uiNrBins - Number of greybins for histogram ("dynamic range")
 *   fCliplimit - Normalized cliplimit (higher values give more contrast)
 * The number of "effective" greylevels in the output image is set by uiNrBins; selecting
 * a small value (eg. 128) speeds up processing and still produce an output image of
 * good quality. The output image will have the same minimum and maximum value as the input
 * image. A clip limit smaller than 1 results in standard (non-contrast limited) AHE.
 */
{
    unsigned int uiX, uiY;		  /* counters */
    unsigned int uiXSize, uiYSize, uiSubX, uiSubY; /* size of context. reg. and subimages */
    unsigned int uiXL, uiXR, uiYU, uiYB;  /* auxiliary variables interpolation routine */
    Tidx ulClipLimit, ulNrPixels;/* clip limit and region pixel count */
    Tdata* pImPointer;		   /* pointer to image */
    Tdata *aLUT=NULL;	    /* lookup table used for scaling of input image */
    Tidx* pulHist, *pulMapArray=NULL; /* pointer to histogram and mappings*/
    Tidx* pulLU, *pulLB, *pulRU, *pulRB; /* auxiliary pointers interpolation */

    y_new<Tdata, Tidx>(aLUT, uiNR_OF_GREY);

    Tidx uiMAX_REG_X = uiXRes; // dimx
    Tidx uiMAX_REG_Y = uiYRes; // dimy

    if (uiNrX > uiMAX_REG_X) return -1;	   /* # of regions x-direction too large */
    if (uiNrY > uiMAX_REG_Y) return -2;	   /* # of regions y-direction too large */
    if (uiXRes % uiNrX) return -3;	  /* x-resolution no multiple of uiNrX */
    if (uiYRes % uiNrY) return -4;	  /* y-resolution no multiple of uiNrY */
    if (Max >= uiNR_OF_GREY) return -5;	   /* maximum too large */
    if (Min >= Max) return -6;		  /* minimum equal or larger than maximum */
    if (uiNrX < 2 || uiNrY < 2) return -7;/* at least 4 contextual regions required */
    if (fCliplimit == 1.0) return 0;	  /* is OK, immediately returns original image. */
    if (uiNrBins == 0) uiNrBins = 128;	  /* default value when not specified */

    y_new<Tidx, Tidx>(pulMapArray, uiNrX*uiNrY*uiNrBins);

    uiXSize = uiXRes/uiNrX; uiYSize = uiYRes/uiNrY;  /* Actual size of contextual regions */
    ulNrPixels = uiXSize * uiYSize;

    if(fCliplimit > 0.0) {		  /* Calculate actual cliplimit	 */
        ulClipLimit = fCliplimit * (uiXSize * uiYSize) / uiNrBins;
        ulClipLimit = (ulClipLimit < 1UL) ? 1UL : ulClipLimit;
    }
    else ulClipLimit = 1UL<<14;		  /* Large value, do not clip (AHE) */
    MakeLut2D<Tdata, Tidx>(aLUT, Min, Max, uiNrBins);	  /* Make lookup table for mapping of greyvalues */

    /* Calculate greylevel mappings for each contextual region */
    for (uiY = 0, pImPointer = pImage; uiY < uiNrY; uiY++) {
        for (uiX = 0; uiX < uiNrX; uiX++, pImPointer += uiXSize) {

            pulHist = &pulMapArray[uiNrBins * (uiY * uiNrX + uiX)];

            MakeHistogram2D<Tdata, Tidx>(pImPointer,uiXRes,uiXSize,uiYSize,pulHist,uiNrBins,aLUT);
            ClipHistogram2D<Tidx>(pulHist, uiNrBins, ulClipLimit);
            MapHistogram2D<Tdata, Tidx>(pulHist, Min, Max, uiNrBins, ulNrPixels);
        }

        pImPointer += (uiYSize - 1) * uiXRes;		  /* skip lines, set pointer */
    }

    /* Interpolate greylevel mappings to get CLAHE image */
    for (pImPointer = pImage, uiY = 0; uiY <= uiNrY; uiY++)
    {
        //
        if (uiY == 0) {					  /* special case: top row */
            uiSubY = uiYSize >> 1;  uiYU = 0; uiYB = 0;
        }
        else {
            if (uiY == uiNrY) {				  /* special case: bottom row */
                uiSubY = uiYSize >> 1;	uiYU = uiNrY-1;	 uiYB = uiYU;
            }
            else {					  /* default values */
                uiSubY = uiYSize; uiYU = uiY - 1; uiYB = uiYU + 1;
            }
        }
        for (uiX = 0; uiX <= uiNrX; uiX++) {
            if (uiX == 0) {				  /* special case: left column */
                uiSubX = uiXSize >> 1; uiXL = 0; uiXR = 0;
            }
            else {
                if (uiX == uiNrX) {			  /* special case: right column */
                    uiSubX = uiXSize >> 1;  uiXL = uiNrX - 1; uiXR = uiXL;
                }
                else {					  /* default values */
                    uiSubX = uiXSize; uiXL = uiX - 1; uiXR = uiXL + 1;
                }
            }

            pulLU = &pulMapArray[uiNrBins * (uiYU * uiNrX + uiXL)];
            pulRU = &pulMapArray[uiNrBins * (uiYU * uiNrX + uiXR)];
            pulLB = &pulMapArray[uiNrBins * (uiYB * uiNrX + uiXL)];
            pulRB = &pulMapArray[uiNrBins * (uiYB * uiNrX + uiXR)];
            Interpolate2D<Tdata, Tidx>(pImPointer,uiXRes,pulLU,pulRU,pulLB,pulRB,uiSubX,uiSubY,aLUT);
            pImPointer += uiSubX;			  /* set pointer on next matrix */
        }
        pImPointer += (uiSubY - 1) * uiXRes;
    }

    // de-alloc
    y_del<Tdata>(aLUT);
    y_del<Tidx>(pulMapArray);

    //
    return 0;						  /* return status OK */
}

template <class Tidx>
void ClipHistogram2D (Tidx* pulHistogram, Tidx uiNrGreylevels, Tidx ulClipLimit)
/* This function performs clipping of the histogram and redistribution of bins.
 * The histogram is clipped and the number of excess pixels is counted. Afterwards
 * the excess pixels are equally redistributed across the whole histogram (providing
 * the bin count is smaller than the cliplimit).
 */
{
    Tidx* pulBinPointer, *pulEndPointer, *pulHisto;
    Tidx ulNrExcess, ulUpper, ulBinIncr, ulStepSize, i;
    long lBinExcess;

    ulNrExcess = 0;  pulBinPointer = pulHistogram;

    for (i = 0; i < uiNrGreylevels; i++) { /* calculate total number of excess pixels */
        lBinExcess = (long)pulBinPointer[i] - (long)ulClipLimit;
        if (lBinExcess > 0) ulNrExcess += lBinExcess;	  /* excess in current bin */
    };


    /* Second part: clip histogram and redistribute excess pixels in each bin */
    ulBinIncr = ulNrExcess / uiNrGreylevels;		  /* average binincrement */
    ulUpper =  ulClipLimit - ulBinIncr;	 /* Bins larger than ulUpper set to cliplimit */

    for (i = 0; i < uiNrGreylevels; i++) {
        if (pulHistogram[i] > ulClipLimit) pulHistogram[i] = ulClipLimit; /* clip bin */
        else {
            if (pulHistogram[i] > ulUpper) {		/* high bin count */
                ulNrExcess -= pulHistogram[i] - ulUpper; pulHistogram[i]=ulClipLimit;
            }
            else {					/* low bin count */
                ulNrExcess -= ulBinIncr; pulHistogram[i] += ulBinIncr;
            }
        }
    }

    while (ulNrExcess) {   /* Redistribute remaining excess  */
        pulEndPointer = &pulHistogram[uiNrGreylevels]; pulHisto = pulHistogram;

        while (ulNrExcess && pulHisto < pulEndPointer) {
            ulStepSize = uiNrGreylevels / ulNrExcess;
            if (ulStepSize < 1) ulStepSize = 1;		  /* stepsize at least 1 */
            for (pulBinPointer=pulHisto; pulBinPointer < pulEndPointer && ulNrExcess;
                 pulBinPointer += ulStepSize) {
                if (*pulBinPointer < ulClipLimit) {
                    (*pulBinPointer)++;	 ulNrExcess--;	  /* reduce excess */
                }
            }
            pulHisto++;		  /* restart redistributing on other bin location */
        }
    }
}

template <class Tdata, class Tidx>
void MakeHistogram2D (Tdata* pImage, Tidx uiXRes,
                      Tidx uiSizeX, Tidx uiSizeY,
                      Tidx* &pulHistogram,
                      Tidx uiNrGreylevels, Tdata* pLookupTable)
/* This function classifies the greylevels present in the array image into
 * a greylevel histogram. The pLookupTable specifies the relationship
 * between the greyvalue of the pixel (typically between 0 and 4095) and
 * the corresponding bin in the histogram (usually containing only 128 bins).
 */
{
    Tdata* pImagePointer;
    unsigned int i;

    for (i = 0; i < uiNrGreylevels; i++) pulHistogram[i] = 0L; /* clear histogram */

    for (i = 0; i < uiSizeY; i++) {
        pImagePointer = &pImage[uiSizeX];
        while (pImage < pImagePointer) pulHistogram[pLookupTable[*pImage++]]++;
        pImagePointer += uiXRes;
        pImage = &pImagePointer[-uiSizeX];
    }
}

template <class Tdata, class Tidx>
void MapHistogram2D (Tidx* pulHistogram, Tdata Min, Tdata Max,
                     Tidx uiNrGreylevels, Tidx ulNrOfPixels)
/* This function calculates the equalized lookup table (mapping) by
 * cumulating the input histogram. Note: lookup table is rescaled in range [Min..Max].
 */
{
    Tidx i;
    Tidx ulSum = 0;
    REAL fScale = ((REAL)(Max - Min)) / ulNrOfPixels;
    Tidx ulMin = (Tidx)Min;

    for (i = 0; i < uiNrGreylevels; i++)
    {
        ulSum += pulHistogram[i];
        pulHistogram[i] = ulMin+ulSum*fScale;
        if (pulHistogram[i] > Max) pulHistogram[i] = Max;
    }
}

template <class Tdata, class Tidx>
void MakeLut2D (Tdata * pLUT, Tdata Min, Tdata Max, Tidx uiNrBins)
/* To speed up histogram clipping, the input image [Min,Max] is scaled down to
 * [0,uiNrBins-1]. This function calculates the LUT.
 */
{
    Tidx i;
    Tdata BinSize = (Tdata) (1 + (Max - Min) / uiNrBins);

    for (i = Min; i <= Max; i++)  pLUT[i] = (i - Min) / BinSize;
}

template <class Tdata, class Tidx>
void Interpolate2D (Tdata * pImage, Tidx uiXRes, Tidx * pulMapLU,
                    Tidx * pulMapRU, Tidx * pulMapLB,  Tidx * pulMapRB,
                    Tidx uiXSize, Tidx uiYSize, Tdata * pLUT)
/* pImage      - pointer to input/output image
 * uiXRes      - resolution of image in x-direction
 * pulMap*     - mappings of greylevels from histograms
 * uiXSize     - uiXSize of image submatrix
 * uiYSize     - uiYSize of image submatrix
 * pLUT	       - lookup table containing mapping greyvalues to bins
 * This function calculates the new greylevel assignments of pixels within a submatrix
 * of the image with size uiXSize and uiYSize. This is done by a bilinear interpolation
 * between four different mappings in order to eliminate boundary artifacts.
 * It uses a division; since division is often an expensive operation, I added code to
 * perform a logical shift instead when feasible.
 */
{
    Tidx uiIncr = uiXRes-uiXSize; /* Pointer increment after processing row */

    Tdata GreyValue;
    unsigned int uiNum = (unsigned int)uiXSize*(unsigned int)uiYSize; /* Normalization factor */

    unsigned int uiXCoef, uiYCoef, uiXInvCoef, uiYInvCoef, uiShift = 0;

    if (uiNum & (uiNum - 1))   /* If uiNum is not a power of two, use division */
        for (uiYCoef = 0, uiYInvCoef = uiYSize; uiYCoef < uiYSize; uiYCoef++, uiYInvCoef--,pImage+=uiIncr) {
            for (uiXCoef = 0, uiXInvCoef = uiXSize; uiXCoef < uiXSize; uiXCoef++, uiXInvCoef--) {
                GreyValue = pLUT[*pImage];		   /* get histogram bin value */
                *pImage++ = (Tdata)((uiYInvCoef * (uiXInvCoef*pulMapLU[GreyValue] + uiXCoef * pulMapRU[GreyValue])
                                     + uiYCoef * (uiXInvCoef * pulMapLB[GreyValue] + uiXCoef * pulMapRB[GreyValue])) / uiNum);
            }
        }
    else {			   /* avoid the division and use a right shift instead */
        while (uiNum >>= 1) uiShift++;		   /* Calculate 2log of uiNum */
        for (uiYCoef = 0, uiYInvCoef = uiYSize; uiYCoef < uiYSize; uiYCoef++, uiYInvCoef--,pImage+=uiIncr) {
            for (uiXCoef = 0, uiXInvCoef = uiXSize; uiXCoef < uiXSize; uiXCoef++, uiXInvCoef--) {
                GreyValue = pLUT[*pImage];	  /* get histogram bin value */
                *pImage++ = (Tdata)((uiYInvCoef * (uiXInvCoef * pulMapLU[GreyValue] + uiXCoef * pulMapRU[GreyValue])
                                     + uiYCoef * (uiXInvCoef * pulMapLB[GreyValue] + uiXCoef * pulMapRB[GreyValue])) >> uiShift);
            }
        }
    }
}
/*** end c codes for CLAHE ***/

template <class Tdata, class Tidx>
void contrastEnhancing(Tdata *&p, Tidx *dims, Tidx datatype, Tidx szblock, Tidx histbins, REAL maxslope)
{
    //
    // 5D CLAHE (XYZCT)
    //

    //
    if(!p || !dims )
    {
        cout<<"Invalid inputs for contrast enhancing function"<<endl;
        return;
    }

    //
    Tidx sx = dims[0];
    Tidx sy = dims[1];
    Tidx sz = dims[2];
    Tidx sc = dims[3];

    Tidx maxIntensity;
    Tdata minv=-1, maxv=-1;

    if(datatype == 1)
    {
        maxIntensity = 256;
        minv = (Tdata)0;
        maxv = (Tdata)255;
    }
    else if(datatype = 2)
    {
        maxIntensity = 4096;
        minv = (Tdata)0;
        maxv = (Tdata)4095;
    }
    else
    {
        cout<<"Contrast Enhancing function only supports 8-bit and 12-bit data"<<endl;
        return;
    }

    Tidx pagesz = sx*sy;
    Tidx pages = sz*sc;
    for(Tidx i=0; i<pages; i++)
    {
        Tdata *pImg = p + i*pagesz;

        //
        if(CLAHE2D<Tdata, Tidx>(pImg, sx, sy, minv, maxv, maxIntensity, szblock, szblock, histbins, maxslope))
        {
            cout<<"Fail to call contrast enhancing function"<<endl;
        }
    }

    //
    return;
}

template <class Tdata, class Tidx>
void guassianFiltering(Tdata *&p, Tidx *dims, Tidx *w)
{
    //
    // Gaussian filtering
    //

    //
    if(!p || !dims || !w)
    {
        cout<<"Invalid inputs for Gaussian Filtering function"<<endl;
        return;
    }

    //
    Tidx xdim, ydim, zdim;

    xdim=dims[0];
    ydim=dims[1];
    zdim=dims[2];

    Tidx ypage, zpage;

    ypage=xdim;
    zpage=xdim*ydim;

    Tidx wx, wy, wz;

    wx=w[0];
    wy=w[1];
    wz=w[2];

    if(wx<1 || wy<1 || wz<1 || wx>xdim || wy>ydim || wz>zdim)
    {
        cout<<"Invalid Gauss kernel size"<<endl;
        return;
    }

    //   Filtering along x
    if(xdim<2)
    {
        //do nothing
    }
    else
    {
        //create Gaussian kernel
        Tdata  *weightsX = NULL;
        y_new<Tdata,Tidx>(weightsX, wx);

        Tidx half = wx >> 1;
        weightsX[half] = 1.;

        for (Tidx weight = 1; weight <= half; ++weight)
        {
            const Tdata  x = 3.* Tdata (weight) / Tdata (half);
            weightsX[half - weight] = weightsX[half + weight] = exp(-x * x / 2.);	// corresponding symmetric weightsX
        }

        Tdata k = 0.;
        for (Tidx weight = 0; weight < wx; ++weight)
            k += weightsX[weight];

        for (Tidx weight = 0; weight < wx; ++weight)
            weightsX[weight] /= k;

        //   Allocate 1-D extension array
        Tdata  *extension_bufferX = NULL;
        y_new<Tdata,Tidx>(extension_bufferX, xdim + (wx<<1));

        Tidx offset = wx>>1;
        const Tdata  *extStop = extension_bufferX + xdim + offset;

        for(Tidx iz = 0; iz < zdim; iz++)
        {
            for(Tidx iy = 0; iy < ydim; iy++)
            {
                Tdata  *extIter = extension_bufferX + wx;
                for(Tidx ix = 0; ix < xdim; ix++)
                {
                    *(extIter++) = p[iz*zpage + iy*ypage + ix];
                }

                // Extend image
                const Tdata  *const stop_line = extension_bufferX - 1;
                Tdata  *extLeft = extension_bufferX + wx - 1;
                const Tdata  *arrLeft = extLeft + 2;
                Tdata  *extRight = extLeft + xdim + 1;
                const Tdata  *arrRight = extRight - 2;

                while (extLeft > stop_line)
                {
                    *(extLeft--) = *(arrLeft++);
                    *(extRight++) = *(arrRight--);
                }

                // Filtering
                extIter = extension_bufferX + offset;

                Tdata  *resIter = p + iz*zpage + iy*ypage;

                while (extIter < extStop)
                {
                    Tdata sum = 0.;
                    const Tdata  *weightIter = weightsX;
                    const Tdata  *const End = weightsX + wx;
                    const Tdata * arrIter = extIter;
                    while (weightIter < End) sum += *(weightIter++) * Tdata (*(arrIter++));
                    extIter++;
                    *(resIter++) = sum;
                }
            }
        }

        //de-alloc
//        y_del<Tdata>(weightsX);
//        y_del<Tdata>(extension_bufferX);
    }

    //   Filtering along y
    if(ydim<2)
    {
        //do nothing
    }
    else
    {
        //create Gaussian kernel
        Tdata  *weightsY = NULL;
        y_new<Tdata, Tidx>(weightsY, wy);

        Tidx half = wy >> 1;
        weightsY[half] = 1.;

        for (Tidx weight = 1; weight <= half; ++weight)
        {
            const Tdata  y = 3.* Tdata (weight) / Tdata (half);
            weightsY[half - weight] = weightsY[half + weight] = exp(-y * y / 2.);	// corresponding symmetric weightsY
        }

        Tdata k = 0.;
        for (Tidx weight = 0; weight < wy; ++weight)
            k += weightsY[weight];

        for (Tidx weight = 0; weight < wy; ++weight)
            weightsY[weight] /= k;

        //
        Tdata  *extension_bufferY = NULL;
        y_new<Tdata,Tidx>(extension_bufferY, ydim + (wy<<1));

        Tidx offset = wy>>1;
        const Tdata *extStop = extension_bufferY + ydim + offset;

        for(Tidx iz = 0; iz < zdim; iz++)
        {
            for(Tidx ix = 0; ix < xdim; ix++)
            {
                Tdata  *extIter = extension_bufferY + wy;
                for(Tidx iy = 0; iy < ydim; iy++)
                {
                    *(extIter++) = p[iz*zpage + iy*ypage + ix];
                }

                // Extend image
                const Tdata  *const stop_line = extension_bufferY - 1;
                Tdata  *extLeft = extension_bufferY + wy - 1;
                const Tdata  *arrLeft = extLeft + 2;
                Tdata  *extRight = extLeft + ydim + 1;
                const Tdata  *arrRight = extRight - 2;

                while (extLeft > stop_line)
                {
                    *(extLeft--) = *(arrLeft++);
                    *(extRight++) = *(arrRight--);
                }

                // Filtering
                extIter = extension_bufferY + offset;

                Tdata  *resIter = p + iz*zpage + ix;

                while (extIter < extStop)
                {
                    Tdata sum = 0.;
                    const Tdata  *weightIter = weightsY;
                    const Tdata  *const End = weightsY + wy;
                    const Tdata * arrIter = extIter;
                    while (weightIter < End) sum += *(weightIter++) * Tdata (*(arrIter++));
                    extIter++;
                    *resIter = sum;
                    resIter += ypage;
                }
            }
        }

        //de-alloc
//        y_del<Tdata>(weightsY);
//        y_del<Tdata>(extension_bufferY);
    }

    // Filtering along z
    if(zdim<2)
    {
        //do nothing
    }
    else
    {
        //create Gaussian kernel
        Tdata  *weightsZ = NULL;
        y_new<Tdata, Tidx>(weightsZ, wz);

        Tidx half = wz >> 1;
        weightsZ[half] = 1.;

        for (Tidx weight = 1; weight <= half; ++weight)
        {
            const Tdata  z = 3.* Tdata (weight) / Tdata (half);
            weightsZ[half - weight] = weightsZ[half + weight] = exp(-z * z / 2.);	// corresponding symmetric weightsZ
        }

        Tdata k = 0.;
        for (Tidx weight = 0; weight < wz; ++weight)
            k += weightsZ[weight];

        for (Tidx weight = 0; weight < wz; ++weight)
            weightsZ[weight] /= k;

        //
        Tdata  *extension_bufferZ = NULL;
        y_new<Tdata, Tidx>(extension_bufferZ, zdim + (wz<<1));

        Tidx offset = wz>>1;
        const Tdata *extStop = extension_bufferZ + zdim + offset;

        for(Tidx iy = 0; iy < ydim; iy++)
        {
            for(Tidx ix = 0; ix < xdim; ix++)
            {
                Tdata  *extIter = extension_bufferZ + wz;
                for(Tidx iz = 0; iz < zdim; iz++)
                {
                    *(extIter++) = p[iz*zpage + iy*ypage + ix];
                }

                // Extend image
                const Tdata  *const stop_line = extension_bufferZ - 1;
                Tdata  *extLeft = extension_bufferZ + wz - 1;
                const Tdata  *arrLeft = extLeft + 2;
                Tdata  *extRight = extLeft + zdim + 1;
                const Tdata  *arrRight = extRight - 2;

                while (extLeft > stop_line)
                {
                    *(extLeft--) = *(arrLeft++);
                    *(extRight++) = *(arrRight--);
                }

                // Filtering
                extIter = extension_bufferZ + offset;

                Tdata  *resIter = p + iy*ypage + ix;

                while (extIter < extStop)
                {
                    Tdata sum = 0.;
                    const Tdata  *weightIter = weightsZ;
                    const Tdata  *const End = weightsZ + wz;
                    const Tdata * arrIter = extIter;
                    while (weightIter < End) sum += *(weightIter++) * Tdata (*(arrIter++));
                    extIter++;
                    *resIter = sum;
                    resIter += zpage;
                }
            }
        }

        //de-alloc
//        y_del<Tdata>(weightsZ);
//        y_del<Tdata>(extension_bufferZ);
    }

    return;
}

template <class Tdata, class Tidx>
void thresholding(Tdata *&p, Tidx *dims)
{
    //
    // threshold segmentation using k-means
    //

    //
    if(!p || !dims)
    {
        cout<<"Invalid inputs for thresholding function"<<endl;
        return;
    }

    //
    Tidx xdim, ydim, zdim;

    xdim=dims[0];
    ydim=dims[1];
    zdim=dims[2];

    Tidx pagesz;
    pagesz=xdim*ydim*zdim;

    //
    Tidx BINS = 1024; // histogram bins

    Tidx *h=NULL, *hc=NULL;
    y_new<Tidx, Tidx>(h, BINS);
    y_new<Tidx, Tidx>(hc, BINS);

    memset(h, 0, sizeof(Tidx)*BINS);

    // histogram
    HistogramLUT<Tdata, Tidx> hlut;
    hlut.initLUT(p, pagesz, BINS);
    foreach(pagesz, i)
    {
        h[hlut.getIndex(p[i])] ++;
    }

    // heuristic init center
    REAL mub=0.05*(hlut.maxv - hlut.minv) + hlut.minv;
    REAL muf=0.30*(hlut.maxv - hlut.minv) + hlut.minv;

    //
    while (true)
    {
        REAL oldmub=mub, oldmuf=muf;

        for(Tidx i=0; i<BINS; i++)
        {
            if(h[i]==0)
                continue;

            REAL cb = y_abs<REAL>(REAL(hlut.lut[i])-mub);
            REAL cf = y_abs<REAL>(REAL(hlut.lut[i])-muf);

            hc[i] = (cb<=cf)?1:2; // class 1 and class 2
        }

        // update centers
        REAL sum_b=0, sum_bw=0, sum_f=0, sum_fw=0;

        for(Tidx i=0; i<BINS; i++)
        {
            if(h[i]==0)
                continue;

            if(hc[i]==1)
            {
                sum_bw += (i+1)*h[i];
                sum_b += h[i];
            }
            else if(hc[i]==2)
            {
                sum_fw += (i+1)*h[i];
                sum_f += h[i];
            }
        }

        mub = hlut.lut[ Tidx(sum_bw/sum_b) ];
        muf = hlut.lut[ Tidx(sum_fw/sum_f) ];

        if(y_abs<REAL>(mub - oldmub)<1 && y_abs<REAL>(muf - oldmuf)<1)  break;
    }

    //
    Tdata threshold = (mub+muf)/2;
    for(Tidx i=0; i<pagesz; i++)
    {
        if(p[i]>threshold)
            p[i] = hlut.maxv;
        else
            p[i] = hlut.minv;
    }

    //
    y_del<Tidx>(h);
    y_del<Tidx>(hc);

    //
    return;
}

// computing image's max and min values
template <class Tdata, class Tidx>
void imaxmin(Tdata *p, Tidx totalplxs, Tdata &max_v, Tdata &min_v)
{
    //
    // find max and min values of the image
    //

    //
    if(!p || totalplxs<1)
    {
        cout<<"Invalid inputs for imaxmin function"<<endl;
        return;
    }

    //
    max_v = p[0];
    min_v = p[0];

    foreach(totalplxs, i)
    {
        max_v = (p[i]>max_v)? p[i] : max_v;
        min_v = (p[i]<min_v)? p[i] : min_v;
    }

    return;
}


template <class Tdata, class Tidx>
void rescaling(Tdata *&p, Tidx totalplxs, Tdata lv, Tdata rv)
{
    //
    // rescale intensity to [lv,rv]
    //

    //
    if(!p || rv<lv)
    {
        cout<<"Invalid inputs for rescaling function"<<endl;
        return;
    }

    //
    Tdata max_v, min_v;
    imaxmin<Tdata, Tidx>(p, totalplxs, max_v, min_v);
    max_v -= min_v;

    if(y_abs<REAL>(max_v)<EPS)
    {
        cout<<"Data not supported!"<<endl;
        return;
    }

    foreach(totalplxs, i)
    {
        p[i]=(rv-lv)*((REAL)(p[i]) - min_v)/max_v + lv;
    }

    return;
}

template <class Tdata, class Tidx>
void normalizing(Tdata *&p, Tidx totalplxs, Tdata lv, Tdata rv)
{
    //
    // normalize and scale intensity to [lv,rv]
    //

    //
    if(!p || rv<lv)
    {
        cout<<"Invalid inputs for normalizing function"<<endl;
        return;
    }

    //
    REAL mean_v = 0, std_v = 0;

    foreach(totalplxs, i)
    {
        mean_v += (REAL)(p[i]);
    }
    mean_v /= (REAL)totalplxs;

    foreach(totalplxs, i)
    {
        REAL val = (REAL)(p[i]) - mean_v;
        std_v += val*val;
    }
    std_v = sqrt(std_v);

    if(y_abs<REAL>(std_v)<EPS )
    {
        cout<<"Error standard deviation"<<endl;
        return;
    }

    foreach(totalplxs, i)
    {
        p[i] = ((REAL)(p[i]) - mean_v)/std_v; // real value!
    }

    //
    rescaling(p, totalplxs, lv, rv);

    //
    return;
}

template <class Tdata, class Tidx>
void gradientMagnitude(Tdata *&p, Tidx *dims, Tidx gstep = 1)
{
    //
    // extract gradient magnitude
    //

    //
    if(!p || !dims)
    {
        cout<<"Invalid inputs for gradient magnitude function"<<endl;
        return;
    }

    //
    Tidx xdim, ydim, zdim;

    xdim=dims[0];
    ydim=dims[1];
    zdim=dims[2];

    Tidx zoffsets = xdim*ydim;
    Tidx yoffsets = xdim;

    Tidx pagesz = zdim*zoffsets;

    Tdata *pData = NULL;
    y_new<Tdata, Tidx>(pData, pagesz);

    for(Tidx i=0; i<pagesz; i++)
    {
        pData[i] = p[i];
    }

    //
    REAL normfactor = -INF;
    for(Tidx z=0;z<zdim;z++)
    {
        Tidx ofz = z*zoffsets;
        for(Tidx y=0;y<ydim;y++)
        {
            Tidx ofy = ofz + y*xdim;
            for(Tidx x=0;x<xdim;x++)
            {
                Tidx idx = ofy + x;

                //
                Tidx xs, xe;
                xs = x-gstep; if(xs<0) xs=0;
                xe = x+gstep; if(xe>=xdim) xe=xdim-1;

                Tidx ys, ye;
                ys = y-gstep; if(ys<0) ys=0;
                ye = y+gstep; if(ye>=ydim) ye=ydim-1;

                Tidx zs, ze;
                zs = z-gstep; if(zs<0) zs=0;
                ze = z+gstep; if(ze>=zdim) ze=zdim-1;

                //
                REAL vxl = pData[ z*zoffsets + y*yoffsets + xs ];
                REAL vxr = pData[ z*zoffsets + y*yoffsets + xe ];

                REAL vyu = pData[ z*zoffsets + ys*yoffsets + x ];
                REAL vyd = pData[ z*zoffsets + ye*yoffsets + x ];

                REAL vzf = pData[ zs*zoffsets + y*yoffsets + x ];
                REAL vzb = pData[ ze*zoffsets + y*yoffsets + x ];

                REAL Ix = vxr - vxl;
                REAL Iy = vyd - vyu;
                REAL Iz = vzb - vzf;

                //
                REAL val = sqrt(Ix*Ix + Iy*Iy + Iz*Iz);

                if(val > normfactor) normfactor = val;

                //
                p[idx] = val;

            }
        }
    }

    foreach(pagesz, i)
    {
        p[i] /= normfactor;
    }

    // de-alloc
    y_del<Tdata>(pData);

    //
    return;
}

template <class Tdata, class Tidx>
void extractsurfaces(Tdata *&p, Tidx *dims)
{
    //
    // extract surfaces from the binary image
    //

    // input: [0, 1] binary image
    REAL thresh = 0.5;

    //
    if(!p || !dims)
    {
        cout<<"Invalid inputs for extract surfaces function"<<endl;
        return;
    }

    //
    Tidx xdim, ydim, zdim;

    xdim=dims[0];
    ydim=dims[1];
    zdim=dims[2];

    Tidx zoffsets = xdim*ydim;
    Tidx yoffsets = xdim;

    Tidx pagesz = zdim*zoffsets;

    Tdata *pData = NULL;
    y_new<Tdata, Tidx>(pData, pagesz);

    foreach(pagesz, i)
    {
        pData[i] = p[i];
        p[i] = 0;
    }

    //
    Tidx neighbor3d[6] = {-1, 1, -yoffsets, yoffsets, -zoffsets, zoffsets};
    Tidx neighbor2d[4] = {-1, 1, -yoffsets, yoffsets};

    //
    for(Tidx z=0;z<zdim;z++)
    {
        Tidx ofz = z*zoffsets;
        for(Tidx y=0;y<ydim;y++)
        {
            Tidx ofy = ofz + y*xdim;
            for(Tidx x=0;x<xdim;x++)
            {
                Tidx idx = ofy + x;

                //
                if(zdim<2)
                {
                    if(x==0 || x==xdim-1 || y==0 || y==ydim-1)
                        continue;
                }
                else
                {
                    if(x==0 || x==xdim-1 || y==0 || y==ydim-1 || z==0 || z==zdim-1)
                        continue;
                }
                //

                if( (REAL)(pData[idx])>thresh )
                {
                    if(zdim<2)
                    {
                        foreach(4, ineighbor)
                        {
                            if( (REAL)(pData[ idx + neighbor2d[ineighbor] ]) < thresh)
                                p[idx] = 1;
                        }
                    }
                    else
                    {
                        foreach(6, ineighbor)
                        {
                            if( (REAL)(pData[ idx + neighbor3d[ineighbor] ]) < thresh)
                                p[idx] = 1;
                        }
                    }
                }

            }
        }
    }

    // de-alloc
    y_del<Tdata>(pData);

    //
    return;
}

template <class Tdata, class Tidx>
void flip(Tdata *&pOut, Tdata *pIn, Tidx *dims, AxisType atype)
{
    //
    // flip image along x, y, z, c, or t
    //

    //
    if(!pIn || !dims)
    {
        cout<<"Invalid inputs for flip function"<<endl;
        return;
    }

    //
    Tidx xdim, ydim, zdim;

    xdim=dims[0];
    ydim=dims[1];
    zdim=dims[2];

    Tidx zoffsets = xdim*ydim;
    Tidx yoffsets = xdim;

    Tidx pagesz = zdim*zoffsets;

    if(!pOut)   y_new<Tdata, Tidx>(pOut, pagesz);

    if(atype==AX)
    {
        // flip along x axis
        foreach(zdim, k)
        {
            Tidx offset_k = k*zoffsets;
            foreach(ydim, j)
            {
                Tidx offset_j = j*yoffsets + offset_k;
                foreach(xdim, i)
                {
                    pOut[offset_j + xdim - 1 - i] = pIn[offset_j + i];
                }
            }
        }
    }
    else if(atype==AY)
    {
        // flip along y axis
        foreach(zdim, k)
        {
            Tidx offset_k = k*zoffsets;
            foreach(ydim, j)
            {
                Tidx offset_j = j*yoffsets + offset_k;
                Tidx offset_j_o = (ydim-1-j)*yoffsets + offset_k;
                foreach(xdim, i)
                {
                    pOut[offset_j_o + i] = pIn[offset_j + i];
                }
            }
        }
    }
    else if(atype==AZ)
    {
        // flip along z axis
        foreach(zdim, k)
        {
            Tidx offset_k = k*zoffsets;
            Tidx offset_k_o = (zdim-1-k)*zoffsets;
            foreach(ydim, j)
            {
                Tidx offset_j = j*yoffsets + offset_k;
                Tidx offset_j_o = j*yoffsets + offset_k_o;
                foreach(xdim, i)
                {
                    pOut[offset_j_o + i] = pIn[offset_j + i];
                }
            }
        }
    }
    else
    {
        cout<<"unknown axis type to flip image"<<endl;
        return;
    }

    //
    return;
}

template <class Tdata, class Tidx>
void imgcopy(Tdata *&pOut, Tdata *pIn, Tidx totalplxs)
{
    //
    // image copy
    //

    //
    if(!pIn || !pOut)
    {
        cout<<"Invalid inputs for image copy function"<<endl;
        return;
    }

    //
    foreach(totalplxs, i)
        pOut[i] = pIn[i];

    //
    return;
}

// tps basic function
template <class Tdata>
Tdata tpsRBF(Tdata r2)
{
    if(r2 == 0.0)
        return 0.0;
    else
        return (0.5*r2*log(r2));
}

// image intersection
template <class Tdata, class Tidx>
void imgintersection(Tdata *&pOut, Tdata *p1, Tdata *p2, Tidx totalplxs, Tidx method, Tdata ceilingval)
{
    //
    // method 0: min; 1: geometric mean; 2: product
    // ceiling value: 255 for 8-bit data; 4095 for 12-bit data
    //

    if(!p1 || !p2)
    {
        cout<<"Invalide inputs for image intersection"<<endl;
        return;
    }

    if(!pOut) y_new<Tdata, Tidx>(pOut, totalplxs);

    if(method==0)
    {
        foreach(totalplxs, i)
        {
            pOut[i] = y_min<Tdata>(p1[i], p2[i]);
        }
    }
    else if(method==1)
    {
        foreach(totalplxs, i)
        {
            pOut[i] = (Tdata)sqrt(double(p1[i])*double(p2[i]));
        }
    }
    else if(method==2)
    {
        REAL minv = INF, maxv = -INF;
        foreach(totalplxs, i)
        {
            REAL val = REAL(p1[i])*REAL(p2[i]);

            if(val<minv) minv = val;
            if(val>maxv) maxv = val;
        }
        maxv -= minv;

        foreach(totalplxs, i)
        {
            pOut[i] = (Tdata)((REAL(p1[i])*REAL(p2[i]))/maxv)*ceilingval;
        }
    }
    else
    {
        cout<<"Invalide inputs for image intersection"<<endl;
        return;
    }

    return;
}

template <class Tdata, class Tidx>
void imgcutting(Tdata *&pOutput, Tidx* szOutput, Tdata *pInput, Tidx *szInput, REAL threshold, Tidx left_x, Tidx right_x, Tidx up_y, Tidx down_y, Tidx front_z, Tidx back_z)
{
    // cutting blank plane occuring in image boundaries
    Tidx pz=0, nz=0, py=0, ny=0, px=0, nx=0;
    Tidx start_z=0;
    Tidx end_z=szInput[2]-1;
    Tidx start_y=0;
    Tidx end_y=szInput[1]-1;
    Tidx start_x=0;
    Tidx end_x=szInput[0]-1;

    Tidx colordim = szInput[3];

    Tidx pagesz = szInput[0]*szInput[1]*szInput[2];

    bool b_blankplanefound;

    // find NULL channel first
    Tidx nullc = -1;
    for(Tidx c=0; c<szInput[3]; c++) // image 1
    {
        Tidx offset_c = c*pagesz;
        double sumint = 0;
        for (Tidx k=0; k<szInput[2]; k++)
        {
            Tidx offset_k = offset_c + k*szInput[0]*szInput[1];
            for(Tidx j=0; j<szInput[1]; j++)
            {
                Tidx offset_j = offset_k + j*szInput[0];
                for(Tidx i=0; i<szInput[0]; i++)
                {
                    sumint += pInput[offset_j + i];
                }
            }
        }

        if(sumint<EMPTY)
        {
            nullc = c;
        }
        else
        {
            if(threshold<=0) threshold = 0.75 * sumint / double(pagesz);
        }
    }

    // using threshold cutting blank planes
    bool b_thresholdcut = true;
    if(front_z>0 || back_z>0 || up_y>0 || down_y>0 || left_x>0 || right_x>0 )
    {
        b_thresholdcut = false;
    }


    //
    double slicesz;

    // cutting
    // z
    slicesz = szInput[0]*szInput[1];

    // along z +
    if(front_z<=0 && b_thresholdcut)
    {
        b_blankplanefound=true;
        while (b_blankplanefound)
        {
            double sum;

            for(Tidx k=0; k<szInput[2]; k++)
            {
                Tidx offset_k = k*szInput[1]*szInput[0];

                b_blankplanefound=false;
                for(Tidx c=0; c<colordim; c++)
                {
                    sum = 0;

                    if(c==nullc) continue;

                    Tidx offset_c = c*pagesz + offset_k;
                    for(Tidx j=0; j<szInput[1]; j++)
                    {
                        Tidx offset_j = offset_c + j*szInput[0];
                        for(Tidx i=0; i<szInput[0]; i++)
                        {
                            sum += pInput[offset_j + i];
                        }
                    }

                    if(  (sum/slicesz) < threshold  )
                    {
                        b_blankplanefound = true;
                    }
                }

                if(b_blankplanefound)
                {
                    pz++;
                }
                else
                {
                    break;
                }
            }
            b_blankplanefound=false;
        }
    }
    else
    {
        pz = front_z;
    }

    // along z -
    if(back_z<=0 && b_thresholdcut)
    {
        b_blankplanefound=true;
        while (b_blankplanefound)
        {
            double sum;

            for(Tidx k=szInput[2]-1; k>0; k--)
            {
                Tidx offset_k = k*szInput[1]*szInput[0];

                b_blankplanefound=false;
                for(Tidx c=0; c<colordim; c++)
                {
                    sum = 0;

                    if(c==nullc) continue;

                    Tidx offset_c = c*pagesz + offset_k;
                    for(Tidx j=0; j<szInput[1]; j++)
                    {
                        Tidx offset_j = offset_c + j*szInput[0];
                        for(Tidx i=0; i<szInput[0]; i++)
                        {
                            sum += pInput[offset_j + i];
                        }
                    }

                    if( (sum/slicesz) < threshold )
                    {
                        b_blankplanefound = true;
                    }
                }

                if(b_blankplanefound)
                {
                    nz++;
                }
                else
                {
                    break;
                }
            }
            b_blankplanefound=false;
        }
    }
    else
    {
        nz = back_z;
    }

    // y
    slicesz = szInput[0]*szInput[2];

    // along y +
    if(up_y<=0 && b_thresholdcut)
    {
        b_blankplanefound=true;
        while (b_blankplanefound)
        {
            double sum;

            for(Tidx j=0; j<szInput[1]; j++)
            {
                Tidx offset_j = j*szInput[0];

                b_blankplanefound=false;
                for(Tidx c=0; c<colordim; c++)
                {
                    sum = 0;

                    if(c==nullc) continue;

                    Tidx offset_c = c*pagesz + offset_j;
                    for(Tidx k=0; k<szInput[2]; k++)
                    {
                        Tidx offset_k = offset_c + k*szInput[1]*szInput[0];

                        for(Tidx i=0; i<szInput[0]; i++)
                        {
                            sum += pInput[offset_k + i];
                        }
                    }

                    if( (sum/slicesz) < threshold )
                    {
                        b_blankplanefound = true;
                    }
                }

                if(b_blankplanefound)
                {
                    py++;
                }
                else
                {
                    break;
                }
            }
            b_blankplanefound=false;
        }
    }
    else
    {
        py = up_y;
    }

    // along y -
    if(down_y<=0 && b_thresholdcut)
    {
        b_blankplanefound=true;
        while (b_blankplanefound)
        {
            double sum;

            for(Tidx j=szInput[1]-1; j>0; j--)
            {
                Tidx offset_j = j*szInput[0];

                b_blankplanefound=false;
                for(Tidx c=0; c<colordim; c++)
                {
                    sum = 0;

                    if(c==nullc) continue;

                    Tidx offset_c = c*pagesz + offset_j;
                    for(Tidx k=0; k<szInput[2]; k++)
                    {
                        Tidx offset_k = offset_c + k*szInput[1]*szInput[0];

                        for(Tidx i=0; i<szInput[0]; i++)
                        {
                            sum += pInput[offset_k + i];
                        }
                    }

                    if( (sum/slicesz) < threshold )
                    {
                        b_blankplanefound = true;
                    }
                }

                if(b_blankplanefound)
                {
                    ny++;
                }
                else
                {
                    break;
                }
            }
            b_blankplanefound=false;
        }
    }
    else
    {
        ny = down_y;
    }

    // x
    slicesz = szInput[1]*szInput[2];

    // along x +
    if(left_x<=0 && b_thresholdcut)
    {
        b_blankplanefound=true;
        while (b_blankplanefound)
        {
            double sum;

            for(Tidx i=0; i<szInput[0]; i++)
            {
                b_blankplanefound=false;
                for(Tidx c=0; c<colordim; c++)
                {
                    sum = 0;

                    if(c==nullc) continue;

                    Tidx offset_c = c*pagesz;
                    for(Tidx k=0; k<szInput[2]; k++)
                    {
                        Tidx offset_k = offset_c + k*szInput[1]*szInput[0];
                        for(Tidx j=0; j<szInput[1]; j++)
                        {
                            Tidx offset_j = offset_k + j*szInput[0];

                            sum += pInput[offset_j + i];
                        }
                    }

                    cout<<endl<<"(sum/slicesz) "<<(sum/slicesz)<<" "<<slicesz;

                    if( (sum/slicesz) < threshold )
                    {
                        b_blankplanefound = true; break;
                    }
                }

                if(b_blankplanefound)
                {
                    px++;
                }
                else
                {
                    break;
                }
            }
            b_blankplanefound=false;
        }
    }
    else
    {
        px = left_x;
    }

    // along x -
    if(right_x<=0 && b_thresholdcut)
    {
        b_blankplanefound=true;
        while (b_blankplanefound)
        {
            double sum;

            for(Tidx i=szInput[0]-1; i>0; i--)
            {
                b_blankplanefound=false;
                for(Tidx c=0; c<colordim; c++)
                {
                    sum = 0;

                    if(c==nullc) continue;

                    Tidx offset_c = c*pagesz;
                    for(Tidx k=0; k<szInput[2]; k++)
                    {
                        Tidx offset_k = offset_c + k*szInput[1]*szInput[0];
                        for(Tidx j=0; j<szInput[1]; j++)
                        {
                            Tidx offset_j = offset_k + j*szInput[0];

                            sum += pInput[offset_j + i];
                        }
                    }

                    if( (sum/slicesz) < threshold )
                    {
                        b_blankplanefound = true;
                    }
                }

                if(b_blankplanefound)
                {
                    nx++;
                }
                else
                {
                    break;
                }
            }
            b_blankplanefound=false;
        }
    }
    else
    {
        nx = right_x;
    }

    //
    start_x += px; start_y += py; start_z += pz;
    end_x -= nx; end_y -= ny; end_z -= nz;

    Tidx sx = end_x - start_x + 1;
    Tidx sy = end_y - start_y + 1;
    Tidx sz = end_z - start_z + 1;
    Tidx pagesz_new = sx*sy*sz;
    Tidx totalplxs = pagesz_new*colordim;

    // new image
    if(totalplxs>0)
    {
        try
        {
            pOutput = new Tdata [totalplxs];

            for(Tidx c=0; c<colordim; c++)
            {
                Tidx offset_c_old = c*pagesz;
                Tidx offset_c_new = c*pagesz_new;
                for(Tidx k=start_z; k<=end_z; k++)
                {
                    Tidx offset_k_old = offset_c_old + k*szInput[0]*szInput[1];
                    Tidx offset_k_new = offset_c_new + (k-start_z)*sx*sy;
                    for(Tidx j=start_y; j<=end_y; j++)
                    {
                        Tidx offset_j_old = offset_k_old + j*szInput[0];
                        Tidx offset_j_new = offset_k_new + (j-start_y)*sx;
                        for(Tidx i=start_x; i<=end_x; i++)
                        {
                            pOutput[offset_j_new + i - start_x] = pInput[offset_j_old + i];
                        }
                    }
                }
            }

            szOutput[0] = sx;
            szOutput[1] = sy;
            szOutput[2] = sz;
            szOutput[3] = colordim;
        }
        catch (...) {
            cout<<"Fail to allocate new image memory."<<endl;
            return;
        }
    }
    else
    {
        cout<<"Image is completely cut off"<<endl;
        return;
    }

    return;
}

// get maximum intensity
template<class Tdata, class Tidx>
void getMaximum(Tdata *p, Tidx sz, Tdata &maxval, Tidx &maxidx)
{
    if(p && sz>1)
    {
        maxval = -INF;
        maxidx = 0;

        //
        Tidx i;

        for(i=0; i<sz; i++)
        {
            if(p[i]>maxval)
            {
                maxval = p[i];
                maxidx = i;
            }
        }
    }
    else
    {
        cout<<"Null image"<<endl;
        return;
    }

    return;
}

// check boundary
template<class Tidx>
Tidx checkBoundary(Tidx dim, Tidx x)
{
    if(x<0) return 0;
    else if(x>=dim) return dim-1;
    else return x;
}

// check loop
template<class T>
bool checkLoop(T vec)
{
    if(vec.size()<=0)
    {
        return false;
    }

    for(int i=0; i<vec.size(); i++)
    {
        if(vec[i].visited==false)
            return true;
    }

    return false;
}

// check repeated
template<class T, class Tidx>
bool checkRepeated(T vec, Tidx x, Tidx y, Tidx z)
{
    if(vec.size()<=0)
    {
        return false;
    }

    for(int i=0; i<vec.size(); i++)
    {
        if(x==vec[i].x && y==vec[i].y && z==vec[i].z)
            return true;
    }

    return false;
}

// region growing
template<class Tdata1, class Tdata2, class Tidx>
void regiongrow(Tdata1* pIm, Tdata2* &pLB, Tidx dimx, Tidx dimy, Tidx dimz, Tidx seedx, Tidx seedy, Tidx seedz, Tidx label, double threshold, double variation)
{
    if(!pIm || !pLB || dimx<1 || dimy<1 || dimz<1 || seedx<1 || seedy<1 || seedz<1)
    {
        cout<< "Invalid Inputs"<<endl;
        return;
    }

    //
    Tidx offset_y, offset_z;

    offset_y=dimx;
    offset_z=dimx*dimy;

    //
    vector< Y_MARKER<Tidx> > segments;

    Y_MARKER<Tidx> seed;
    seed.x = seedx;
    seed.y = seedy;
    seed.z = seedz;
    seed.visited = false;

    segments.clear();
    segments.push_back(seed);

    Tidx idx,i;
    Tidx x,y,z, xp, yp, zp, xn, yn, zn;

    double val;

    //
    while( checkLoop< vector< Y_MARKER<Tidx> > > (segments) )
    {
        for(i=0; i<segments.size(); i++)
        {
            if(segments[i].visited==false)
            {
                seedx = segments[i].x;
                seedy = segments[i].y;
                seedz = segments[i].z;

                segments[i].visited = true; // process this position

                break;
            }
        }

        //
        idx = seedz*offset_z + seedy*offset_y + seedx;
        val = pIm[idx];

        //cout<<"test ... "<<segments.size()<<" "<<seedx<<" "<<seedy<<" "<<seedz<<endl;

        pLB[idx] = label; // assign

        //
        xn = seedx - 1;
        yn = seedy - 1;
        zn = seedz - 1;

        xn = checkBoundary<Tidx>(dimx, xn);
        yn = checkBoundary<Tidx>(dimy, yn);
        zn = checkBoundary<Tidx>(dimz, zn);

        xp = seedx + 1;
        yp = seedy + 1;
        zp = seedz + 1;

        xp = checkBoundary<Tidx>(dimx, xp);
        yp = checkBoundary<Tidx>(dimy, yp);
        zp = checkBoundary<Tidx>(dimz, zp);

        // 1
        x = xn; y = seedy; z = seedz; // x-
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);

            //cout<<"diff "<<diff<<" "<<pIm[idx]<<" "<<val<<endl;

            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 18
        x = xp; y = seedy; z = seedz; // x+
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 10
        x = seedx; y = yn; z = seedz; // y-
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 13
        x = seedx; y = yp; z = seedz; // y+
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 16
        x = seedx; y = seedy; z = zn; // z-
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 17
        x = seedx; y = seedy; z = zp; // z+
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 2
        x = xn; y = seedy; z = zn;
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 3
        x = xn; y = seedy; z = zp;
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 4
        x = xn; y = yn; z = seedz;
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 5
        x = xn; y = yn; z = zn;
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 6
        x = xn; y = yn; z = zp;
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 7
        x = xn; y = yp; z = seedz;
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 8
        x = xn; y = yp; z = zn;
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 9
        x = xn; y = yp; z = zp;
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 11
        x = xn; y = yn; z = zn;
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 12
        x = seedx; y = yn; z = zp;
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 14
        x = seedx; y = yp; z = zn;
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 15
        x = seedx; y = yp; z = zp;
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 19
        x = xp; y = seedy; z = zn;
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 20
        x = xp; y = seedy; z = zp;
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 21
        x = xp; y = yn; z = seedz;
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 22
        x = xp; y = yn; z = zn;
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 23
        x = xp; y = yn; z = zp;
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 24
        x = xp; y = yp; z = seedz;
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 25
        x = xp; y = yp; z = zn;
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }

        // 26
        x = xp; y = yp; z = zp;
        if(!checkRepeated<vector< Y_MARKER<Tidx> >, Tidx>(segments, x, y, z))
        {
            idx = z*offset_z+y*offset_y+x;

            double diff = fabs(double(pIm[idx]) - val);
            if(diff>threshold && diff<variation)
            {
                seed.x = x;
                seed.y = y;
                seed.z = z;
                seed.visited = false;

                segments.push_back(seed);
            }
        }


    } // while


    return;
}

//-------------------------------------------------------------------------------
/// Matrix class
//-------------------------------------------------------------------------------

template<class Tdata, class Tidx>
Y_MAT<Tdata, Tidx> :: Y_MAT()
{
    p=NULL; v=NULL; row=0; column=0;
}

template<class Tdata, class Tidx>
Y_MAT<Tdata, Tidx> :: ~Y_MAT()
{
}

template<class Tdata, class Tidx>
Y_MAT<Tdata, Tidx> :: Y_MAT(Tdata *pInput, Tidx dimx, Tidx dimy)
{
    p = pInput;
    row = dimx;
    column = dimy;

    // construct 2d pointer proxy
    try
    {
        v=new Tdata *[column];
        for(Tidx j=0; j<column; j++)
        {
            v[j] = p + j*row;
        }
    }
    catch(...)
    {
        cout<<"Fail to allocate memory for new Matrix!"<<endl;
        return;
    }
}

template<class Tdata, class Tidx>
void Y_MAT<Tdata, Tidx> :: setVal(Tidx r, Tidx c, Tdata val)
{
    if(r<0 || r>=row || c<0 || c>=column)
    {
        cout<<"Invalid index"<<endl;
        return;
    }

    v[c][r] = val; // show example here

    return;
}

template<class Tdata, class Tidx>
void Y_MAT<Tdata, Tidx> :: zeros()
{
    if(!p || !v)
    {
        cout<<"Matrix is NULL"<<endl;
        return;
    }

    for(Tidx j=0; j<column; j++)
        for(Tidx i=0; i<row; i++)
            v[j][i] = 0;

    return;
}

template<class Tdata, class Tidx>
void Y_MAT<Tdata, Tidx> :: unit()
{
    if(!p || !v)
    {
        cout<<"Matrix is NULL"<<endl;
        return;
    }

    if(column<row)
    {
        for(Tidx j=0; j<column; j++)
        {
            for(Tidx i=0; i<row; i++)
            {
                v[j][i] = 0;
            }

            v[j][j] = 1;
        }
    }
    else
    {
        for(Tidx i=0; i<row; i++)
        {
            for(Tidx j=0; j<column; j++)
            {
                v[j][i] = 0;
            }

            v[i][i] = 1;
        }
    }

    return;
}

template<class Tdata, class Tidx>
void Y_MAT<Tdata, Tidx> :: init(Tidx dimx, Tidx dimy, Tdata value)
{
    // value == 0 is a zero matrix
    // value == 1 is a unitary matrix
    // otherwise create a diagonal matrix

    row = dimx;
    column = dimy;

    Tidx N = row*column;
    y_new<Tdata, Tidx>(p, N);

    // construct 2d pointer proxy
    try
    {
        v=new Tdata *[column];
        for(Tidx j=0; j<column; j++)
        {
            v[j] = p + j*row;
        }
    }
    catch(...)
    {
        cout<<"Fail to allocate memory!"<<endl;
        return;
    }

    if(column<row)
    {
        for(Tidx j=0; j<column; j++)
        {
            for(Tidx i=0; i<row; i++)
            {
                v[j][i] = 0;
            }

            v[j][j] = (Tdata)value;
        }
    }
    else
    {
        for(Tidx i=0; i<row; i++)
        {
            for(Tidx j=0; j<column; j++)
            {
                v[j][i] = 0;
            }

            v[i][i] = (Tdata)value;
        }
    }
}

template<class Tdata, class Tidx>
void Y_MAT<Tdata, Tidx> :: assign(const Y_MAT M) const
{
    if(row!=M.row || column!=M.column)
    {
        cout<<"Fail to assign matrix"<<endl;
        return;
    }

    for(Tidx j=0; j<column; j++)
    {
        for(Tidx i=0; i<row; i++)
        {
            v[j][i] = M.v[j][i];
        }
    }

    return;
}

template<class Tdata, class Tidx>
void Y_MAT<Tdata, Tidx> :: clone(Y_MAT pIn, bool t)
{
    if(t) // transpose
    {
        row = pIn.column;
        column = pIn.row;
    }
    else
    {
        row = pIn.row;
        column = pIn.column;
    }

    Tidx N = row*column;

    try
    {
        p=new Tdata [N];
        v=new Tdata *[column];

        for(Tidx j=0; j<column; j++)
        {
            v[j] = p + j*row;
        }

        if(t)
        {
            for(Tidx j=0; j<column; j++)
            {
                for(Tidx i=0; i<row; i++)
                {
                    v[j][i] = pIn.v[i][j];
                }
            }
        }
        else
        {
            for(Tidx i=0; i<N; i++)
            {
                p[i] = pIn.p[i];
            }
        }

    }
    catch(...)
    {
        cout<<"Fail to allocate memory for new Matrix!"<<endl;
        return;
    }

    return;
}

template<class Tdata, class Tidx>
void Y_MAT<Tdata, Tidx> :: copy(Y_MAT M)
{
    clean();
    clone(M, false);
}

template<class Tdata, class Tidx>
void Y_MAT<Tdata, Tidx> :: clean()
{
    y_del<Tdata>(p);
    if(v){ delete [] v; v=NULL;} // because v is a 2D proxy pointer of p and p has been deleted
}

template<class Tdata, class Tidx>
void Y_MAT<Tdata, Tidx> :: add(Y_MAT M)
{
    // inputs: this, M
    // output: M+this

    //
    if(row!=M.row || column != M.column)
    {
        cout<<"Matrix cannot be add because dimensions are not match!"<<endl;
        return;
    }

    // add
    for(Tidx i=0; i<row; i++)
    {
        for(Tidx j=0; j<column; j++)
        {
            v[j][i] += M.v[j][i];
        }
    }

    //
    return;
}

template<class Tdata, class Tidx>
void Y_MAT<Tdata, Tidx> :: prod(Y_MAT M)
{
    // inputs: this, M
    // output: M*this

    //
    if(column!=M.row)
    {
        cout<<"Matrix cannot multiply because dimensions are not match!"<<endl;
        return;
    }

    Y_MAT T;
    T.init(row,M.column, 0);

    // multiply
    for(Tidx k=0; k<M.column; k++)
    {
        for(Tidx i=0; i<row; i++)
        {
            Tdata sum=0;
            for(Tidx j=0; j<column; j++)
            {
                sum += v[j][i]*M.v[k][j];
            }
            T.v[k][i] = sum;
        }
    }

    //
    copy(T);
    T.clean();

    //
    return;
}

template<class Tdata, class Tidx>
void Y_MAT<Tdata, Tidx> :: transpose()
{
    Y_MAT T; // tmp Matrix

    T.clone(*this, true);

    copy(T);

    T.clean();
}

template<class Tdata, class Tidx>
void Y_MAT<Tdata, Tidx> :: pseudoinverse()
{
    // A = U*S*V'
    // inv(A) = V*inv(S)*U'

    if(row!=column)
    {
        cout<<"Need a square matrix to decomposition!"<<endl;
        return;
    }

    //
    Tidx N = row;

    // init
    Y_MAT V;
    V.init(N, N, 0);
    Y_MAT S;
    S.init(N, N, 0);
    Y_MAT U;
    U.copy(*this);

    svd(U, S, V);

    // U'
    U.transpose();

    // inv(S)
    for(Tidx i=0; i<N; i++)
    {
        if(S.v[i][i]>EPS)
            S.v[i][i] = 1.0/S.v[i][i];
        else
            S.v[i][i] = 0.0;
    }

    // V*inv(S)*U'
    V.prod(S);
    V.prod(U);

    copy(V);

    //
    V.clean();
    S.clean();
    U.clean();
}

// computes (sqrt(a^2 + b^2)) without destructive underflow or overflow
// "Numerical Recipes" 1992
template<class Tdata, class Tidx>
Tdata Y_MAT<Tdata, Tidx> :: pythagorean(Tdata a, Tdata b)
{
    Tdata at = fabs(a), bt = fabs(b), ct, result;

    if (at > bt)       { ct = bt / at; result = at * sqrt(1.0 + ct * ct); }
    else if (bt > 0.0) { ct = at / bt; result = bt * sqrt(1.0 + ct * ct); }
    else result = 0.0;

    return result;
}

// if A=U*S*V'
//  INPUT: A (U=A, S=0, V=0)
// OUTPUT: U, S, V
// row=column case of the routine SVDCMP from "Numerical Recipes" 1992
template<class Tdata, class Tidx>
void Y_MAT<Tdata, Tidx> :: svd(Y_MAT U, Y_MAT S, Y_MAT V)
{
    if(row!=column)
    {
        cout<<"Need a square matrix to decomposition!"<<endl;
        return;
    }

    Tidx N = row;

    Tidx flag, i, its, j, jj, k, l, nm;
    Tdata anorm = 0.0, g = 0.0, scale = 0.0;

    Tdata c, f, h, s, x, y, z;
    Tdata *rv1=NULL;

    try
    {
        rv1 = new Tdata [N];
    }
    catch(...)
    {
        cout<<"Fail to allocate memory for temporary variable in svd function!"<<endl;
        y_del<Tdata>(rv1);
        return;
    }

    // Householder reduction to bidiagonal form
    for (i = 0; i < N; i++)
    {
        l = i + 1;
        rv1[i] = scale * g;

        // left-hand reduction
        g = s = scale = 0.0;
        if (i < N)
        {
            for (k = i; k < N; k++)
                scale += fabs(U.v[i][k]);
            if (scale)
            {
                for (k = i; k < N; k++)
                {
                    U.v[i][k] /= scale;
                    s += (U.v[i][k] * U.v[i][k]);
                }
                f = U.v[i][i];
                g = -y_sign2v<Tdata>(sqrt(s), f);
                h = f * g - s;
                U.v[i][i] = f - g;
                if (i != N - 1)
                {
                    for (j = l; j < N; j++)
                    {
                        for (s = 0.0, k = i; k < N; k++)
                            s += (U.v[i][k] * U.v[j][k]);
                        f = s / h;
                        for (k = i; k < N; k++)
                            U.v[j][k] += f * U.v[i][k];
                    }
                }
                for (k = i; k < N; k++)
                    U.v[i][k] *= scale;
            }
        }
        S.v[i][i] = scale * g;

        // right-hand reduction
        g = s = scale = 0.0;
        if (i < N && i != N - 1)
        {
            for (k = l; k < N; k++)
                scale += fabs(U.v[k][i]);
            if (scale)
            {
                for (k = l; k < N; k++)
                {
                    U.v[k][i] /= scale;
                    s += U.v[k][i] * U.v[k][i];
                }
                f = U.v[l][i];
                g = -y_sign2v<Tdata>(sqrt(s), f);
                h = f * g - s;
                U.v[l][i] = f - g;
                for (k = l; k < N; k++)
                    rv1[k] = U.v[k][i] / h;
                if (i != N - 1)
                {
                    for (j = l; j < N; j++)
                    {
                        for (s = 0.0, k = l; k < N; k++)
                            s += U.v[k][j] * U.v[k][i];
                        for (k = l; k < N; k++)
                            U.v[k][j] += s * rv1[k];
                    }
                }
                for (k = l; k < N; k++)
                    U.v[k][i] *= scale;
            }
        }
        anorm = y_max(anorm, (fabs(S.v[i][i]) + fabs(rv1[i])));
    }

    // accumulate the right-hand transformation
    for (i = N - 1; i >= 0; i--)
    {
        if (i < N - 1)
        {
            if (g)
            {
                for (j = l; j < N; j++)
                    V.v[i][j] = (U.v[j][i] / U.v[l][i]) / g;
                // double division to avoid underflow
                for (j = l; j < N; j++)
                {
                    for (s = 0.0, k = l; k < N; k++)
                        s += (U.v[k][i] * V.v[j][k]);
                    for (k = l; k < N; k++)
                        V.v[j][k] += (s * V.v[i][k]);
                }
            }
            for (j = l; j < N; j++)
                V.v[i][j] = V.v[j][i] = 0.0;
        }
        V.v[i][i] = 1.0;
        g = rv1[i];
        l = i;
    }

    // accumulate the left-hand transformation
    for (i = N - 1; i >= 0; i--)
    {
        l = i + 1;
        g = S.v[i][i];
        if (i < N - 1)
            for (j = l; j < N; j++)
                U.v[j][i] = 0.0;
        if (g)
        {
            g = 1.0 / g;
            if (i != N - 1)
            {
                for (j = l; j < N; j++)
                {
                    for (s = 0.0, k = l; k < N; k++)
                        s += (U.v[i][k] * U.v[j][k]);
                    f = (s / U.v[i][i]) * g;
                    for (k = i; k < N; k++)
                        U.v[j][k] += f * U.v[i][k];
                }
            }
            for (j = i; j < N; j++)
                U.v[i][j] *= g;
        }
        else
        {
            for (j = i; j < N; j++)
                U.v[i][j] = 0.0;
        }
        ++U.v[i][i];
    }

    // diagonalize the bidiagonal form
    for (k = N - 1; k >= 0; k--)
    {   // loop over singular values
        for (its = 0; its < 30; its++)
        {   // loop over allowed iterations
            flag = 1;
            for (l = k; l >= 0; l--)
            {   // Test for splitting
                // Note that rv1[1] is always zero
                nm = l - 1;
                if (fabs(rv1[l]) + anorm == anorm)
                {
                    flag = 0;
                    break;
                }
                if (fabs(S.v[nm][nm]) + anorm == anorm)
                    break;
            }
            if (flag)
            {
                c = 0.0;
                s = 1.0;
                for (i = l; i <= k; i++)
                {
                    f = s * rv1[i];
                    if (fabs(f) + anorm != anorm)
                    {
                        g = S.v[i][i];
                        h = pythagorean(f, g);
                        S.v[i][i] = h;
                        h = 1.0 / h;
                        c = g * h;
                        s = (- f * h);
                        for (j = 0; j < N; j++)
                        {
                            y = U.v[nm][j];
                            z = U.v[i][j];
                            U.v[nm][j] = y*c + z*s;
                            U.v[i][j] = z*c - y*s;
                        }
                    }
                }
            }
            z = S.v[k][k];
            if (l == k)
            {   //convergence
                if (z < 0.0)
                {   // singular value is made nonnegative
                    S.v[k][k] = -z;
                    for (j = 0; j < N; j++)
                        V.v[k][j] = -V.v[k][j];
                }
                break;
            }
            if (its >= 30) {
                y_del<Tdata>(rv1);
                cout<<"No convergence in 30 svdcmp iterations!"<<endl;
                return;
            }

            // shift from bottom 2-by-2 minor
            x = S.v[l][l];
            nm = k - 1;
            y = S.v[nm][nm];
            g = rv1[nm];
            h = rv1[k];
            f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
            g = pythagorean(f, 1.0);
            f = ((x - z) * (x + z) + h * ((y / (f + y_sign2v<Tdata>(g, f))) - h)) / x;

            // next QR transformation
            c = s = 1.0;
            for (j = l; j <= nm; j++)
            {
                i = j + 1;
                g = rv1[i];
                y = S.v[i][i];
                h = s * g;
                g = c * g;
                z = pythagorean(f, h);
                rv1[j] = z;
                c = f / z;
                s = h / z;
                f = x * c + g * s;
                g = g * c - x * s;
                h = y * s;
                y = y * c;
                for (jj = 0; jj < N; jj++)
                {
                    x = V.v[j][jj];
                    z = V.v[i][jj];
                    V.v[j][jj] = x*c + z*s;
                    V.v[i][jj] = z*c - x*s;
                }
                z = pythagorean(f, h);
                S.v[j][j] = z;
                if (z)
                {
                    z = 1.0 / z;
                    c = f * z;
                    s = h * z;
                }
                f = (c * g) + (s * y);
                x = (c * y) - (s * g);
                for (jj = 0; jj < N; jj++)
                {
                    y = U.v[j][jj];
                    z = U.v[i][jj];
                    U.v[j][jj] = y*c + z*s;
                    U.v[i][jj] = z*c - y*s;
                }
            }
            rv1[l] = 0.0;
            rv1[k] = f;
            S.v[k][k] = x;
        }
    }
    y_del<Tdata>(rv1);

    return;
}

// if a*x=b
//  INPUT: a, b
// OUTPUT: x
// Solve a linear equation system a*x=b using LU decomposition.
// Crout's algorithm from "Numerical Recipes" 1992
template<class Tdata, class Tidx>
void Y_MAT<Tdata, Tidx> :: lu(Y_MAT a, Y_MAT b)
{
    //
    /// Solve linear system: Anxn * Xdxn = Bdxn
    //

    if(a.row!=a.column)
    {
        cout<<"Invalid matrix dimensions!"<<endl;
        return;
    }

    Tidx n = a.row, d = b.row;
    Tdata pivsign = 1; // no row interchanges

    Tidx * piv = NULL;
    y_new<Tidx, Tidx>(piv, n);

    REAL *scalefactor = NULL;
    y_new<REAL, Tidx>(scalefactor, n);

    //
    /// LU DECOMPOSITION
    //

    // init ordering
    for (Tidx i = 0; i < n; ++i) piv[i] = i;

    // scaling factor
    for (Tidx i=0;i<n;i++)
    {
        REAL big=0.0;
        for (Tidx j=0;j<=n;j++)
        {
            Tdata val = a.v[i][j];

            if(y_abs<Tdata>(val)>big) big = val;
        }
        if (big == 0.0) throw("Singular matrix in LU decomposition");
        scalefactor[i]=1.0/big;
    }

    // decomposition
    for (Tidx k=0;k<n;k++)
    {
        Tidx imax=k;
        REAL big=0.0;

        for (Tidx i=k;i<n;i++) //search for largest pivot element
        {
            REAL temp=scalefactor[i]*y_abs<Tdata>(a.v[k][i]);
            if (temp > big)
            {
                big=temp;
                imax=i;
            }
        }
        if (k != imax) // interchange rows?
        {
            for (Tidx j=0;j<n;j++)
            {
                Tdata temp=a.v[j][imax];
                a.v[j][imax]=a.v[j][k];
                a.v[j][k]=temp;
            }
            pivsign = -(pivsign);
            scalefactor[imax]=scalefactor[k];
        }
        piv[k]=imax;
        if (a.v[k][k] == 0.0) a.v[k][k]=EPS;
        for (Tidx i=k+1;i<n;i++)
        {
            REAL temp = a.v[k][i] /= a.v[k][k];
            for (Tidx j=k+1;j<n;j++)
                a.v[j][i] -= temp*a.v[j][k];
        }
    }

    //
    /// SOLVE
    //

    Tidx ii = 0; // When ii is set to a positive value, it will become the index of thefirst nonvanishing element of b.

    //
    for(Tidx k = 0; k<d; k++)
    {
        for(Tidx i=0;i<n;i++)
        {
            Tidx ip=piv[i];
            Tdata sum=b.v[ip][k];
            b.v[ip][k]=b.v[i][k];
            if (ii!=0)
                for (Tidx j=ii-1;j<i;j++) sum -= a.v[j][i]*b.v[j][k];
            else if (sum!=0)
                ii=i+1;
            b.v[i][k]=sum;
        }
        for (Tidx i=n-1;i>=0;i--)
        {
            Tdata sum=b.v[i][k];
            for (Tidx j=i+1;j<n;j++) sum -= a.v[j][i]*b.v[j][k];
            b.v[i][k]=sum/a.v[i][i];
        }
    }

    // de-alloc
    y_del<Tidx>(piv);
    y_del<REAL>(scalefactor);

    //
    return;
}

// vector product
template<class Tdata, class Tidx>
void Y_MAT<Tdata, Tidx> :: vectorMul(REAL *in, REAL *&out)
{
    // input:  1d vector "in"
    // output: 1d vector "out"

    if(row!=column)
    {
        cout<<"Error: fail to run function vectorMul"<<endl;
        return;
    }
    Tidx sz = row;

    for(Tidx j=0; j<sz; j++)
    {
        out[j] = 0;
        for(Tidx i=0; i<sz; i++)
        {
            out[j] += v[j][i]*in[i];
        }
    }

    return;
}

//
template<class Tdata, class Tidx>
void Y_MAT<Tdata, Tidx> :: normalize3DAffine(Y_MAT M)
{
    // input:  M 3xn matrix
    // output: 4x4 affine matrix and normalized M

    //
    Tidx n = M.column;
    Tdata cx=0, cy=0, cz=0; // centroid
    for(Tidx i=0; i<n; i++)
    {
        cx+=M.v[i][0];
        cy+=M.v[i][1];
        cz+=M.v[i][2];
    }
    cx/=n;
    cy/=n;
    cz/=n;
    for(Tidx i=0; i<n; i++)
    {
        M.v[i][0]-=cx;
        M.v[i][1]-=cy;
        M.v[i][2]-=cz;
    }

    Tdata scaleGrid= 0;
    for(Tidx i=0; i<n; i++)
    {
        scaleGrid += sqrt(M.v[i][0]*M.v[i][0] + M.v[i][1]*M.v[i][1] + M.v[i][2]*M.v[i][2]);
    }
    scaleGrid = n/scaleGrid;
    for(Tidx i=0; i<n; i++)
    {
        M.v[i][0] *= scaleGrid;
        M.v[i][1] *= scaleGrid;
        M.v[i][2] *= scaleGrid;
    }

    //
    v[0][0] = scaleGrid;
    v[0][1] = 0;
    v[0][2] = 0;
    v[0][3] = -scaleGrid*cx;
    v[1][0] = 0;
    v[1][1] = scaleGrid;
    v[1][2] = 0;
    v[1][3] = -scaleGrid*cy;
    v[2][0] = 0;
    v[2][1] = 0;
    v[2][2] = scaleGrid;
    v[2][3] = -scaleGrid*cz;
    v[3][0] = 0;
    v[3][1] = 0;
    v[3][2] = 0;
    v[3][3] = 1;

    return;
}

template<class Tdata, class Tidx>
void Y_MAT<Tdata, Tidx> :: getRigid(Y_MAT M1, Y_MAT M2)
{
    // inputs: fields M1 & M2
    // output: rigid matrix

    //
    unit();

    //
    Y_MAT aff1, aff2;
    aff1.init(4, 4, 1); // 3D
    aff2.init(4, 4, 1);

    //
    Y_MAT m1, m2;
    m1.copy(M1);
    m2.copy(M2);

    aff1.normalize3DAffine(m1);
    aff2.normalize3DAffine(m2);

    //
    m2.transpose();
    m1.prod(m2); // 3x3 matrix

    //
    Y_MAT<REAL, Tidx> V;
    V.init(3, 3, 0);
    Y_MAT<REAL, Tidx> S;
    S.init(3, 3, 0);

    //
    m1.svd(m1, S, V);
    m1.transpose();
    V.prod(m1); // R

    //
    for(Tidx i=0; i<3; i++)
        for(Tidx j=0; j<3; j++)
            v[i][j] = V.v[i][j];

    // aff2-1*v*aff1
    aff2.pseudoinverse();
    aff2.prod(*this);
    aff2.prod(aff1);

    //
    assign(aff2);

    // de-alloc
    aff1.clean();
    aff2.clean();
    m1.clean();
    m2.clean();
    V.clean();
    S.clean();

    //
    return;
}

template<class Tdata, class Tidx>
void Y_MAT<Tdata, Tidx> :: sortEigenVectors(Y_MAT E)
{
    // sort by descend order
    //
    //EXAMPLE:
    // E =
    //    100     0     0
    //      0    25     0
    //      0     0    50
    // this =
    //      1     2     3
    //      1     2     3
    //      1     2     3
    //
    //sortEigenVectors(E)
    // E =
    //    100     0     0
    //      0    50     0
    //      0     0    25
    // this =
    //      1     3     2
    //      1     3     2
    //      1     3     2

    //
    if(E.row!=row || E.column!=column)
    {
        cout<<"Invalid inputs"<<endl;
        return;
    }

    //
    Y_MAT Ssorted;
    Ssorted.init(row, column, 1);

    Tidx N = row;

    Tdata *data=NULL;
    y_new<Tdata, Tidx>(data, N);

    foreach(N, i) // row < column
    {
        data[i] = E.v[i][i];
    }

    //
    quickSort<Tdata, Tidx>(data, 0, N-1, N);

    foreach(N, k)
    {
        Tdata val = data[k];

        Tidx idx = 0;
        foreach(N, n)
        {
            if(val == E.v[n][n])
            {
                idx = n; break;
            }
        }

        foreach(N, i)
        {
            Ssorted.v[i][k] = v[i][idx];
        }
    }

    assign(Ssorted);

    //
    Ssorted.clean();
    y_del<Tdata>(data);

    //
    return;
}

template<class Tdata, class Tidx>
void Y_MAT<Tdata, Tidx> :: sortEigenValues()
{
    //
    /// sort by descend order
    //
    //EXAMPLE:
    //
    //    100     0     0
    //      0    25     0
    //      0     0    50
    //
    //sortEigenValues():
    //
    //    100     0     0
    //      0    50     0
    //      0     0    25
    //

    Tidx N = row; // assume row <= column

    Tdata *data=NULL;
    y_new<Tdata, Tidx>(data, N);

    foreach(N, i)
    {
        data[i] = v[i][i];
    }

    //
    quickSort<Tdata, Tidx>(data, 0, N-1, N);

    foreach(N, i)
    {
        v[i][i] = data[i];
    }

    //
    y_del<Tdata>(data);

    //
    return;
}

//-------------------------------------------------------------------------------
/// image (XYZCT) data structure
//-------------------------------------------------------------------------------

template<class Tdata, class Tidx>
Y_IMAGE<Tdata, Tidx> :: Y_IMAGE()
{
    dk=IMAGE;
    dt=(DataType)sizeof(Tdata);
    dimt=UNKNOWNDIMTYPE;
    pImg=NULL;
    dims=NULL;
    centroid=NULL;
    means=NULL;
    totalplxs=0;
    bbx=0; bex=1;
    bby=0; bey=1;
    bbz=0; bez=1;
}

template<class Tdata, class Tidx>
Y_IMAGE<Tdata, Tidx> :: ~Y_IMAGE()
{
    // clean();
}

// clean
template<class Tdata, class Tidx>
void Y_IMAGE<Tdata, Tidx> :: clean()
{
    if(pImg){
        delete [] pImg;
    }

    if(dims){
        delete [] dims;
    }

    if(centroid){
        delete [] centroid;
    }

    if(means){
        delete [] means;
    }

    //
    return;
}

// set an image
template<class Tdata, class Tidx>
void Y_IMAGE<Tdata, Tidx> :: setImage(Tdata *p, Tidx *sz, Tidx imgdim) //
{
    if(!p || !sz || imgdim<=0)
    {
        cout<<"Invalid inputs"<<endl;
        return;
    }

    pImg=p; dims=sz; dimt = (DimType)imgdim;

    totalplxs=1;
    for(Tidx i=0; i<dimt; i++) totalplxs*=dims[i];

    if(dimt==4) set4doffsets(); // 4D image

    refc = 0;

    return;
}

// init an image
template<class Tdata, class Tidx>
void Y_IMAGE<Tdata, Tidx> :: initImage(Tidx *sz, Tidx imgdim) // init a zero image
{
    if(!sz || imgdim<=0)
    {
        cout<<"Invalid inputs"<<endl;
        return;
    }
    dimt = (DimType)imgdim;
    y_new<Tidx, Tidx>(dims, imgdim);

    totalplxs=1;
    for(Tidx i=0; i<imgdim; i++)
    {
        dims[i] = sz[i];
        totalplxs *= dims[i];
    }

    try
    {
        pImg=new Tdata [totalplxs];
        memset(pImg, 0, sizeof(Tdata)*totalplxs);
    }
    catch(...)
    {
        printf("Fail to allocate memory.\n");
        y_del<Tdata>(pImg);
        return;
    }

    if(dimt==4) set4doffsets(); // 4D image

    refc = 0;

    return;
}

// init a field
template<class Tdata, class Tidx>
void Y_IMAGE<Tdata, Tidx> :: initField(Tidx *sz, Tidx imgdim) // init a zero field
{
    dk = FIELD;
    initImage(sz, imgdim);
    return;
}

// set 4d image offset
template<class Tdata, class Tidx>
void Y_IMAGE<Tdata, Tidx> :: set4doffsets()
{
    if(dims)
    {
        of3 = dims[2]*dims[1]*dims[0];
        of2 = dims[1]*dims[0];
        of1 = dims[0];
    }
    else
    {
        cout<<"Fail to set offsets"<<endl;
    }

    return;
}

// get mass center
template<class Tdata, class Tidx>
void Y_IMAGE<Tdata, Tidx> :: getMassCenter()
{
    if(pImg && dims)
    {
        y_new<REAL, Tidx>(centroid, (Tidx)dimt);

        REAL sum = 0.0;
        switch(dimt)
        {
        case D1D:

            // X

            break;
        case D2D:

            // XY

            break;
        case D3D:

            // XYZ

        case D4D:

            // XYZC C=1

            centroid[0] = 0.0;
            centroid[1] = 0.0;
            centroid[2] = 0.0;

            for(Tidx z=0; z<dims[2]; z++)
            {
                for(Tidx y=0; y<dims[1]; y++)
                {
                    for(Tidx x=0; x<dims[0]; x++)
                    {
                        REAL val = (REAL)(val4d(refc, z, y, x));

                        centroid[0] += val*(REAL)x;
                        centroid[1] += val*(REAL)y;
                        centroid[2] += val*(REAL)z;

                        sum += val;
                    }
                }
            }

            centroid[0] /= sum;
            centroid[1] /= sum;
            centroid[2] /= sum;

            break;
        case D5D:

            break;
        default:
            printf("Dimensions too complicated.\n");
        }
    }
}

// get mean intensity values
template<class Tdata, class Tidx>
void Y_IMAGE<Tdata, Tidx> :: getMeanIntensityValue()
{
    if(pImg && dims)
    {
        Tidx szVolume = dims[0]*dims[1]*dims[2];
        Tidx nonZeros = 0;

        switch(dimt)
        {
        case D1D:

            // X

            break;
        case D2D:

            // XY

            break;
        case D3D:

            // XYZ

        case D4D:

            // XYZC

            y_new<REAL, Tidx>(means, dims[3]);

            //
            for(Tidx c=0; c<dims[3]; c++)
            {
                means[c] = 0;

                for(Tidx z=0; z<dims[2]; z++)
                {
                    for(Tidx y=0; y<dims[1]; y++)
                    {
                        for(Tidx x=0; x<dims[0]; x++)
                        {
                            REAL val = (REAL)(val4d(c, z, y, x));

                            if(val>0)
                            {
                                means[c] += val;
                                nonZeros++;
                            }
                        }
                    }
                }
                means[c] /= nonZeros;
            }

            break;
        case D5D:

            break;
        default:
            printf("Dimensions too complicated.\n");
        }
    }
}

// get mean intensity values
template<class Tdata, class Tidx>
void Y_IMAGE<Tdata, Tidx> :: getBoundingBox()
{
    if(pImg && dims)
    {
        Tidx bx, ex, by, ey, bz, ez, sum;
        Tidx x, y, z, c;

        switch(dimt)
        {
        case D1D:

            // X

            break;
        case D2D:

            // XY

            break;
        case D3D:

            // XYZ

        case D4D:

            // XYZC

            //
            for(c=0; c<dims[3]; c++)
            {
                // yz plane
                bx = 0;
                ex = dims[0];
                bool found = false;

                for(x=0; x<dims[0]; x++)
                {
                    sum = 0;
                    for(y=0; y<dims[1]; y++)
                    {
                        for(z=0; z<dims[2]; z++)
                        {
                            if(val4d(c, z, y, x))
                            {
                                sum++;
                            }
                        }
                    }

                    if(sum<1)
                    {
                        if(!found)
                        {
                            bx = x;
                        }

                        if(x<ex && x>bx)
                        {
                            ex = x;
                            break;
                        }
                    }
                    else
                    {
                        found = true;
                    }
                }

                if(c==0)
                {
                    bbx = bx;
                    bex = ex;
                }
                else
                {
                    if(ex-bx > bex - bbx)
                    {
                        bbx = bx;
                        bex = ex;
                    }
                }


                // xz plane
                by = 0;
                ey = dims[1];
                found = false;

                for(y=0; y<dims[1]; y++)
                {
                    sum = 0;
                    for(x=0; x<dims[0]; x++)
                    {
                        for(z=0; z<dims[2]; z++)
                        {
                            if(val4d(c, z, y, x))
                            {
                                sum++;
                            }
                        }
                    }

                    if(sum<1)
                    {
                        if(!found)
                        {
                            by = y;
                        }

                        if(y<ey && y>by)
                        {
                            ey = y;
                            break;
                        }
                    }
                    else
                    {
                        found = true;
                    }
                }

                if(c==0)
                {
                    bby = by;
                    bey = ey;
                }
                else
                {
                    if(ey-by > bey - bby)
                    {
                        bby = by;
                        bey = ey;
                    }
                }

                // xy plane
                bz = 0;
                ez = dims[2];
                found = false;

                for(z=0; z<dims[2]; z++)
                {
                    sum = 0;
                    for(y=0; y<dims[1]; y++)
                    {
                        for(x=0; x<dims[0]; x++)
                        {
                            if(val4d(c, z, y, x))
                            {
                                sum++;
                            }
                        }
                    }

                    if(sum<1)
                    {
                        if(!found)
                        {
                            bz = z;
                        }

                        if(z<ez && z>bz)
                        {
                            ez = z;
                            break;
                        }
                    }
                    else
                    {
                        found = true;
                    }
                }

                if(c==0)
                {
                    bbz = bz;
                    bez = ez;
                }
                else
                {
                    if(ez-bz > bez - bbz)
                    {
                        bbz = bz;
                        bez = ez;
                    }
                }
            }

            break;
        case D5D:

            break;
        default:
            printf("Dimensions too complicated.\n");
        }
    }
}

// set reference channel
template<class Tdata, class Tidx>
void Y_IMAGE<Tdata, Tidx> :: setRefChn(Tidx refcinput)
{
    refc = refcinput;
}

// zeros
template<class Tdata, class Tidx>
void Y_IMAGE<Tdata, Tidx> :: zeros()
{
    for(Tidx i=0; i<totalplxs; i++) pImg[i] = 0;
}

// voxel value of 4d image
template<class Tdata, class Tidx>
Tdata Y_IMAGE<Tdata, Tidx> :: val4d(Tidx z, Tidx y, Tidx x, Tidx v)
{
    return (pImg[ z*of3 + y*of2 + x*of1 + v ]);
}

// 1d index of 4d image
template<class Tdata, class Tidx>
Tidx Y_IMAGE<Tdata, Tidx> :: idx4d(Tidx z, Tidx y, Tidx x, Tidx v)
{
    return (z*of3 + y*of2 + x*of1 + v);
}


//-------------------------------------------------------------------------------
/// 3d landmark structure
//-------------------------------------------------------------------------------

template <class Tidx>
Y_MARKER<Tidx>  :: Y_MARKER()
{
}

template <class Tidx>
Y_MARKER<Tidx> :: ~Y_MARKER()
{
}

template <class Tidx>
REAL Y_MARKER<Tidx> :: len()
{
    return sqrt(x*x + y*y + z*z);
}

#endif //__Y_IMG_H__
