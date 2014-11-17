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

#ifndef _IIM_VIRTUAL_FMT_MNGR_H
#define _IIM_VIRTUAL_FMT_MNGR_H

#include "IM_config.h"

#include "RawFmtMngr.h"
#include "Tiff3DMngr.h"

class iim::VirtualFmtMngr {

public:

    VirtualFmtMngr(){}
    virtual ~VirtualFmtMngr(){}

	virtual char *loadMetadata ( char * filename, iim::sint64 * &sz, int &datatype, int &b_swap, void * &fhandle, int &header_len ) = 0;
	/* opens the file filename in raw format containing a 4D image and returns in parameters:
	 *    sz:       a four component array containing image dimensions along horizontal (x), 
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

	virtual void closeFile ( void *fhandle ) = 0;
	/* closes the file associated to fhandle which is a pointer to e FILE structure */

	virtual char *load2SubStack ( void *fhandle, unsigned char *img, iim::sint64 *sz, 
							 iim::sint64 startx, iim::sint64 starty, iim::sint64 startz, 
							 iim::sint64 endx,iim::sint64 endy, iim::sint64 endz, 
							 int datatype, int b_swap, int header_len ) = 0; //4-byte raw reading
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

	virtual char *initFile ( char *filename, const iim::sint64 *sz, int datatype ) = 0;
	/* creates a file containing an empty 3D, multi-channel image 
	 *
	 * filename: complete path of the file to be initialized
	 * sz:       4-element array containing width, height, depth and the number of channels 
	 * datatype: pixel size in bytes
	 */

	virtual char *writeSlice2File ( char *filename, int slice, unsigned char *img, int img_height, int img_width ) = 0;
	/* writes one slice to a file containing a 3D image
	 * 
	 * filename:   complete path of the file to be modified
	 * img:        pointer to slice (2D buffer)
	 * img_height: height of the slice
	 * img_width:  width of the slice
	 */

	virtual char *copyFileBlock2Buffer ( char *filename, int sV0, int sV1, int sH0, int sH1, int sD0, int sD1,
									unsigned char *buf, int pxl_size, iim::sint64 offs, iim::sint64 stridex, iim::sint64 stridexy, iim::sint64 stridexyz ) = 0;
	/* copies a block in file 'filename' to a region of 3D buffer buf
	 *
	 * filename:      complete path of the file to be read
	 * sv0, ..., sD0: indices of the up, left, front pixel of the block stored in 'filename'
	 * sv1, ..., sD1: indices of the bottom, right, back pixel of the block stored in 'filename'
	 * buf:           pointer to buffer to be filled
	 * pxl_size:      pixel size in bytes
	 * offs:          offset on buf from which the copy has to start
	 * stridex:       number of pixels of the 3D buffer along x (H) dimension
	 * stridexy:      number of pixels in one slice of the 3D buffer (plane xy or VH)
	 * stridexyz:     number of pixels in one channel of the 3D buffer (volume xyz or VHD)
	 * 
	 * WARNING: current implementation assumes that datatype is 1-byte pixels and that 
	 * the endianess of the machine is the same as that of the machine that generated the
	 * data in 'filename'
	 */
	 
	 static void copyBlock2SubBuf ( unsigned char *src, unsigned char *dst, int dimi, int dimj, int dimk, int typesize,
	 								sint64 s_stridej, sint64 s_strideij, sint64 d_stridej, sint64 d_strideij );
	/* copies a block from buffer src to a sub-buffer of dst
	 *
	 * src:        pointer to the source buffer
	 * dst:        pointer to the destination buffer
	 * dimi:       dimension in pixels along V
	 * dimj:       dimension in pixels along H
	 * dimk:       dimension in pixels along D
	 * typesize:   number of bytes of one element
	 * s_stridej:  stride between rows in source buffer
	 * s_strideij: stride between slices in source buffer
	 * d_stridej:  stride between rows in destination buffer
	 * d_strideij: stride abetween slices in destination buffer
	 */

	 static void copyRGBBlock2Vaa3DRawSubBuf ( unsigned char *src, unsigned char *dst, int dimi, int dimj, int dimk, int typesize,
	 								sint64 s_stridej, sint64 s_strideij, sint64 d_stridej, sint64 d_strideij, sint64 d_strideijk );
	/* copies a block with 3 RGB channels from buffer src to a sub-buffer dst which stores channels in Vaa3DRaw format 
	 *
	 * src:         pointer to the source buffer
	 * dst:         pointer to the destination buffer
	 * dimi:        dimension in pixels along V
	 * dimj:        dimension in pixels along H
	 * dimk:        dimension in pixels along D
	 * typesize:    number of bytes of one element
	 * s_stridej:   stride between rows in source buffer
	 * s_strideij:  stride between slices in source buffer
	 * d_stridej:   stride between rows in destination buffer
	 * d_strideij:  stride between slices in destination buffer
	 * d_strideijk: stride between channels in destination buffer
	 */
};

class iim::Tiff3DFmtMngr : public iim::VirtualFmtMngr {

public:

    Tiff3DFmtMngr(){}

	char *loadMetadata ( char * filename, iim::sint64 * &sz, int &datatype, int &b_swap, void * &fhandle, int &header_len );

	void closeFile ( void *fhandle ) { closeTiff3DFile(fhandle); }

	char *load2SubStack ( void *fhandle, unsigned char *img, iim::sint64 *sz, 
							 iim::sint64 startx, iim::sint64 starty, iim::sint64 startz, 
							 iim::sint64 endx,iim::sint64 endy, iim::sint64 endz, 
							 int datatype, int b_swap, int header_len );

	char *initFile ( char *filename, const iim::sint64 *sz, int datatype );

	char *writeSlice2File ( char *filename, int slice, unsigned char *img, int img_height, int img_width );

	char *copyFileBlock2Buffer ( char *filename, int sV0, int sV1, int sH0, int sH1, int sD0, int sD1,
                                unsigned char *buf, int pxl_size, iim::sint64 offs, iim::sint64 stridex, iim::sint64 stridexy, iim::sint64 stridexyz );

};

class iim::Vaa3DRawFmtMngr : public iim::VirtualFmtMngr {

public:

    Vaa3DRawFmtMngr(){}

	char *loadMetadata ( char * filename, iim::sint64 * &sz, int &datatype, int &b_swap, void * &fhandle, int &header_len );

	void closeFile ( void *fhandle ) { closeRawFile(fhandle); }

	char *load2SubStack ( void *fhandle, unsigned char *img, iim::sint64 *sz, 
							 iim::sint64 startx, iim::sint64 starty, iim::sint64 startz, 
							 iim::sint64 endx,iim::sint64 endy, iim::sint64 endz, 
							 int datatype, int b_swap, int header_len );

	char *initFile ( char *filename, const iim::sint64 *sz, int datatype );

	char *writeSlice2File ( char *filename, int slice, unsigned char *img, int img_height, int img_width );

	char *copyFileBlock2Buffer ( char *filename, int sV0, int sV1, int sH0, int sH1, int sD0, int sD1,
                                unsigned char *buf, int pxl_size, iim::sint64 offs, iim::sint64 stridex, iim::sint64 stridexy, iim::sint64 stridexyz );

};

#endif
