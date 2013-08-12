/*
 * This file is a modified version of code extracted from the files stackutil.cpp and
 * stackutil-11.cpp of the V3D project. See the following licence notice for more details.
 *
 * modified by Giulio Iannello, Centro Integrsto di Ricerca, Universita' Campus Bio-Medico di Roma
 * December 2012
 */



/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




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
 * 081204: add overload functions to read only a channel of a stack
 * 090413: add MRC rec file reading and writing
 * 090802: add .raw5d read and write interface
 * 100519: add v3d_basicdatatype.h
 * 100817: add mylib interface, PHC
 */

#ifndef RAWFMTMNGR_H
#define RAWFMTMNGR_H

//the folowing conditional compilation is added by PHC, 2010-05-20
//#if defined (_MSC_VER)
//#include "../basic_c_fun/vcdiff.h"
//#else
//#endif
//
//extern "C" {
//#include "tiffio.h"
//};

//#include "v3d_basicdatatype.h"

#if defined(_MSC_VER) && (_WIN64)
//#if defined(_MSC_VER) && defined(_WIN64) //correct?

#define V3DLONG long long

#else

#define V3DLONG long

#endif


typedef char BIT8_UNIT;
typedef short int BIT16_UNIT;
typedef int BIT32_UNIT;
typedef V3DLONG BIT64_UNIT;

char *loadRaw2Metadata ( char * filename, V3DLONG * &sz, int &datatype, int &b_swap, void * &fhandle, int &header_len );
/* opens the file filename in raw format containing a 4D image and returns in parameters:
 *    sz:       a four component array containing image dimensions along horizontal (x), 
 *              vertical (y), depth (z) directions, and the number of channels
 *    datatype: the number of bytes per pixel
 *    b_swap:   a 0/1 value that indicates if endianness of the file is the same (0) or 
 *              is different (1) from the one of the current machine
 *    fhandle:  a pointer to a FILE structure associated to the file which is left opened
 *
 * if some exception occurs, returns a string describing the exception; returns a NULL pointer
 * if there are no exceptions
 */

void closeRawFile ( void *fhandle );
/* closes the file associated to fhandle which is a pointer to e FILE structure */

char *loadRaw2SubStack ( void *fhandle, unsigned char *img, V3DLONG *sz, 
						 V3DLONG startx, V3DLONG starty, V3DLONG startz, 
						 V3DLONG endx,V3DLONG endy, V3DLONG endz, 
						 int datatype, int b_swap, int header_len ); //4-byte raw reading
/* read a substack of the 4D image stored in raw format in the file associated to fhandle
 * input parameters:
 *    image:      a pointer to an empty buffer large enough to contain the substack
 *    sz:         a four component array containing image dimensions along horizontal (x),
 *                vertical (y), depth (z) directions, and the number of channels
 *    startx:     index along x of the pixel which is the first vertex of the substack 
 *    starty:     index along y of the pixel which is the first vertex of the substack
 *    startz:     index along z of the pixel which is the first vertex of the substack
 *    endx:       index along x of the pixel which is the last vertex of the substack
 *    endy:       index along y of the pixel which is the last vertex of the substack
 *    endz:       index along z of the pixel which is the last vertex of the substack
 *    datatype:   the number of bytes per pixel
 *    b_swap:     a 0/1 value that indicates if endianness of the file is the same (0) or 
 *                is different (1) from the one of the current machine
 *    header_len: the length of the header in bytes
 *
 * if some exception occurs, returns a string describing the exception; returns a NULL pointer
 * if there are no exceptions
 */


char *loadRaw2WholeStack(char * filename, unsigned char * & img, V3DLONG * & sz, int & datatype); //4-byte raw reading
/* opens and reads the file filename in raw format containing a 4D image and returns 
 * in parameters:
 *    img:      a pointer to a newly allocated buffere where the wole omage is stored
 *              one channel after another
 *    sz:       a four component array containing image dimensions along horizontal (x), 
 *              vertical (y), depth (z) directions, and the number of channels
 *    datatype: the number of bytes per pixel
 *
 * if some exception occurs, returns a string describing the exception; returns a NULL pointer
 * if there are no exceptions
 */

char *initRawFile ( char *filename, const V3DLONG *sz, int datatype );

char *writeSlice2RawFile ( char *filename, int slice, unsigned char *img, int img_height, int img_width );

char *copyRawFileBlock2Buffer ( char *filename, int sV0, int sV1, int sH0, int sH1, int sD0, int sD1,
							    unsigned char *buf, int pxl_size, int offs, int stridex, int stridexy, int stridexyz );

#endif



