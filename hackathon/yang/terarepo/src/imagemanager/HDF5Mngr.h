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
* 2015-12-29. Giulio. @ADDED red_factor parameter to 'BDV_HDF5getSubVolume'
* 2015-11-17. Giulio. @CREATED 
*/

#ifndef HDF5_MNGR_H
#define HDF5_MNGR_H

#include "IM_config.h"
#include <string>

/* Hyperslab conventions
 *
 * for N-dimensional datasets are 4 x N matrices
 * first row is START (offset)
 * second row is STRIDE
 * third row is COUNT (how many elements)
 * fourth row is BLOCK (how many blocks)
 *
 * For N=3, for each row:
 * first element is D dimension
 * second element is V dimension
 * third element is H dimension
 */


void BDV_HDF5init ( std::string fname, void *&descr, int vxl_nbytes = 1 ) throw (iim::IOException);
/* opens or creates an HDF5 file fname according to the BigDataViewer format and returns an opaque descriptor
 *
 * fname:      HDF5 filename
 * vxl_nbytes: number of bytes per voxel of datasets to be stored in the file; it is ignored if the file already exists
 * descr:      pointer to the returned opaque descriptor
 */
 
 int BDV_HDF5n_resolutions ( void *descr );
 /* returns how many resolutions there are in the HDF5 file handled by descr
  * It is assumed that all resolutions from 0 to the interger returned minus 1 are available
  */

void BDV_HDF5close ( void *descr );
/* close the HDF5 file represented by descr and deallocates the descriptor
 *
 * descr: opaque descriptor of the HDF5 file
 */

void BDV_HDF5addSetups ( void *file_descr, iim::sint64 height, iim::sint64 width, iim::sint64 depth, 
				 float vxlszV, float vxlszH, float vxlszD, bool *res, int res_size, int chans, int block_height = -1, int block_width = -1, int block_depth = -1 ); 
/* creates setup descriptors with resolution and subdivisions datasets
 *
 * file_descr:
 * height:
 * width:
 * depth:
 * vxlszV:
 * vxlszH*
 * vxlszD:
 * res:
 * res_size:
 * chans: 
 * block_height:
 * block_width:
 * block_depth:  
 */


void BDV_HDF5addTimepoint ( void *file_descr, int tp = 0 ); 
/* add time point at time tp (first time point is the default)
 */


void BDV_HDF5writeHyperslab ( void *file_descr, iim::uint8 *buf, iim::sint64 *dims, iim::sint64 *hl, int r, int s, int tp = 0 );
/* write one hiperslab to file  
 */


void BDV_HDF5getVolumeInfo ( void *descr, int tp, int res, void *&volume_descr, 
								float &VXL_1, float &VXL_2, float &VXL_3, 
								float &ORG_V, float &ORG_H, float &ORG_D, 
								iim::uint32 &DIM_V, iim::uint32 &DIM_H, iim::uint32 &DIM_D,
							    int &DIM_C, int &BYTESxCHAN, int &DIM_T, int &t0, int &t1 );
/* Open a volume (i.e. a resolution) and return corresponding metadata 
 */


void BDV_HDF5getSubVolume ( void *descr, int V0, int V1, int H0, int H1, int D0, int D1, int setup, iim::uint8 *buf, int red_factor = 1 );
/* must copy a subvolume into buffer buf; voxels have to be converted to 8-bit if needed
 */


void BDV_HDF5closeVolume ( void *descr );


 #endif
 