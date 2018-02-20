//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).  
// All rights reserved.
//------------------------------------------------------------------------------------------------

/*******************************************************************************************************************************************************************************************
*    LICENSE NOTICE
********************************************************************************************************************************************************************************************
*    By downloading/using/running/editing/changing any portion of codes in this package you agree to this license. If you do not agree to this license, do not download/use/run/edit/change
*    this code.
********************************************************************************************************************************************************************************************
*    1. This material is free for non-profit research, but needs a special license for any commercial purpose. Please contact Alessandro Bria at a.bria@unicas.it or Giulio Iannello at 
*       g.iannello@unicampus.it for further details.
*    2. You agree to appropriately cite this work in your related studies and publications.
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not 
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

/******************
*    CHANGELOG    *
*******************
*******************
* 2017-10-05. Giulio.     @FIXED a problem with slices having very large width in 'initTiff3DFile' and 'appendSlice2Tiff3DFile'
* 2017-05-03. Giulio.     @ADDED ruotine resetLibTIFFcfg to reconfigure an already configured library
* 2017-04-02. Giulio.     @ADDED support for creation of BigTiff files
* 2017-04-02. Giulio.     @FIXED a memory leak in 'openTiff3DFile'
* 2016-11-27. Giulio.     @FIXED bug in 'initTiff3DFile': the fake buffer was allocated before updating spp
* 2016-10-07. Giulio.     @CHANGED spp is set to 1 if teg SAMPLESPERPIXEL is not defined 
* 2016-09-10. Giulio.     @ADDED support for reading internally tiled images 
* 2016-06-17. Giulio.     @ADDED the possibility of performing downsampling on-the-fly when reading an image
* 2016-06-17. Giulio.     @ADDED the ability to control the number of rows per strip (rps) 
* 2015-03-03. Giulio.     @FIXED RGB photometric interprettion has to be set when there is more than one channel 
* 2015-02-06. Giulio.     @ADDED append operation that assume an already open and positioned file
* 2015-02-06. Giulio.     @ADDED open operation
* 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
* 2014-12-10. Giulio.     @ADDED added management of mismatch between machine/image endian
* 2014-12-06. Giulio.     @FIXED input file should NOT be closed at the end of 'loadTiff3D2Metadata'
* 2014-12-05. Giulio.     @ADDED input file should be closed at the end of 'loadTiff3D2Metadata'
*/

#include "Tiff3DMngr.h"
#include <stdlib.h> // needed by clang: defines size_t
#include <string.h>
#include "tiffio.h"
#ifndef _VAA3D_TERAFLY_PLUGIN_MODE
#include "tiffiop.h"
#endif
#include "IM_config.h"

#include <iostream>
using namespace std;

#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
#include <QElapsedTimer>
#include "PLog.h"
#include "COperation.h"
#endif

#define GBSIZE ((iim::sint64) (1024*1024*1024))

static
void copydata ( unsigned char *psrc, uint32 stride_src, unsigned char *pdst, uint32 stride_dst, uint32 width, uint32 len ) {
    uint32 i;
    for ( uint32 i=0; i<len; i++, psrc+=stride_src, pdst+=stride_dst )
        memcpy(pdst, psrc, width*sizeof(unsigned char));
}

static bool unconfigured = true;
static bool compressed = true;
static int32 rowsPerStrip = 1;
static bool bigtiff = false;

void setLibTIFFcfg ( bool cmprssd,  bool _bigtiff,  int rps ) {
    bigtiff = _bigtiff;
    if ( unconfigured ) {
        compressed = cmprssd;
        if ( compressed ) {
            rowsPerStrip = rps;
        }
        unconfigured = false;
    }
    else
        iim::warning(iim::strprintf("setLibTIFFcfg ( bool cmprssd = %s, unsigned int rps = %d )", cmprssd ? "true" : "false", rps).c_str());
}


void resetLibTIFFcfg ( bool cmprssd,  bool _bigtiff,  int rps ) {
    bigtiff = _bigtiff;
    compressed = cmprssd;
    if ( compressed ) {
        rowsPerStrip = rps;
    }
    unconfigured = false;
}


char *loadTiff3D2Metadata ( char * filename, unsigned int &sz0, unsigned int  &sz1, unsigned int  &sz2, unsigned int  &sz3, int &datatype, int &b_swap, void * &fhandle, int &header_len ) {

    // 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_START(TiffLoadMetadata)
        #endif

            uint32 XSIZE;
    uint32 YSIZE;
    uint16 bpp;
    uint16 spp;
    uint16 Cpage;
    uint16 Npages;
    TIFF *input;
    int check;

    //disable warning and error handlers to avoid messages on unrecognized tags
    TIFFSetWarningHandler(0);
    TIFFSetErrorHandler(0);

    input=TIFFOpen(filename,"r");
    if (!input)
    {
        //throw iim::IOException(strprintf("in IOManager::readTiffMultipage(...): Cannot open the file %s",finName).c_str());
        return ((char *) "Cannot open the file.");
    }

    check=TIFFGetField(input, TIFFTAG_IMAGEWIDTH, &XSIZE);
    if (!check)
    {
        TIFFClose(input);
        //throw iim::IOException(strprintf("in IOManager::readTiffMultipage(...): Image length of %s undefined\n", finName).c_str());
        return ((char *) "Image width of undefined.");
    }
    
    check=TIFFGetField(input, TIFFTAG_IMAGELENGTH, &YSIZE);
    if (!check)
    {
        TIFFClose(input);
        //throw iim::IOException(strprintf("in IOManager::readTiffMultipage(...): Image length of %s undefined\n", finName).c_str());
        return ((char *) "Image length of undefined.");
    }
    
    check=TIFFGetField(input, TIFFTAG_BITSPERSAMPLE, &bpp);
    if (!check)
    {
        TIFFClose(input);
        //throw iim::IOException(strprintf("in IOManager::readTiffMultipage(...): Undefined bits per sample in %s \n", finName).c_str());
        return ((char *) "Undefined bits per sample.");
    }

    check=TIFFGetField(input, TIFFTAG_SAMPLESPERPIXEL, &spp);
    if (!check)
    {
        spp = 1;
        //TIFFClose(input);
        //throw iim::IOException(strprintf("in IOManager::readTiffMultipage(...): Undefined bits per sample in %s \n", finName).c_str());
        //return ((char *) "Undefined samples per pixel.");
    }

    // Onofri
    check=TIFFGetField(input, TIFFTAG_PAGENUMBER, &Cpage, &Npages);
    if (!check || Npages==0) { // the tag has not been read correctly
        // Add warning?
        Npages = 0;
        do {
            Npages++;
        } while ( TIFFReadDirectory(input) );
    }

    sz0 = XSIZE;
    sz1 = YSIZE;
    sz2 = Npages;
    sz3 = spp;
    datatype = bpp/8;

    //b_swap = 0;
    b_swap=TIFFIsByteSwapped(input);
    fhandle = (void *) input;
    header_len = -1;

    // the file must non be closed (it is responsibility of the caller)
    //TIFFClose(input);

    // 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_STOP(TiffLoadMetadata, tf::IO, tf::strprintf("successfully loaded metadata from file \"%s\"", filename))
        #endif

            return ((char *) 0);
}

char *openTiff3DFile ( char *filename, char *mode, void *&fhandle, bool reopen ) {
    char *completeFilename = (char *) 0;
    char *completeMode     = (char *) 0;
    int fname_len = (int) strlen(filename);
    char *suffix = strstr(filename,".tif");
    while ( suffix && (fname_len - (suffix-filename) > 5) )
        suffix = strstr(suffix+4,".tif");
    //if ( (suffix != 0) && (fname_len - (suffix-filename) <= 5) ) { // a substring ".tif is already at the end of the filename
    if ( suffix ) { // a substring ".tif is already at the very end of the filename
        completeFilename = new char[fname_len+1];
        strcpy(completeFilename,filename);
    }
    else {
        completeFilename = new char[fname_len+4+1];
        strcpy(completeFilename,filename);
        strcat(completeFilename,".");
        strcat(completeFilename,TIFF3D_SUFFIX);
    }

    //disable warning and error handlers to avoid messages on unrecognized tags
    TIFFSetWarningHandler(0);
    TIFFSetErrorHandler(0);

    // if mode is 'w' and reopen is true check if the file esists
    bool mybigtiff = bigtiff;
    if ( mode[0] == 'w' && reopen ) {
        fhandle = TIFFOpen(completeFilename,"r");
        if ( fhandle ) {
#ifndef _VAA3D_TERAFLY_PLUGIN_MODE
            // get the file format (classic tiff of bigtiff)
            if ( ((TIFF *) fhandle)->tif_flags&TIFF_BIGTIFF )
                mybigtiff = true;
#endif
            TIFFClose((TIFF *) fhandle);
        }
    }

    if ( mybigtiff && !strstr(mode,"8") ) {
        completeMode = new char[strlen(mode)+2];
        strcpy(completeMode,mode);
        strcat(completeMode,"8");
    }
    else {
        completeMode = new char[strlen(mode)+2];
        strcpy(completeMode,mode);
    }

    fhandle = TIFFOpen(completeFilename,completeMode);

    delete completeFilename;
    delete completeMode;

    if (!fhandle)
    {
        return ((char *) "Cannot open the file.");
    }
    return ((char *) 0);
}

void closeTiff3DFile ( void *fhandle ) {
    TIFFClose((TIFF *) fhandle);
}

// modify initTiff3DFile to save a chunk tif image with all zeros by YY 2/14
char *initTiff3DFile ( char *filename, unsigned int sz0, unsigned int sz1, unsigned int sz2, unsigned int sz3, int datatype ) {
    //int initTiff3DFile ( char *filename, uint32 XSIZE, uint32 YSIZE, uint16 spp, uint16 Npages, int datatype){

    // 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
    #ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_START(TiffInitData);
    #endif

    uint32 XSIZE  = sz0;
    uint32 YSIZE  = sz1;
    uint16 Npages = sz2;
    uint16 spp    = sz3;

    uint16 bpp=8 * datatype;

    int check;

    if ( sz3 == 1 )
        spp = sz3;
    else if ( sz3 < 4 )
        spp = 3;
    else
        return ((char *) "More than 3 channels in Tiff files.");

    //
    long szSlice = ((iim::sint64) XSIZE) * ((iim::sint64) YSIZE) * spp * (bpp/8);
    unsigned char *fakeData=NULL;
    try
    {
        fakeData = new unsigned char[ szSlice ];
        memset(fakeData,0,szSlice);
    }
    catch(...)
    {
        return ((char *)"Fail to alloc memory\n");
    }

    char *completeFilename = (char *) 0;
    int fname_len = (int) strlen(filename);
    char *suffix = strstr(filename,".tif");
    while ( suffix && (fname_len - (suffix-filename) > 5) )
        suffix = strstr(suffix+4,".tif");
    //if ( (suffix != 0) && (fname_len - (suffix-filename) <= 5) ) { // a substring ".tif is already at the end of the filename
    if ( suffix ) { // a substring ".tif is already at the very end of the filename
        completeFilename = new char[fname_len+1];
        strcpy(completeFilename,filename);
    }
    else {
        completeFilename = new char[fname_len+4+1];
        strcpy(completeFilename,filename);
        strcat(completeFilename,".");
        strcat(completeFilename,TIFF3D_SUFFIX);
    }


    // 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
    #ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_STOP(TiffInitData, tf::CPU, tf::strprintf("generated fake data for 3D tiff \"%s\"", completeFilename));
    TERAFLY_TIME_RESTART(TiffInitData);
    #endif

    //disable warning and error handlers to avoid messages on unrecognized tags
    TIFFSetWarningHandler(0);
    TIFFSetErrorHandler(0);

    TIFF *output;

    iim::sint64 expectedSize = ((iim::sint64) sz0) * ((iim::sint64) sz1) * ((iim::sint64) sz2) * ((iim::sint64) sz3) * ((iim::sint64) datatype);

    if ( bigtiff || expectedSize > (4*GBSIZE) ) {
        if ( (rowsPerStrip == -1 && (((iim::sint64) sz0) * ((iim::sint64) sz1)) > (4*GBSIZE)) || ((rowsPerStrip * ((iim::sint64) sz0)) > (4*GBSIZE)) )
            // one strip is larger than 4GB
            return ((char *) "Too many rows per strip for this image width.");
        else
            output = TIFFOpen(completeFilename,"w8");
    }
    else
        output = TIFFOpen(completeFilename,"w");


    if (!output) {
        delete completeFilename;
        return ((char *) "Cannot open the file.");
    }

    check = TIFFSetField(output, TIFFTAG_IMAGEWIDTH, XSIZE);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the image width.");
    }

    check = TIFFSetField(output, TIFFTAG_IMAGELENGTH, YSIZE);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the image height.");
    }

    check = TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, bpp);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the image bit per sample.");
    }

    check = TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, spp);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the image sample per pixel.");
    }

    check = TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, (rowsPerStrip == -1) ? YSIZE : (uint32)rowsPerStrip);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the image rows per strip.");
    }

    check = TIFFSetField(output, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the image orientation.");
    }

    check = TIFFSetField(output, TIFFTAG_COMPRESSION, compressed ? COMPRESSION_LZW : COMPRESSION_NONE);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the compression tag.");
    }

    check = TIFFSetField(output, TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the planarconfig tag.");
    }

    if ( spp == 1 )
        check = TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    else // spp == 3
        check = TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the photometric tag.");
    }

    /* We are writing single page of the multipage file */
    check = TIFFSetField(output, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the subfiletype tag.");
    }

    check = TIFFSetField(output, TIFFTAG_PAGENUMBER, 0, Npages);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the page number.");
    }

    //check = (int)TIFFWriteEncodedStrip(output, 0, fakeData, XSIZE * YSIZE);
    //if (!check) {
    //	delete completeFilename;
    //	return ((char *) "Cannot write encoded strip to file.");
    //   }

    if ( rowsPerStrip == -1 ) {
        TIFFSetDirectory(output,0);
        check = TIFFWriteEncodedStrip(output, 0, fakeData, szSlice);
        if (!check) {
            delete completeFilename;
            return ((char *) "Cannot write encoded strip to file.");
        }
    }
    else {
        int check,StripsPerImage,LastStripSize;
        uint32 rps = (uint32)rowsPerStrip;
        unsigned char *buf = fakeData;

        StripsPerImage =  (YSIZE + rps - 1) / rps;
        LastStripSize = YSIZE % rps;
        if (LastStripSize==0)
            LastStripSize=rps;

        for (int i=0; i < StripsPerImage-1; i++){
            check = TIFFWriteEncodedStrip(output, i, buf, spp * rps * XSIZE * (bpp/8));
            if (!check) {
                delete completeFilename;
                return ((char *) "Cannot write encoded strip to file.");
            }
            buf = buf + spp * rps * XSIZE * (bpp/8);
        }

        check = TIFFWriteEncodedStrip(output, StripsPerImage-1, buf, spp * LastStripSize * XSIZE * (bpp/8));
        if (!check) {
            delete completeFilename;
            return ((char *) "Cannot write encoded strip to file.");
        }
        //}
        buf = buf + spp * LastStripSize * XSIZE * (bpp/8);
    }

    if(fakeData)
        delete[] fakeData;

    check = TIFFWriteDirectory(output);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot write a new directory.");
    }

    TIFFClose(output);

    // 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
    #ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_STOP(TiffInitData, tf::IO, tf::strprintf("written initialized 3D tiff \"%s\"", completeFilename));
    #endif

    delete completeFilename;

    return (char *) 0;
}

char *appendSlice2Tiff3DFile ( char *filename, int slice, unsigned char *img, unsigned int img_width, unsigned int img_height ) {
    // 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
    #ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_START(TiffAppendData);
    #endif

    TIFF *output;
    uint16 spp, bpp, NPages, pg0;
    uint32 rperstrip;
    uint16 cmprssd;
    uint16 photomtrcintrp;

    //disable warning and error handlers to avoid messages on unrecognized tags
    TIFFSetWarningHandler(0);
    TIFFSetErrorHandler(0);

    output=TIFFOpen(filename,"r");
    TIFFGetField(output, TIFFTAG_BITSPERSAMPLE, &bpp);
    TIFFGetField(output, TIFFTAG_SAMPLESPERPIXEL, &spp);
    TIFFGetField(output, TIFFTAG_PAGENUMBER, &pg0, &NPages);
    TIFFGetField(output, TIFFTAG_ROWSPERSTRIP, &rperstrip);
    TIFFGetField(output, TIFFTAG_COMPRESSION, &cmprssd);
    TIFFGetField(output, TIFFTAG_PHOTOMETRIC, &photomtrcintrp);
    TIFFClose(output);

    iim::sint64 expectedSize = ((iim::sint64) img_width) * ((iim::sint64) img_height) * ((iim::sint64) NPages) * ((iim::sint64) spp) * ((iim::sint64) (bpp/8));

    if ( bigtiff || expectedSize > (4*GBSIZE) ) {
        if ( (rowsPerStrip == -1 && (((iim::sint64) img_width) * ((iim::sint64) img_height)) > (4*GBSIZE)) || ((rowsPerStrip * ((iim::sint64) img_width)) > (4*GBSIZE)) )
            // one strip is larger than 4GB
            return ((char *) "Too many rows per strip for this image width.");
        else
            output = (slice==0)? TIFFOpen(filename,"w8") : TIFFOpen(filename,"a8");
    }
    else
        output = (slice==0)? TIFFOpen(filename,"w") : TIFFOpen(filename,"a");

    TIFFSetDirectory(output,slice); // WARNING: slice must be the first page after the last, otherwise the file can be corrupted

    TIFFSetField(output, TIFFTAG_IMAGEWIDTH, img_width);
    TIFFSetField(output, TIFFTAG_IMAGELENGTH, img_height);
    TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, bpp);
    TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, spp);
    TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, rperstrip);
    TIFFSetField(output, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(output, TIFFTAG_COMPRESSION, cmprssd);
    TIFFSetField(output, TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
    TIFFSetField(output, TIFFTAG_PHOTOMETRIC, photomtrcintrp);
    // We are writing single page of the multipage file
    TIFFSetField(output, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
    TIFFSetField(output, TIFFTAG_PAGENUMBER, (uint16)slice, NPages);

    if ( rowsPerStrip == -1 )
        TIFFWriteEncodedStrip(output, 0, img, img_width * img_height * spp * (bpp/8));
    else {
        int check,StripsPerImage,LastStripSize;
        uint32 rps = rperstrip;
        unsigned char *buf = img;

        StripsPerImage =  (img_height + rps - 1) / rps;
        LastStripSize = img_height % rps;
        if (LastStripSize==0)
            LastStripSize=rps;

        for (int i=0; i < StripsPerImage-1; i++){
            TIFFWriteEncodedStrip(output, i, buf, spp * rps * img_width * (bpp/8));
            buf = buf + spp * rps * img_width * (bpp/8);
        }

        TIFFWriteEncodedStrip(output, StripsPerImage-1, buf, spp * LastStripSize * img_width * (bpp/8));
        buf = buf + spp * LastStripSize * img_width * (bpp/8);
    }
    //img +=  img_width * img_height;

    TIFFWriteDirectory(output);

    TIFFClose(output);

    // 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
    #ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_STOP(TiffAppendData, tf::IO, tf::strprintf("appended slice %d x %d to 3D tiff \"%s\"", img_width, img_height, filename));
    #endif

    return (char *) 0;
}

char *appendSlice2Tiff3DFile ( void *fhandler, int slice, unsigned char *img, unsigned int  img_width, unsigned int  img_height, int spp, int bpp, int NPages ) {
    TIFF *output = (TIFF *) fhandler;

    TIFFSetDirectory(output,slice); // WARNING: slice must be the first page after the last, otherwise the file can be corrupted

    TIFFSetField(output, TIFFTAG_IMAGEWIDTH, img_width);
    TIFFSetField(output, TIFFTAG_IMAGELENGTH, img_height);
    TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, (uint16)bpp);
    TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, (uint16)spp);
    TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, (rowsPerStrip == -1) ? img_height : (uint32)rowsPerStrip);
    TIFFSetField(output, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(output, TIFFTAG_COMPRESSION, compressed ? COMPRESSION_LZW : COMPRESSION_NONE);
    TIFFSetField(output, TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
    if ( spp == 1 )
        TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    else // spp == 3
        TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    // We are writing single page of the multipage file
    TIFFSetField(output, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
    TIFFSetField(output, TIFFTAG_PAGENUMBER, (uint16)slice, (uint16)NPages);

    // the file has been already opened: rowsPerStrip it is not too large for this image width
    if ( rowsPerStrip == -1 )
        TIFFWriteEncodedStrip(output, 0, img, img_width * img_height * spp * (bpp/8));
    else {
        int check,StripsPerImage,LastStripSize;
        uint32 rps = (uint32)rowsPerStrip;
        unsigned char *buf = img;

        StripsPerImage =  (img_height + rps - 1) / rps;
        LastStripSize = img_height % rps;
        if (LastStripSize==0)
            LastStripSize=rps;

        for (int i=0; i < StripsPerImage-1; i++){
            TIFFWriteEncodedStrip(output, i, buf, spp * rps * img_width * (bpp/8));
            buf = buf + spp * rps * img_width * (bpp/8);
        }

        TIFFWriteEncodedStrip(output, StripsPerImage-1, buf, spp * LastStripSize * img_width * (bpp/8));
        buf = buf + spp * LastStripSize * img_width * (bpp/8);
    }
    //img +=  img_width * img_height;

    TIFFWriteDirectory(output);

    return (char *) 0;
}

char *readTiff3DFile2Buffer ( char *filename, unsigned char *img, unsigned int img_width, unsigned int img_height, unsigned int first, unsigned int last, 
                              int downsamplingFactor, int starti, int endi, int startj, int endj ) {

    // 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_START(TiffLoadData)
        #endif

            TIFF *input;

    //disable warning and error handlers to avoid messages on unrecognized tags
    TIFFSetWarningHandler(0);
    TIFFSetErrorHandler(0);

    input=TIFFOpen(filename,"r");
    if (!input)
    {
        //throw iim::IOException(strprintf("in IOManager::readTiffMultipage(...): Cannot open the file %s",finName).c_str());
        return ((char *) "Cannot open the file.");
    }
    
    int b_swap=TIFFIsByteSwapped(input);
    char *err_msg = readTiff3DFile2Buffer(input,img,img_width,img_height,first,last,b_swap,downsamplingFactor,starti,endi,startj,endj);

    TIFFClose(input);

    // 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_STOP(TiffLoadData, tf::IO, tf::strprintf("loaded block x(%d), y(%d), z(%d-%d) from 3D tiff \"%s\"", img_width, img_height, first, last, filename))
        #endif

            return err_msg;
}

char *readTiff3DFile2Buffer ( void *fhandler, unsigned char *img, unsigned int img_width, unsigned int img_height, unsigned int first, unsigned int last, 
                              int b_swap, int downsamplingFactor, int starti, int endi, int startj, int endj ) {
    uint32 rps;
    uint16 spp, bpp, orientation, photo, comp, planar_config;
    int check, StripsPerImage,LastStripSize;
    uint32 XSIZE;
    uint32 YSIZE;

    TIFF *input = (TIFF *) fhandler;

    check=TIFFGetField(input, TIFFTAG_BITSPERSAMPLE, &bpp);
    if (!check)
    {
        return ((char *) "Undefined bits per sample.");
    }

    check=TIFFGetField(input, TIFFTAG_SAMPLESPERPIXEL, &spp);
    if (!check)
    {
        spp = 1;
        //return ((char *) "Undefined samples per pixel.");
    }

    check=TIFFGetField(input, TIFFTAG_PHOTOMETRIC, &photo);
    if (!check)
    {
        return ((char *) "Cannot determine photometric interpretation.");
    }

    check=TIFFGetField(input, TIFFTAG_COMPRESSION, &comp);
    if (!check)
    {
        return ((char *) "Cannot determine compression technique.");
    }

    check=TIFFGetField(input, TIFFTAG_PLANARCONFIG, &planar_config);
    if (!check)
    {
        return ((char *) "Cannot determine planar configuration.");
    }

    starti = (starti == -1) ? 0 : starti;
    endi   = (endi == -1) ? img_height-1 : endi;
    startj = (startj == -1) ? 0 : startj;
    endj   = (endj == -1) ? img_width-1 : endj;

    check=TIFFIsTiled(input);
    if (check) // file is internally tiled
    {
        uint32 tilewidth;
        uint32 tilelength;
        uint32 tiledepth;
        tsize_t tilenum;
        tsize_t tilesize;
        tsize_t tilenum_width;
        tsize_t tilenum_length;
        ttile_t tile;
        tdata_t data;
        unsigned char *psrc; // pointer in the tile buffer to the top left pixel of the current block to be copied
        unsigned char *pdst; // pointer in the image buffer to the top left pixel of the current block to be filled
        uint32 stride_src;
        uint32 stride_dst;
        int i; // row index in the slice of the top left pixel of the current block to be filled
        int j; // column index in the slice of the top left pixel of the current block to be filled
        uint32 width; // width of the current block to be filled (in pixels)
        uint32 len; // length of the current block to be filled (in pixels)
        int page;

        // checks
        if ( TIFFGetField(input, TIFFTAG_TILEDEPTH, &tiledepth) )
            return ((char *) "Tiling among slices (z direction) not supported.");
        if ( spp > 1 )
            if ( TIFFGetField(input, TIFFTAG_PLANARCONFIG, &planar_config) )
                if ( planar_config > 1 )
                    return ((char *) "Non-interleaved multiple channels not supported with tiling.");

        // tiling is in x,y only
        TIFFGetField(input, TIFFTAG_TILEWIDTH, &tilewidth);
        TIFFGetField(input, TIFFTAG_TILELENGTH, &tilelength);
        tilenum = TIFFNumberOfTiles(input);
        tilesize = TIFFTileSize(input);
        tilenum_width  = (img_width % tilewidth) ? (img_width / tilewidth) + 1 : img_width / tilewidth;
        tilenum_length = (img_height % tilelength) ? (img_height / tilelength) + 1 : img_height / tilelength;

        data = new unsigned char[tilesize];
        stride_src = tilewidth * spp; // width of tile (in bytes)
        stride_dst = (endj - startj + 1) * spp; // width of subregion (in bytes)

        page = 0;
        do {

            psrc = ((unsigned char *)data) + ((starti % tilelength)*tilewidth + (startj % tilewidth)) * spp; // in the first tile skip (starti % tilelength) rows plus (startj % tilewidth) pixels
            pdst = img; // the buffer has the size of the subregion
            len = tilelength - (starti % tilelength); // rows to be copied for the first row of tiles
            tile = TIFFComputeTile(input,startj,starti,0,0); // index of the first tile to be copied in the current row of tiles
            for ( i=starti; i<=endi; ) {
                width = tilewidth - (startj%tilewidth); // width of the first block to be copied/filled
                for ( j=startj; j<=endj; ) {
                    TIFFReadEncodedTile(input,tile,data,(tsize_t) -1); // read tile into tile buffer
                    copydata (psrc,stride_src,pdst,stride_dst,(width * spp),len); // copy the block
                    j += width;
                    tile++; // index of the next tile in the same row of tiles
                    psrc = ((unsigned char *)data) + ((i % tilelength)*tilewidth) * spp; // the block in the next tile begins just after (i % tilelength) rows
                    pdst += width * spp; // the block in the image buffer move forward of width pixels
                    width = (((tile%tilenum_width) + 1) * tilewidth <= (endj+1)) ? tilewidth : ((endj+1)%tilewidth); // if the next tile in the row is all within the subregion, width is tilewidth otherwise it is shorter
                }
                i += len;
                tile = TIFFComputeTile(input,startj,i,0,0); // index of the first tile to be copied in the current row of tiles
                psrc = ((unsigned char *)data) + ((i % tilelength)*tilewidth + (startj % tilewidth)) * spp; // in the first tile of the next row of tiles skip (i % tilelength) rows plus (startj % tilewidth) pixels
                pdst = img + ((i-starti) * stride_dst); // the block in the image buffer begin after (i-starti) rows
                len = (((tile/tilenum_width) + 1) * tilelength <= (endi+1)) ? tilelength : ((endi+1)%tilelength); // if the next row of tiles is all within the subregion, len is tilelength otherwise it is shorter
            }

            page++;

        }while ( page < static_cast<int>(last-first+1) && TIFFReadDirectory(input));

        return (char *) 0;
    }

    check=TIFFGetField(input, TIFFTAG_ROWSPERSTRIP, &rps);
    if (!check)
    {
        return ((char *) "Undefined rows per strip.");
    }
    
    //check=TIFFGetField(input, TIFFTAG_ORIENTATION, &orientation);
    //if (!check)
    //{
    //	return ((char *) "Image orientation undefined.");
    //}
    
    StripsPerImage =  (img_height + rps - 1) / rps;
    LastStripSize = img_height % rps;
    if (LastStripSize==0)
        LastStripSize=rps;

    unsigned char *buf = img;
    int page=0;

    if ( downsamplingFactor == 1 ) { // read without downsampling

        if ( starti < 0 || endi >= img_height || startj < 0 || endj >= img_width || starti >= endi || startj >= endj )
        {
            return ((char *) "Wrong substack indices.");
        }

        if ( starti == 0 && endi == (img_height-1) && startj == 0 && endj == (img_width-1) ) { // read whole images from files

            check=TIFFSetDirectory(input, first);
            if (!check)
            {
                return ((char *) "Cannot open the requested first strip.");
            }

            do{

                for (int i=0; i < StripsPerImage-1; i++){
                    if (comp==1) {
                        TIFFReadRawStrip(input, i, buf, spp * rps * img_width * (bpp/8));
                        buf = buf + spp * rps * img_width * (bpp/8);
                    }
                    else{
                        TIFFReadEncodedStrip(input, i, buf, spp * rps * img_width * (bpp/8));
                        buf = buf + spp * rps * img_width * (bpp/8);
                    }
                }

                if (comp==1) {
                    TIFFReadRawStrip(input, StripsPerImage-1, buf, spp * LastStripSize * img_width * (bpp/8));
                }
                else{
                    TIFFReadEncodedStrip(input, StripsPerImage-1, buf, spp * LastStripSize * img_width * (bpp/8));
                }
                buf = buf + spp * LastStripSize * img_width * (bpp/8);

                page++;

            }while ( page < static_cast<int>(last-first+1) && TIFFReadDirectory(input));//while (TIFFReadDirectory(input));

        }
        else { // read only a subregion of images from files

            check=TIFFGetField(input, TIFFTAG_IMAGEWIDTH, &XSIZE);
            if (!check)
            {
                return ((char *) "Image width of undefined.");
            }
            check=TIFFGetField(input, TIFFTAG_IMAGELENGTH, &YSIZE);
            if (!check)
            {
                return ((char *) "Image length of undefined.");
            }

            unsigned char *rowbuf = new unsigned char[spp * rps * XSIZE * (bpp/8)];
            unsigned char *bufptr;

            do{
                check=TIFFSetDirectory(input, first + page);
                if (!check)
                {
                    return ((char *) "Cannot open next requested strip.");
                }

                int stripIndex = (starti / rps) - 1; // the strip preceeding the first one
                for (int i=starti; i <= endi; i++) {
                    if ( floor((double)i / rps) > stripIndex ) { // read a new strip
                        stripIndex = (int)floor((double)i / rps);
                        if (comp==1) {
                            TIFFReadRawStrip(input, stripIndex, rowbuf, spp * ((stripIndex < StripsPerImage) ? rps :LastStripSize) * XSIZE * (bpp/8));
                        }
                        else{
                            TIFFReadEncodedStrip(input, stripIndex, rowbuf, spp * ((stripIndex < StripsPerImage) ? rps :LastStripSize) * XSIZE * (bpp/8));
                        }
                    }
                    bufptr = rowbuf + (i % rps) * (spp * XSIZE * (bpp/8));
                    if ( bpp == 8 )
                        for (int j=0, j1=startj; j<=(endj-startj); j++, j1++) {
                            for (int c=0; c<spp; c++) {
                                buf[j * spp + c] = bufptr[j1 * spp + c];
                            }
                        }
                    else
                        for (int j=0 , j1=startj; j<=(endj-startj); j++, j1++) {
                            for (int c=0; c<spp; c++) {
                                ((uint16 *)buf)[j * spp + c] = ((uint16 *)bufptr)[j1 * spp + c];
                            }
                        }
                    buf = buf + spp * (endj-startj+1) * (bpp/8);
                }

                page++;

            }while ( page < static_cast<int>(last-first+1) );

            delete []rowbuf;

        }

        // input file is assumedo ti be already open and it is provided as an handler; the file should be closed by caller
        //TIFFClose(input);

        if ( page < static_cast<int>(last-first+1) ){
            return ((char *) "Cannot read all the pages.");
        }
    }
    else { // read with downsampling

        // preliminary checks
        if ( starti != 0 || endi != (img_height-1) || startj != 0 || endj != (img_width-1) ) { // a subregion has been requested
            return ((char *) "Subregion extraction not supported with downsampling.");
        }
        check=TIFFGetField(input, TIFFTAG_IMAGEWIDTH, &XSIZE);
        if (!check)
        {
            return ((char *) "Image width of undefined.");
        }
        check=TIFFGetField(input, TIFFTAG_IMAGELENGTH, &YSIZE);
        if (!check)
        {
            return ((char *) "Image length of undefined.");
        }

        if ( (int)ceil((double)XSIZE/downsamplingFactor) < img_width )
        {
            return ((char *) "Requested image width too large.");
        }
        if ( (int)ceil((double)YSIZE/downsamplingFactor) < img_height )
        {
            return ((char *) "Requested image height too large.");
        }

        unsigned char *rowbuf = new unsigned char[spp * rps * XSIZE * (bpp/8)];
        unsigned char *bufptr;

        do{
            check=TIFFSetDirectory(input, ((first + page) * downsamplingFactor));
            if (!check)
            {
                return ((char *) "Cannot open next requested strip.");
            }

            int stripIndex = -1; // the strip preceeding the first one
            for (int i=0; i < img_height; i++) {
                if ( floor(i * downsamplingFactor / (double)rps) > stripIndex ) { // read a new strip
                    stripIndex = (int)floor(i * downsamplingFactor / (double)rps);
                    if (comp==1) {
                        TIFFReadRawStrip(input, stripIndex, rowbuf, spp * ((stripIndex < StripsPerImage) ? rps :LastStripSize) * XSIZE * (bpp/8));
                    }
                    else{
                        TIFFReadEncodedStrip(input, stripIndex, rowbuf, spp * ((stripIndex < StripsPerImage) ? rps :LastStripSize) * XSIZE * (bpp/8));
                    }
                }
                bufptr = rowbuf + ((i * downsamplingFactor) % rps) * (spp * XSIZE * (bpp/8));
                if ( bpp == 8 )
                    for (int j=0; j<img_width; j++) {
                        for (int c=0; c<spp; c++) {
                            buf[j * spp + c] = bufptr[j * spp * downsamplingFactor + c];
                        }
                    }
                else
                    for (int j=0; j<img_width; j++) {
                        for (int c=0; c<spp; c++) {
                            ((uint16 *)buf)[j * spp + c] = ((uint16 *)bufptr)[j * spp * downsamplingFactor + c];
                        }
                    }
                buf = buf + spp * img_width * (bpp/8);
            }

            page++;

        }while ( page < static_cast<int>(last-first+1) );

        delete []rowbuf;
    }
    
    // swap the data bytes if necessary
    if (b_swap)
    {
        int i;
        size_t total = img_width * img_height * spp * (last-first+1);
        if (bpp/8 == 2)
        {
            for (i=0;i<total; i++)
            {
                iim::swap2bytes((void *)(img+2*i));
            }
        }
        else if (bpp/8 == 4)
        {
            for (i=0;i<total; i++)
            {
                iim::swap4bytes((void *)(img+4*i));
            }
        }
    }

    return (char *) 0;
}


// save a chunk image as a tif file
char *initTiffFile(char *filename, unsigned int sz0, unsigned int  sz1, unsigned int  sz2, unsigned int  sz3, int datatype)
{
    cout<<"initTiffFile"<<endl;

    //
    uint32 XSIZE  = sz0;
    uint32 YSIZE  = sz1;
    uint16 Npages = sz2;
    uint16 spp    = sz3;

    uint16 bpp=8 * datatype;

    if ( sz3 == 1 )
        spp = sz3;
    else if ( sz3 < 4 )
        spp = 3;
    else
        return ((char *) "More than 3 channels in Tiff files.");

    char *completeFilename = NULL;
    int fname_len = (int) strlen(filename);
    char *suffix = strstr(filename,".tif");
    while ( suffix && (fname_len - (suffix-filename) > 5) )
        suffix = strstr(suffix+4,".tif");
    if ( suffix ) { // a substring ".tif is already at the very end of the filename
        completeFilename = new char[fname_len+1];
        strcpy(completeFilename,filename);
    }
    else {
        completeFilename = new char[fname_len+4+1];
        strcpy(completeFilename,filename);
        strcat(completeFilename,".");
        strcat(completeFilename,TIFF3D_SUFFIX);
    }

    //
    TIFF *output = NULL;

    iim::sint64 expectedSize = ((iim::sint64) sz0) * ((iim::sint64) sz1) * ((iim::sint64) sz2) * ((iim::sint64) sz3) * ((iim::sint64) datatype);

    if ( bigtiff || expectedSize > (4*GBSIZE) ) {
        if ( (rowsPerStrip == -1 && (((iim::sint64) sz0) * ((iim::sint64) sz1)) > (4*GBSIZE)) || ((rowsPerStrip * ((iim::sint64) sz0)) > (4*GBSIZE)) )
            // one strip is larger than 4GB
            return ((char *) "Too many rows per strip for this image width.");
        else
            output = TIFFOpen(completeFilename,"w8");
    }
    else
    {
        output = TIFFOpen(completeFilename,"w");
    }


    if (!output) {
        delete completeFilename;
        return ((char *) "Cannot open the file.");
    }

    //
    uint32 szPage = XSIZE * YSIZE * spp * datatype;

    unsigned char *img = NULL;
    try
    {
        img = new unsigned char [szPage];
        memset(img, 0, szPage);
    }
    catch(...)
    {
        return ((char *)"fail to alloc memory");
    }

    cout<<"saving "<<Npages<<" slices to "<<filename<<endl;
    cout<<"size "<<XSIZE<<" "<<YSIZE<<" "<<spp<<" "<<datatype<<endl;
    printf("image[1] = %d\n", img[1]);

    // the file has been already opened: rowsPerStrip it is not too large for this image width
    if ( rowsPerStrip == -1 )
    {
        //
        for(int slice=0; slice<Npages; slice++)
        {
            cout<<"slice #"<<slice<<endl;

            TIFFSetDirectory(output,slice);

            TIFFSetField(output, TIFFTAG_IMAGEWIDTH, XSIZE);
            TIFFSetField(output, TIFFTAG_IMAGELENGTH, YSIZE);
            TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, (uint16)bpp);
            TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, (uint16)spp);
            TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, (rowsPerStrip == -1) ? YSIZE : (uint32)rowsPerStrip);
            TIFFSetField(output, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
            TIFFSetField(output, TIFFTAG_COMPRESSION, compressed ? COMPRESSION_LZW : COMPRESSION_NONE);
            TIFFSetField(output, TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
            if ( spp == 1 )
                TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
            else // spp == 3
                TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

            // We are writing single page of the multipage file
            TIFFSetField(output, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
            TIFFSetField(output, TIFFTAG_PAGENUMBER, (uint16)slice, (uint16)Npages);

            //
            TIFFWriteEncodedStrip(output, 0, img, szPage);

            //
            TIFFWriteDirectory(output);
        }
    }
    else
    {
        int StripsPerImage,LastStripSize;
        uint32 rps = (uint32)rowsPerStrip;
        unsigned char *buf = img;

        StripsPerImage =  (YSIZE + rps - 1) / rps;
        LastStripSize = YSIZE % rps;
        if (LastStripSize==0)
            LastStripSize=rps;

        for (int i=0; i < StripsPerImage-1; i++){
            TIFFWriteEncodedStrip(output, i, buf, spp * rps * XSIZE * (bpp/8));
            buf = buf + spp * rps * XSIZE * (bpp/8);
        }

        TIFFWriteEncodedStrip(output, StripsPerImage-1, buf, spp * LastStripSize * XSIZE * (bpp/8));
        buf = buf + spp * LastStripSize * XSIZE * (bpp/8);
    }

    //
    if(img)
        delete [] img;


    //
    delete completeFilename;

    //
    TIFFClose(output);

    //
    return (char *) 0;
}

// save a chunk image as a tif file
char *writeTiff3DFile(char *filename, unsigned int sz0, unsigned int  sz1, unsigned int  sz2, unsigned int  sz3, int datatype, unsigned char *img)
{
    //
#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_START(TiffInitData);
#endif

    uint32 XSIZE  = sz0;
    uint32 YSIZE  = sz1;
    uint16 Npages = sz2;
    uint16 spp    = sz3;

    uint16 bpp=8 * datatype;

    int check;

    if ( sz3 == 1 )
        spp = sz3;
    else if ( sz3 < 4 )
        spp = 3;
    else
        return ((char *) "More than 3 channels in Tiff files.");

    char *completeFilename = (char *) 0;
    int fname_len = (int) strlen(filename);
    char *suffix = strstr(filename,".tif");
    while ( suffix && (fname_len - (suffix-filename) > 5) )
        suffix = strstr(suffix+4,".tif");
    //if ( (suffix != 0) && (fname_len - (suffix-filename) <= 5) ) { // a substring ".tif is already at the end of the filename
    if ( suffix ) { // a substring ".tif is already at the very end of the filename
        completeFilename = new char[fname_len+1];
        strcpy(completeFilename,filename);
    }
    else {
        completeFilename = new char[fname_len+4+1];
        strcpy(completeFilename,filename);
        strcat(completeFilename,".");
        strcat(completeFilename,TIFF3D_SUFFIX);
    }


    // 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_STOP(TiffInitData, tf::CPU, tf::strprintf("generated fake data for 3D tiff \"%s\"", completeFilename));
    TERAFLY_TIME_RESTART(TiffInitData);
#endif

    //disable warning and error handlers to avoid messages on unrecognized tags
    TIFFSetWarningHandler(0);
    TIFFSetErrorHandler(0);

    TIFF *output = NULL;

    iim::sint64 expectedSize = ((iim::sint64) sz0) * ((iim::sint64) sz1) * ((iim::sint64) sz2) * ((iim::sint64) sz3) * ((iim::sint64) datatype);

    if ( bigtiff || expectedSize > (4*GBSIZE) ) {
        if ( (rowsPerStrip == -1 && (((iim::sint64) sz0) * ((iim::sint64) sz1)) > (4*GBSIZE)) || ((rowsPerStrip * ((iim::sint64) sz0)) > (4*GBSIZE)) )
            // one strip is larger than 4GB
            return ((char *) "Too many rows per strip for this image width.");
        else
            output = TIFFOpen(completeFilename,"w8");
    }
    else
        output = TIFFOpen(completeFilename,"w");


    if (!output) {
        delete completeFilename;
        return ((char *) "Cannot open the file.");
    }

    check = TIFFSetField(output, TIFFTAG_IMAGEWIDTH, XSIZE);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the image width.");
    }

    check = TIFFSetField(output, TIFFTAG_IMAGELENGTH, YSIZE);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the image height.");
    }

    check = TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, bpp);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the image bit per sample.");
    }

    check = TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, spp);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the image sample per pixel.");
    }

    check = TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, (rowsPerStrip == -1) ? YSIZE : (uint32)rowsPerStrip);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the image rows per strip.");
    }

    check = TIFFSetField(output, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the image orientation.");
    }

    check = TIFFSetField(output, TIFFTAG_COMPRESSION, compressed ? COMPRESSION_LZW : COMPRESSION_NONE);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the compression tag.");
    }

    check = TIFFSetField(output, TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the planarconfig tTIFFSetDirectoryag.");
    }

    if ( spp == 1 )
        check = TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    else // spp == 3
        check = TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the photometric tag.");
    }

    /* We are writing single page of the multipage file */
    check = TIFFSetField(output, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the subfiletype tag.");
    }

    check = TIFFSetField(output, TIFFTAG_PAGENUMBER, 0, Npages);
    if (!check) {
        delete completeFilename;
        return ((char *) "Cannot set the page number.");
    }

    //
    uint32 szPage = XSIZE * YSIZE * spp * datatype;

    for(int slice=0; slice<Npages; slice++)
    {
        TIFFSetDirectory(output,slice);

        TIFFSetField(output, TIFFTAG_IMAGEWIDTH, XSIZE);
        TIFFSetField(output, TIFFTAG_IMAGELENGTH, YSIZE);
        TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, (uint16)bpp);
        TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, (uint16)spp);
        TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, (rowsPerStrip == -1) ? YSIZE : (uint32)rowsPerStrip);
        TIFFSetField(output, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
        TIFFSetField(output, TIFFTAG_COMPRESSION, compressed ? COMPRESSION_LZW : COMPRESSION_NONE);
        TIFFSetField(output, TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
        if ( spp == 1 )
            TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        else // spp == 3
            TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
        // We are writing single page of the multipage file
        TIFFSetField(output, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
        TIFFSetField(output, TIFFTAG_PAGENUMBER, (uint16)slice, (uint16)Npages);

        // the file has been already opened: rowsPerStrip it is not too large for this image width
        if ( rowsPerStrip == -1 )
        {
            //printf("offset ... %ld\n",  slice*szPage);
            TIFFWriteEncodedStrip(output, 0, img + slice*szPage, szPage);

//            for ( unsigned int ih = 0; ih < YSIZE; ih++ )
//            {
//                if ( TIFFWriteScanline(output, img, ih, 0) < 0 )
//                {
//                    printf("Error out of disk space.\n");
//                    break;
//                }
//                img += XSIZE*datatype;
//            }
        }
        else
        {
            int StripsPerImage,LastStripSize;
            uint32 rps = (uint32)rowsPerStrip;
            unsigned char *buf = img;

            StripsPerImage =  (YSIZE + rps - 1) / rps;
            LastStripSize = YSIZE % rps;
            if (LastStripSize==0)
                LastStripSize=rps;

            for (int i=0; i < StripsPerImage-1; i++){
                TIFFWriteEncodedStrip(output, i, buf, spp * rps * XSIZE * (bpp/8));
                buf = buf + spp * rps * XSIZE * (bpp/8);
            }

            TIFFWriteEncodedStrip(output, StripsPerImage-1, buf, spp * LastStripSize * XSIZE * (bpp/8));
            buf = buf + spp * LastStripSize * XSIZE * (bpp/8);
        }

        //
        TIFFWriteDirectory(output);
    }

    //
    TIFFClose(output);

    // 2015-01-30. Alessandro. @ADDED performance (time) measurement in all most time-consuming methods.
#ifdef _VAA3D_TERAFLY_PLUGIN_MODE
    TERAFLY_TIME_STOP(TiffInitData, tf::IO, tf::strprintf("written initialized 3D tiff \"%s\"", completeFilename));
#endif

    delete completeFilename;

    //
    return (char *) 0;
}
