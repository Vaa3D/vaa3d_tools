// y_NiftiImageIO.h
// created by Yang Yu, Dec 20, 2011
//

#ifndef __Y_NIFTIIMAGEIO_H__
#define __Y_NIFTIIMAGEIO_H__

#include "niftilib/nifti1_io.h"

#include <string>
#include <ctime>
#include <iostream>
#include <cmath>
#include <limits>

using namespace std;

#define EPS 1e-10

// rewrite itk itkNiftiImageIO class
// http://analyzedirect.com/support/10.0Documents/Analyze_Resource_01.pdf

// Analyze image file sets consist of at least 2 files:
// REQUIRED:
//    - an image file  ([basename].img or [basename].img.gz or [basename].img.Z)
//          This contains the binary represenation of the raw voxel values.
//          If the file is uncompressed, it should be of of size (sizeof(storagetype)//NX//NY//NZ(//NT).
//          The format of the image file is very simple; containing usually
//          uncompressed voxel data for the images in one of the several
//          possible voxel formats:
//             - 1 bit  packed binary (slices begin on byte boundaries)
//             - 8 bit  (unsigned char) gray scale unless .lkup file present
//             - 16 bit signed short
//             - 32 bit signed integers or float
//             - 24 bit RGB, 8 bits per channel
//    - a header file  ([basename].hdr)
//          This a 348 byte file 99.99% of all images that contains a binary represenation of the C-struct
//          defined in this file.  The analyze 7.5 header structure may, however, be extended beyond this minimal defintion
//          to encompase site specific information, and would have more than 348 bytes.  Given that the
//          ability to extend the header has rarely been used, this implementation of the Analyze 7.5
//          file will only read the first 348 bytes into the structure defined in this file, and all informaiton beyond the
//          348 bytes will be ignored.
// OPTIONAL:
//    - a color lookup file ([basename].lkup)
//      The .lkup file is a plain ASCII text file that contains 3 integer values between 0 and 255
//      on each line.  Each line of the lkup file represents one color table entry for the Red,
//      Green and Blue color components, respectively.  The total dynamic range of the image
//      is divided by the number of colors in color table to determine mapping of the image through
//      these colors.
//       For example, an 8-color 'rainbow colors' lookup table is represented as:
//       ===========================
//       255 0 0
//       255 128 0
//       255 255 0
//       128 255 0
//       0 255 0
//       0 0 255
//       128 0 255
//       255 0 255
//       ===========================
//    - an object file ([basename].obj)
//      A specially formated file with a mapping between object name and image code used to associate
//      image voxel locations with a label.  This file is run length encoded to save disk storage.

typedef  enum { UNKNOWNCOMPONENTTYPE, UCHAR, CHAR, USHORT, SHORT, UINT, INT, ULONG, LONG, FLOAT, DOUBLE } IOComponentType;
typedef  enum { UNKNOWNPIXELTYPE, SCALAR, RGB, RGBA, OFFSET, VECTOR,
                POINT, COVARIANTVECTOR, SYMMETRICSECONDRANKTENSOR,
                DIFFUSIONTENSOR3D, COMPLEX, FIXEDARRAY, MATRIX }  IOPixelType;
//
#define MIN_HEADER_SIZE 348
#define NII_HEADER_SIZE 352

//
class NiftiImageIO
{

public:
    NiftiImageIO();
    ~NiftiImageIO();

public:

    // reading
    /** Determine if the file can be read with this ImageIO implementation. */
    bool canReadFile(char *FileNameToRead);

    /** loading image to memory */
    bool load(void *&buffer);

    // writing
    /** Determine if the file can be written with this ImageIO implementation. */
    bool canWriteFile(char *FileNameToWrite);

    /** Writes the data to disk from the memory buffer provided. */
    void write(const void *buffer, long sx, long sy, long sz, long sc, long st, int datatype);

    //
    void writeSampled(const void *buffer, long sx, long sy, long sz, long sc, long st, int datatype, float srx, float sry, float srz);

    //
    char* getFileName();
    void setFileName(char* FileName);

    // compressed ?
    int isCompressed(const char * filename);
    
    //
    long getDimx();
    long getDimy();
    long getDimz();
    long getDimc();
    long getDimt();

    void setDimx(long x);
    void setDimy(long y);
    void setDimz(long z);
    void setDimc(long c);
    void setDimt(long t);

    int getDataType();
    void setDataType(int datatype);

    double getDx();
    double getDy();
    double getDz();

private:
    bool mustRescale();
    void setPixelType(IOPixelType pt);

private:
    nifti_image *m_NiftiImage;

    double m_RescaleSlope;
    double m_RescaleIntercept;

    bool m_LegacyAnalyze75Mode;

    char* m_FileName;

    IOPixelType m_PixelType;
    IOComponentType m_ComponentType;

    long dimx, dimy, dimz, dimc, dimt; // dimension
    double dx, dy, dz, dc, dt; // spacing
};

#endif // __Y_NIFTIIMAGEIO_H__
