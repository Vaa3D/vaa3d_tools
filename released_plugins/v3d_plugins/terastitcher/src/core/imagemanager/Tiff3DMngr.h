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
* 2015-02-15. Giulio. @CHANGED revised all interfaces passing always width and height in this order
* 2015-02-06. Giulio. @ADDED append operation that assumes an already open and positioned file
* 2015-02-06. Giulio. @ADDED open operation
*/

#ifndef TIFF3D_MNGR_H
#define TIFF3D_MNGR_H

#define TIFF3D_SUFFIX   "tif"


char *loadTiff3D2Metadata ( char * filename, unsigned int &sz0, unsigned int  &sz1, unsigned int  &sz2, unsigned int  &sz3, int &datatype, int &b_swap, void * &fhandle, int &header_len );
/* opens the file filename in raw format containing a 4D image and returns in parameters:
 *    szX:      a four values representing image dimensions along horizontal (x), 
 *              vertical (y), depth (z) directions, and the number of channels
 *    datatype: the number of bytes per pixel
 *    b_swap:   a 0/1 value that indicates if endianness of the file is the same (0) or 
 *              is different (1) from the one of the current machine
 *    fhandle:  a pointer to a FILE structure associated to the file which is left opened
 *
 * the file is not closed
 *
 * if some exception occurs, returns a string describing the exception; returns a NULL pointer
 * if there are no exceptions
 */

char *openTiff3DFile ( char *filename, char *mode, void *&fhandle );
/* opens the file 'filename' in mode 'mode' and returns a fhandle which is a pointer to an opaque structure */

void closeTiff3DFile ( void *fhandle );
/* closes the file associated to fhandle which is a pointer to a FILE structure */

char *initTiff3DFile ( char *filename, unsigned int sz0, unsigned int  sz1, unsigned int  sz2, unsigned int  sz3, int datatype );
/* creates a file containing an empty 3D, multi-channel image 
 *
 * filename: complete path of the file to be initialized
 * sz:       4-element array containing width, height, depth and the number of channels 
 * datatype: pixel size in bytes
 */

char *appendSlice2Tiff3DFile ( char *filename, int slice, unsigned char *img, unsigned int  img_width, unsigned int  img_height );
/* writes one slice to a file containing a 3D image
 * 
 * filename:   complete path of the file to be modified
 * slice:      slice index to be appended
 * img:        pointer to slice (2D buffer)
 * img_width:  width of the slice
 * img_height: height of the slice
 */

char *appendSlice2Tiff3DFile ( void *fhandler, int slice, unsigned char *img, unsigned int  img_width, unsigned int  img_height, int spp, int bpp, int NPages );
/* writes one slice to a file containing a 3D image
 * 
 * fhandler:   handler of the file to be modified
 * slice:      slice index to be appended
 * img:        pointer to the slice (2D buffer of (img_width * img_height * spp * (bpp/8)) bytes)
 * img_width:  width of the slice
 * img_height: height of the slice
 * spp:        samples per pixel (channels)
 * bpp:        bits per pixel (pixel depth)
 * NPages:     total number of pages of the file (when all pages have been appended)
 *
 * WARNING: the file is already open and it is not closed after data have been read
 */

char *readTiff3DFile2Buffer ( char *filename, unsigned char *img, unsigned int img_width, unsigned int img_height, unsigned int first, unsigned int last );
/* reads a substack from a file containing a 3D image
 * 
 * filename:   complete path of the file to be modified
 * img:        pointer to slice (3D buffer)
 * img_width:  width of the slice
 * img_height: height of the slice
 * first:      index of first slice
 * last:       index of last slice
 *
 * PRE: img points to a buffer of img_height * img_width * (last-first+1) * bps * spp
 * where bps and spp are the bit-per-sample and sample-per-pixel tags of the multipage tiff file
 */

char *readTiff3DFile2Buffer ( void *fhandler, unsigned char *img, unsigned int img_width, unsigned int img_height, unsigned int first, unsigned int last, int b_swap );
/* reads a substack from a file containing a 3D image
 * 
 * fhandler:   handler of the file to be modified
 * img:        pointer to slice (3D buffer)
 * img_width:  width of the slice
 * img_height: height of the slice
 * first:      index of first slice
 * last:       index of last slice
 * b_swap:     a 0/1 value that indicates if endianness of the file is the same (0) or 
 *             is different (1) from the one of the current machine
 *
 * WARNING: the file is already open and it is not closed after data have been read
 *
 * WARNING: the value of b_swap determines if swapping has to be performed or not before returning the buffer
 * filled with data; it is responsibility of the caller to set this parameter correctly
 *
 * PRE: img points to a buffer of img_height * img_width * (last-first+1) * bps * spp
 * where bps and spp are the bit-per-sample and sample-per-pixel tags of the multipage tiff file
 */

 #endif
 