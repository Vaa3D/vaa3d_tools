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
* 2017-04-20. Giulio  @CHANGED interface of 'IMS_HDF5init' to improve structure initialization
* 2017-04-20. Giulio. @ADDED an operation to adjust the object list
* 2017-04-17. Giulio. @CHENGED 'IMS_HDF5init' interface to extract file structure only upon request
* 2017-04-08. Giulio. @ADDED support for additional attributes required by the IMS format
* 2015-12-29. Giulio. @ADDED red_factor parameter to 'IMS_HDF5getSubVolume'
* 2015-11-17. Giulio. @CREATED 
*/

#ifndef IMS_HDF5_MNGR_H
#define IMS_HDF5_MNGR_H

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


// structure representing histograms
struct histogram_t {
	int hmin;
	int hmax;
	int hlen;
	iim::uint64 *hist;

	histogram_t ( int _hmin, int _hmax, int _hlen, iim::uint64 *_hist ) {
		hmin = _hmin;
		hmax = _hmax;
		hlen = _hlen;
		hist = _hist;
	}

	histogram_t(){
		hmin = hmax = hlen = 0;
		hist = (iim::uint64 *) 0;
	}

	// this copy constructor must used to pass by value parameters created on the fly that will be immediately released
	histogram_t ( const histogram_t &ex ) {
		hmin = ex.hmin;
		hmax = ex.hmax;
		hlen = ex.hlen;
		hist = new iim::uint64[hlen];
		memcpy(hist,ex.hist,hlen*sizeof(iim::uint64));
	}

	~histogram_t ( ) {
		if ( hist )
			delete hist;
	}
};



void IMS_HDF5init ( std::string fname, void *&descr, bool loadstruct = false, int vxl_nbytes = 1, void *obj_info = (void *)0, void *root_attr_info = (void *)0  ) throw (iim::IOException);
/* opens or creates an HDF5 file fname according to the BigDataViewer format and returns an opaque descriptor
 *
 * fname:      HDF5 filename
 * vxl_nbytes: number of bytes per voxel of datasets to be stored in the file; it is ignored if the file already exists
 * descr:      pointer to the returned opaque descriptor
 * obj_info:   to be used only if the file does not exist yet and it must be created; is the description of the structure of the 
 *             file to be created; the description is deallocated after it is used and cannot be used by the caller
 */
 
void *IMS_HDF5get_olist ( void *descr, std::string fname = "", int height = 0, int width = 0, int z_points = 0, int n_chans = 1, int n_timepoints = 1, float abs_V = 0.0, float abs_H = 0.0 ) throw (iim::IOException);
/* return a structure describing a hierarchical file structure in terms of groups and attributes
 * if 'descr' is not null, the other parameters are not used and the returned structure is the one of the file which descriptor is 'descr'
 * if 'descr' is null a default strucutre is built using the other parameters to set some attributes
 */

void *IMS_HDF5adjust_olist ( void *olist, std::string fname, int height, int width, int z_points, iim::uint32 *chans, int n_chans, float abs_V = 0.0, float abs_H = 0.0 ) throw (iim::IOException);
/* adjust the structure 'olist' which describes a hierarchical file structure in terms of groups and attributes
 * the other parameters are used to change some groups and attributes
 * the adjusted file structure is returned
 */

void *IMS_HDF5get_rootalist ( void *descr ) throw (iim::IOException);
/*
 */

 int IMS_HDF5n_resolutions ( void *descr ) throw (iim::IOException);
 /* returns how many resolutions there are in the HDF5 file handled by descr
  * It is assumed that all resolutions from 0 to the interger returned minus 1 are available
  */

 void IMS_HDF5setVxlSize ( void *descr, double szV, double szH, double szD ) throw (iim::IOException);
 /* set voxel size at resolution 0 */

 void IMS_HDF5getVxlSize ( void *descr, double &szV, double &szH, double &szD ) throw (iim::IOException);
 /* return voxel size at resolution 0 */

void IMS_HDF5close ( void *descr ) throw (iim::IOException);
/* close the HDF5 file represented by descr and deallocates the descriptor
 *
 * descr: opaque descriptor of the HDF5 file
 */

void IMS_HDF5set_histogram ( void *descr, histogram_t *buf, int r = 0, int ch = 0, int tp = 0 ) throw (iim::IOException);
/* set the histogram of channel ch at time point tp at resolution r to be saved when the file is closed 
 * (has effect only if the file is being created) 
 */

void IMS_HDF5set_thumbnail ( void *descr, iim::uint8 *buf, iim::uint32 thumbnail_sz ) throw (iim::IOException);
/* set the thumbnail to be saved when the file is closed (has effect only if the file is being created) */

void IMS_HDF5addResolution ( void *file_descr, iim::sint64 height, iim::sint64 width, iim::sint64 depth, int nchans, int r = 0, bool is_first = false ) throw (iim::IOException); 
/* add resolution r
 *
 * height: image height at resolution 0
 * width:  image width at resolution 0
 * depth:  image depth at resolution 0
 *
 * WARNING: parameters height, width and depth must refer to dimensions of the highest resolution that can be stired in the file that is resolution 0
 * These dimensions must be coherent with the voxel size computed for this resolution 0
 */


void IMS_HDF5addChans ( void *file_descr, iim::sint64 height, iim::sint64 width, iim::sint64 depth, 
				 float vxlszV, float vxlszH, float vxlszD, bool *res, int res_size, int chans, int block_height = -1, int block_width = -1, int block_depth = -1 ) throw (iim::IOException); 
/* creates chan descriptors with resolution and subdivisions datasets
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


void IMS_HDF5addTimepoint ( void *file_descr, int tp = 0, std::string params = "" ) throw (iim::IOException); 
/* add time point at time tp (first time point is the default)
 */


void IMS_HDF5writeHyperslab ( void *file_descr, iim::uint8 *buf, iim::sint64 *dims, iim::sint64 *hl, int r, int s, int tp = 0 ) throw (iim::IOException);
/* write one hyperslab to file  
 */


void IMS_HDF5getVolumeInfo ( void *descr, int tp, int res, void *&volume_descr, 
								float &VXL_1, float &VXL_2, float &VXL_3, 
								float &ORG_V, float &ORG_H, float &ORG_D, 
								iim::uint32 &DIM_V, iim::uint32 &DIM_H, iim::uint32 &DIM_D,
							    int &DIM_C, int &BYTESxCHAN, int &DIM_T, int &t0, int &t1 ) throw (iim::IOException);
/* Open a volume (i.e. a resolution) and return the opaque descriptor of the volume and the corresponding metadata 
 */


void IMS_HDF5getSubVolume ( void *descr, int V0, int V1, int H0, int H1, int D0, int D1, int chan, iim::uint8 *buf, int red_factor = 1 ) throw (iim::IOException);
/* must copy a subvolume into buffer buf; voxels have to be converted to 8-bit if needed
 */


void IMS_HDF5closeVolume ( void *descr ) throw (iim::IOException);


 #endif
 