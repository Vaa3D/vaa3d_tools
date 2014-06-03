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

#define TIFF3D_SUFFIX   "tif"


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
 * img:        pointer to slice (2D buffer)
 * img_width:  width of the slice
 * img_height: height of the slice
 */

char *readTiff3DFile2Buffer ( char *filename, unsigned char *img, unsigned int img_width, unsigned int img_height, unsigned int first, unsigned int last );
/* writes one slice to a file containing a 3D image
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

