/*
 *  stackutil.h
 *  
 *
 *  Created by Hanchuan Peng on 7/31/06.
 *  Copyright 2006 __Hanchuan Peng__. All rights reserved.
 *
 *  Updated: 060816: add tif support
 *  060828: add surfix extraction function here
 *  060920: add the 2-byte raw support for compatibility
 *  070214: add two simple interface to read any file formats supported (raw or tif) based on the surfix of filename
 *  070806: add LSM file reading functions
 *  070819: add LSM thumbnail stack reading and the middle-slice-only reading
 *  070823: add a more comprehensive function to read either 1 slice (original or thumbnail) or the entire stack (original or thumbnail) from lsm file
 *  080213: add functions to read/ single slice/thumbnail from tiff/raw stacks
 */

#ifndef __STACKUTIL__
#define __STACKUTIL__

extern "C" {
#include "tiffio.h"
};

typedef char BIT8_UNIT;
typedef short int BIT16_UNIT;
typedef int BIT32_UNIT;
typedef long BIT64_UNIT;

int loadRaw2Stack(char * filename, unsigned char * & img, long * & sz, int & datatype); //4-byte raw reading
int saveStack2Raw(const char * filename, const unsigned char * img, const long * sz, int datatype); //4-byte raw writing

int loadRaw2Stack_2byte(char * filename, unsigned char * & img, long * & sz, int & datatype);
int saveStack2Raw_2byte(const char * filename, const unsigned char * img, const long * sz, int datatype);

void swap2bytes(void *targetp);
void swap4bytes(void *targetp);
char checkMachineEndian();

int loadTif2Stack(char * filename, unsigned char * & img, long * & sz, int & datatype);
int saveStack2Tif(const char * filename, const unsigned char * img, const long * sz, int datatype);

//the following two functions are the major routines to load LSM file
int loadLsm2Stack_obsolete(char * filename, unsigned char * & img, long * & sz, int & datatype); //070806
int loadLsm2Stack(char * filename, unsigned char * & img, long * & sz, int & datatype); //070806

//the following three functions are the low-level routines to load LSM file
int read_lsm_slice(TIFF *in, unsigned char * pointer_first_page, long pagepixelnumber, long channelpixelnumber, int datatype); 
int read_lsm_slice_strip(TIFF *in, unsigned char * pointer_first_page, long pagepixelnumber, long channelpixelnumber, int datatype);
int read_lsm_slice_tile(TIFF *in, unsigned char * pointer_first_page, long pagepixelnumber, long channelpixelnumber, int datatype);
int loadLsm2Stack_middle(char * filename, unsigned char * & img, long * & sz, int & datatype); //070819

//read LSM thumbnail. 070819
int loadLsmThumbnail2Stack(char * filename, unsigned char * & img, long * & sz, int & datatype);
int loadLsmThumbnail2Stack_middle(char * filename, unsigned char * & img, long * & sz, int & datatype);

//a more comprehensive way to read LSM file. 070823
int loadLsmSlice(char * filename, unsigned char * & img, long * & sz, int & datatype, long sliceno, bool b_thumbnail);

//more comprehensive interfaces to read tiff and raw files. 080213
int loadTifSlice(char * filename, unsigned char * & img, long * & sz, int & datatype, long sliceno, bool b_thumbnail);
int read_tif_slice(TIFF *tif, unsigned char * pointer_first_page, long width, long height);

int read_tif_slice_strip(TIFF *in, unsigned char * pointer_first_page, long pagepixelnumber, long channelpixelnumber, int datatype);
int read_tif_slice_tile(TIFF *in, unsigned char * pointer_first_page, long pagepixelnumber, long channelpixelnumber, int datatype);

int loadRawSlice(char * filename, unsigned char * & img, long * & sz, int & datatype, long sliceno, bool b_thumbnail);
int loadRawSlice_2byte(char * filename, unsigned char * & img, long * & sz, int & datatype, long sliceno, bool b_thumbnail);

//

char * getSuffix(char *filename); //note that no need to delete the returned pointer as it is actually a location to the "filename"

bool ensure_file_exists_and_size_not_too_big(char *filename, long sz_thres);

bool loadImage(char imgSrcFile[], unsigned char *& data1d, long * &sz, int & datatype); //070215
bool saveImage(const char filename[], const unsigned char * data1d, const long * sz, const int datatype); //070214

#endif



