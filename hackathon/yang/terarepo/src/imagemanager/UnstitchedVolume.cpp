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
* 2017-08-19. Giulio.     @CHANGED only requested data (actually only whole rows) are copied into the internal buffer if buffering is not enabled 
* 2017-08-19. Giulio.     @CHANGED the method reading only the actually requested data is invoked only if buffering is not enabled 
* 2017-07-15. Giulio.     @CHANGED the way the returned stitched buffer is copied to the final buffer in order to cope with the case the returned buffer is smaller than the requeste one
* 2017-07-06. Giulio.     @CHANGED call to 'getStripe2' in 'internal_loadSubvolume_to_real32' to enable selective read of data
* 2017-04-12. Giulio.     @ADDED release of allocated buffers if an exception is raised in 'getStripe' (prevent further exceptions in the GUI version)
* 2017-04-01. Giulio.     @ADDED support for multi-layer stitching
* 2016-09-13. Giulio.     @ADDED a cache manager to store stitched subregions
* 2016-08-30. Giulio.     @FIXED bug in 'internal_loadSubvolume_to_real32': for each row/column both tests have to be performed to check if border tiles have to be added on both sides 
* 2016-08-30. Giulio.     @FIXED bug in 'internal_loadSubvolume_to_real32': vxl_i was not correctly initialized and it was incorrectly compared with V1/H1
* 2016-08-20. Giulio.     @FIXED bug in 'loadSubvolume_to_real32': the buffer allocated was too big (BYTESxCHAN and DIM_C should not be considered)
* 2016-08-20. Giulio.     @FIXED bug in 'loadSubvolume_to_UINT3' about the management of active channels
* 2016-06-19. Giulio.     @FIXED bug in the call to input plugin (introduced the information on the plugin type: 2D/3D)
* 2016-06-10. Giulio.     @ADDED the code to load only active channels
* 2016-06-10. Giulio.     @ADDED the code to manage the case when channels are more than three
* 2016-05-03. Giulio.     @FIXED 'internal_loadSubvolume_to_real32' checks to guarantee that merging is performed also at the border of the subvolume
* 2016-03-23. Giulio.     @FIXED 'internal_loadSubvolume_to_real32' bug on the idetification of the tiles involved in the subvolume extraction 
* 2015-12-28. Giulio.     @ADDED 'internal_loadSubvolume_to_real32' now check the ret_type parameter to determine voxel depth to be returned 
* 2015-12-28. Giulio.     @FIXED 'internal_loadSubvolume_to_real32' did not check the ret_type parameter to determine voxel depth to be returned 
* 2015-03-13. Giulio.     @FIXED a bug in 'internal_loadSubvolume_to_real32': MEC must be divided by VXL (and not multiplied)
* 2015-03-13. Giulio.     @FIXED a bug in 'internal_loadSubvolume_to_real32': getWIDTH -> getHEIGHT in computing tiles' row indices
* 2015-02-28. Giulio.     @ADDED management of multi-channel, multi-bytes per channel images: currently supported up to three channels 8 or 16 bits per channel
* 2015-02-27. Alessandro. @ADDED automated selection of IO plugin if not provided.
* 2015-02-18. Giulio.     @CREATED  
*/

#include <iostream>
#include <string>
#include "UnstitchedVolume.h"
#include "vmBlockVolume.h"
#include "vmStackedVolume.h"

#include "IOPluginAPI.h"
#include "iomanager.config.h"

#ifdef _WIN32
#include "dirent_win.h"
#else
#include <dirent.h>
#endif

#include <list>
#include <fstream>
#include "ProgressBar.h"

using namespace std;
using namespace iim;

/* Indices on unstitched volume refer to absolute position of voxels in the matix obtained placing tiles at their aligned position with respect 
 * to up-left-top voxel of the up-left tile which is conventionally assgned the indices (0,0,0)
 * According to the strategy of image reconstruction, the minimun value for indices V and H is <= 0, whereas the minimum index D is >= 0.
 * These minimum values are stored in the V0, H0, D0 data members of the stitcher class after the method 'computeVolumeDims' has been run of 
 * all tiles
 * Indices on the stitched volume (i.e. the volume visible through the VirtualVolume interface) always start from 0
 *
 * Indices on unsitiched and stiched volume must therefore mapped each other:
 * - to map indices from stitched to unsitiched the values of V0, H0, D0 must be added, whereas to map unstitched to stitched those values 
 * must be subtracted
 */


UnstitchedVolume::UnstitchedVolume(void) : VirtualVolume() 
{
	volume = (volumemanager::VirtualVolume *) 0;
	stitcher = (StackStitcher *) 0;

    reference_system.first = reference_system.second = reference_system.third = iim::axis_invalid;

	stripesCoords = (stripe_2Dcoords  *) 0;
	stripesCorners = (stripe_2Dcorners *) 0;

	cb = (iim::CacheBuffer *) 0;
}


UnstitchedVolume::UnstitchedVolume(const char* _root_dir, bool cacheEnabled, int _blending_algo)  throw (IOException)
: VirtualVolume(_root_dir), volume_external(false)
{
    /**/iim::debug(iim::LEV3, strprintf("_root_dir=%s", _root_dir).c_str(), __iim__current__function__);

	// 2014-09-29. Alessandro. @ADDED automated selection of IO plugin if not provided.
	if(iom::IMIN_PLUGIN.compare("empty") == 0)
	{
		std::string volformat = vm::VirtualVolume::getVolumeFormat(_root_dir);

		if(volformat.compare(vm::StackedVolume::id) == 0)
			iom::IMIN_PLUGIN = "tiff2D";
		else if(volformat.compare(vm::BlockVolume::id) == 0)
			iom::IMIN_PLUGIN = "tiff3D";
	}

	bool flag = false;
	try{
		plugin_type = "3D image-based I/O plugin";
		flag = iom::IOPluginFactory::getPlugin3D(iom::IMIN_PLUGIN)->desc().find(plugin_type) != std::string::npos;
	}
	catch (...) {
		plugin_type = "2D image-based I/O plugin";
		flag = iom::IOPluginFactory::getPlugin2D(iom::IMIN_PLUGIN)->desc().find(plugin_type) != std::string::npos;
	}
	if ( !flag )
 		throw iim::IOException(iom::strprintf("cannot determine the type of the input plugin"), __iom__current__function__);

    // @FIXED by Alessandro on 2016-12-15. Convert iom::exception to iim::IOException (the only ones this function can throw)
    try
    {
        volume = volumemanager::VirtualVolumeFactory::createFromXML(_root_dir,false);
    }
    catch(iom::exception & ex)
    {
        throw iim::IOException(ex.what());
    }

	stitcher = new StackStitcher(volume);

	reference_system.first  = (iim::axis(volume->getREF_SYS().first));
	reference_system.second = (iim::axis(volume->getREF_SYS().second));
	reference_system.third  = (iim::axis(volume->getREF_SYS().third));

	VXL_V = volume->getVXL_V();
	VXL_H = volume->getVXL_H();
	VXL_D = volume->getVXL_D();

	// these must be corrected after the real size od the stitched volume have been computed
	ORG_V = volume->getORG_V();
	ORG_H = volume->getORG_H();
	ORG_D = volume->getORG_D();

	DIM_C = volume->getDIM_C();
	BYTESxCHAN = volume->getBYTESxCHAN();

    // @FIXED by Alessandro on 2016-12-15. Convert iom::exception to iim::IOException (the only ones this function can throw)
    try
    {
        if ( (plugin_type.compare("3D image-based I/O plugin") == 0) ?
              iom::IOPluginFactory::getPlugin3D(iom::IMIN_PLUGIN)->isChansInterleaved() :
              iom::IOPluginFactory::getPlugin2D(iom::IMIN_PLUGIN)->isChansInterleaved() ) {
            if ( DIM_C > 3 || BYTESxCHAN > 2 )
                throw iim::IOException(iom::strprintf("image not supported by UnstitchedVolume (DIM_C=%d, BYTESxCHAN=%d)", DIM_C, BYTESxCHAN), __iom__current__function__);
        }
        else { // if channels are not interleaved more than 3 channels are possible
            if ( BYTESxCHAN > 2 )
                throw iim::IOException(iom::strprintf("image not supported by UnstitchedVolume (BYTESxCHAN=%d)", BYTESxCHAN), __iom__current__function__);
        }
    }
    catch(iom::exception & ex)
    {
        throw iim::IOException(ex.what());
    }

    active = (iim::uint32 *) new iim::uint32[DIM_C];
    n_active = DIM_C;
	for ( int i=0; i<DIM_C; i++ )
		active[i] = i;

	current_channel = -1;
	internal_buffer_deallocate = true;

	t0 = t1 = 0;
	DIM_T = 1;

 	stitcher->computeVolumeDims(false); // set index limits of unstitched volume

	// 2016-03-23. Giulio.     @ADDED offsets of unstitched volume with respect to nominal origin (0,0,0) 
	V0_offs = stitcher->V0; // may be negative and must be subtracted to map indices of unstitched volume (that may start from a negative value) to indices of stitched volume (starting from 0)
	H0_offs = stitcher->H0; // may be negative and must be subtracted to map indices of unstitched volume (that may start from a negative value) to indices of stitched volume (starting from 0)
	D0_offs = stitcher->D0; // may be positive and must be subtracted to map indices of unstitched volume (that may start from a positive value) to indices of stitched volume (starting from 0)

	// correct origin coordinates of the stitched volume
	ORG_V += V0_offs * VXL_V;
	ORG_H += H0_offs * VXL_H;
	ORG_D += D0_offs * VXL_D;

	blending_algo = _blending_algo;

	DIM_V = stitcher->V1 - stitcher->V0;
	DIM_H = stitcher->H1 - stitcher->H0;
	DIM_D = stitcher->D1 - stitcher->D0;

	stripesCoords = new stripe_2Dcoords[volume->getN_ROWS()];
	stripesCorners = new stripe_2Dcorners[volume->getN_ROWS()];

	cb = new iim::CacheBuffer(this);
}

// constructor 2 @ADDED by Alessandro on 2014-04-18: takes vm::VirtualVolume in input
UnstitchedVolume::UnstitchedVolume(vm::VirtualVolume * _imported_volume, bool cacheEnabled, int _blending_algo)  throw (iim::IOException)
	: VirtualVolume(_imported_volume->getSTACKS_DIR()), volume(_imported_volume), volume_external(true) 
{
	/**/iim::debug(iim::LEV3, strprintf("_root_dir=%s", volume->getSTACKS_DIR()).c_str(), __iim__current__function__);

	// 2014-09-29. Alessandro. @ADDED automated selection of IO plugin if not provided.
	if(iom::IMIN_PLUGIN.compare("empty") == 0)
	{
		if(dynamic_cast<vm::StackedVolume*>(volume))
			iom::IMIN_PLUGIN = "tiff2D";
		else if(dynamic_cast<vm::BlockVolume*>(volume))
			iom::IMIN_PLUGIN = "tiff3D";
	}

	bool flag = false;
	try{
		plugin_type = "3D image-based I/O plugin";
		flag = iom::IOPluginFactory::getPlugin3D(iom::IMIN_PLUGIN)->desc().find(plugin_type) != std::string::npos;
	}
	catch (...) {
		plugin_type = "2D image-based I/O plugin";
		flag = iom::IOPluginFactory::getPlugin2D(iom::IMIN_PLUGIN)->desc().find(plugin_type) != std::string::npos;
	}
	if ( !flag )
		throw iim::IOException(iom::strprintf("cannot determine the type of the input plugin"), __iom__current__function__);

	

	stitcher = new StackStitcher(volume);

	reference_system.first  = (iim::axis(volume->getREF_SYS().first));
	reference_system.second = (iim::axis(volume->getREF_SYS().second));
	reference_system.third  = (iim::axis(volume->getREF_SYS().third));

	VXL_V = volume->getVXL_V();
	VXL_H = volume->getVXL_H();
	VXL_D = volume->getVXL_D();

	// these must be corrected after the real size od the stitched volume have been computed
	ORG_V = volume->getORG_V();
	ORG_H = volume->getORG_H();
	ORG_D = volume->getORG_D();

	DIM_C = volume->getDIM_C();
	BYTESxCHAN = volume->getBYTESxCHAN();

	// @FIXED by Alessandro on 2016-12-15. Convert iom::exception to iim::IOException (the only ones this function can throw)
	try
	{
		if ( (plugin_type.compare("3D image-based I/O plugin") == 0) ?
			iom::IOPluginFactory::getPlugin3D(iom::IMIN_PLUGIN)->isChansInterleaved() :
			iom::IOPluginFactory::getPlugin2D(iom::IMIN_PLUGIN)->isChansInterleaved() ) {
				if ( DIM_C > 3 || BYTESxCHAN > 2 )
					throw iim::IOException(iom::strprintf("image not supported by UnstitchedVolume (DIM_C=%d, BYTESxCHAN=%d)", DIM_C, BYTESxCHAN), __iom__current__function__);
		}
		else { // if channels are not interleaved more than 3 channels are possible
			if ( BYTESxCHAN > 2 )
				throw iim::IOException(iom::strprintf("image not supported by UnstitchedVolume (BYTESxCHAN=%d)", BYTESxCHAN), __iom__current__function__);
		}
	}
	catch(iom::exception & ex)
	{
		throw iim::IOException(ex.what());
	}

	active = (iim::uint32 *) new iim::uint32[DIM_C];
	n_active = DIM_C;
	for ( int i=0; i<DIM_C; i++ )
		active[i] = i;

	current_channel = -1;
	internal_buffer_deallocate = true;

	t0 = t1 = 0;
	DIM_T = 1;

	stitcher->computeVolumeDims(false); // set index limits of unstitched volume

	// 2016-03-23. Giulio.     @ADDED offsets of unstitched volume with respect to nominal origin (0,0,0) 
	V0_offs = stitcher->V0; // may be negative and must be subtracted to map indices of unstitched volume (that may start from a negative value) to indices of stitched volume (starting from 0)
	H0_offs = stitcher->H0; // may be negative and must be subtracted to map indices of unstitched volume (that may start from a negative value) to indices of stitched volume (starting from 0)
	D0_offs = stitcher->D0; // may be positive and must be subtracted to map indices of unstitched volume (that may start from a positive value) to indices of stitched volume (starting from 0)

	// correct origin coordinates of the stitched volume
	ORG_V += V0_offs * VXL_V;
	ORG_H += H0_offs * VXL_H;
	ORG_D += D0_offs * VXL_D;

	blending_algo = _blending_algo;

	DIM_V = stitcher->V1 - stitcher->V0;
	DIM_H = stitcher->H1 - stitcher->H0;
	DIM_D = stitcher->D1 - stitcher->D0;

	stripesCoords = new stripe_2Dcoords[volume->getN_ROWS()];
	stripesCorners = new stripe_2Dcorners[volume->getN_ROWS()];

	cb = new iim::CacheBuffer(this);
}

UnstitchedVolume::~UnstitchedVolume(void) throw (iim::IOException)
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);
	if ( stripesCorners ) {
		for ( int i=0; i<volume->getN_ROWS(); i++ ) {
			stripesCorners[i].ups.clear();
			stripesCorners[i].bottoms.clear();
			stripesCorners[i].merged.clear();
		}
		delete []stripesCorners;
	}
	if ( stripesCoords )
		delete stripesCoords;
	if ( volume && !volume_external) // stitcher does not deallocate its volume
		delete volume;
	if ( stitcher )
		delete stitcher;
	if ( cb )
		delete cb;
}


void UnstitchedVolume::init()
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);
}

void UnstitchedVolume::initChannels ( ) throw (IOException)
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);
}


void UnstitchedVolume::updateTilesPositions ( ) 
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

 	stitcher->computeVolumeDims(false); // set index limits of unstitched volume

	// offsets of unstitched volume with respect to nominal origin (0,0,0) 
	V0_offs = stitcher->V0; // may be negative and must be subtracted to map indices of unstitched volume (that may start from a negative value) to indices of stitched volume (starting from 0)
	H0_offs = stitcher->H0; // may be negative and must be subtracted to map indices of unstitched volume (that may start from a negative value) to indices of stitched volume (starting from 0)
	D0_offs = stitcher->D0; // may be positive and must be subtracted to map indices of unstitched volume (that may start from a positive value) to indices of stitched volume (starting from 0)

	// correct origin coordinates of the stitched volume
	ORG_V += V0_offs * VXL_V;
	ORG_H += H0_offs * VXL_H;
	ORG_D += D0_offs * VXL_D;

	DIM_V = stitcher->V1 - stitcher->V0;
	DIM_H = stitcher->H1 - stitcher->H0;
	DIM_D = stitcher->D1 - stitcher->D0;
}


//PRINT method
void UnstitchedVolume::print()
{
}

//rotate stacks matrix around D axis (accepted values are theta=0,90,180,270)
void UnstitchedVolume::rotate(int theta)
{
    /**/iim::debug(iim::LEV3, strprintf("theta=%d", theta).c_str(), __iim__current__function__);
}

//mirror stacks matrix along mrr_axis (accepted values are mrr_axis=1,2,3)
void UnstitchedVolume::mirror(axis mrr_axis)
{
    /**/iim::debug(iim::LEV3, strprintf("mrr_axis=%d", mrr_axis).c_str(), __iim__current__function__);
}

//extract spatial coordinates (in millimeters) of given Stack object
void UnstitchedVolume::extractCoordinates(Block* blk, int z, int* crd_1, int* crd_2, int* crd_3)
{
}

//loads given subvolume in a 1-D array of float
real32* UnstitchedVolume::internal_loadSubvolume_to_real32(int &VV0,int &VV1, int &HH0, int &HH1, int &DD0, int &DD1, int V0,int V1, int H0, int H1, int D0, int D1) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d", V0, V1, H0, H1, D0, D1).c_str(), __iim__current__function__);
	
	// dummy variabiles
	StackRestorer *stk_rst = NULL;
	int restore_direction = -1;

	 //initializations
    V0 = V0 < 0 ? 0 : V0;
    H0 = H0 < 0 ? 0 : H0;
    D0 = D0 < 0 ? 0 : D0;
    V1 = (V1 < 0 || V1 > (int)DIM_V) ? DIM_V : V1; 
    H1 = (H1 < 0 || H1 > (int)DIM_H) ? DIM_H : H1; 
    D1 = (D1 < 0 || D1 > (int)DIM_D) ? DIM_D : D1; 

	//if (V0 > 0 || V1 < DIM_V || H0 > 0 || H1 < DIM_H ) // // it is not a complete layer
	//	throw iom::exception(iom::strprintf("only subvolumes corresponding to whole layers can be extracted"), __iom__current__function__);

	// change when subvolumes are enabled
	int row_start;
	int row_end;
	int col_start;
	int col_end;

	// change when subvolumes are enabled
	//row_start = stitcher->ROW_START;
	//row_end   = stitcher->ROW_END;;
	//col_start = stitcher->COL_START;
	//col_end   = stitcher->COL_END;

	// compute which tiles have to be loaded
	int vxl_i;       // last voxel index included in current tile
	int tile_offset; // tile_size - overlap: vxl_i + tile_offset is the last voxel index include in next tile

	// 2016-03-23. Giulio.     @FIXED the way tile indices are found 
	row_start = 0;
	tile_offset = (int) floor( volume->getMEC_V() / volume->getVXL_V() );
	//vxl_i = (int) floor( volume->getMEC_V() / volume->getVXL_V() ); // size the first time and offset the next ones
	vxl_i = volume->getSTACKS()[0][0]->getHEIGHT()-1; // 2016-08-30. Giulio. 
	while ( vxl_i < V0 && row_start < (volume->getN_ROWS()-1) ) { // row_start must be a valid tile index
		row_start++;
		//vxl_i += volume->getSTACKS()[row_start][0]->getHEIGHT(); // size the first time and offset the next ones
		vxl_i += tile_offset;
	}
	row_end   = row_start;
	while ( vxl_i < (V1-1) && row_end < (volume->getN_ROWS()-1) ) { // 2016-08-30. Giulio.
		row_end++;
		// vxl_i += volume->getSTACKS()[row_end][0]->getHEIGHT(); // size the first time and offset the next ones
		vxl_i += tile_offset;
	}

	col_start = 0;
	tile_offset = (int) floor( volume->getMEC_H() / volume->getVXL_H() );
	//vxl_i = (int) floor( volume->getMEC_H() / volume->getVXL_H()); // size the first time and offset the next ones
	vxl_i = volume->getSTACKS()[0][0]->getWIDTH()-1; // 2016-08-30. Giulio.
	while ( vxl_i < H0 && col_start < (volume->getN_COLS()-1) ) {
		col_start++;
		//vxl_i += volume->getSTACKS()[0][col_start]->getWIDTH();
		vxl_i += tile_offset;
	}
	col_end   = col_start;
	while ( vxl_i < (H1-1) && col_end < (volume->getN_COLS()-1) ) { // 2016-08-30. Giulio.
		col_end++;
		//vxl_i += volume->getSTACKS()[0][col_end]->getWIDTH();
		vxl_i += tile_offset;
	}

	// check if tiles have to be extended to guarantee that merging is performed ate the border of the subvolume
	bool found_start, found_end;
	int i;

	// check along the row_start and row_end rows
	bool changed_row_start = false;
	bool changed_row_end   = false;
	found_start = (row_start == 0) ? true : false;
	found_end   = (row_end == volume->getN_ROWS()-1) ? true : false;
	i = col_start;
	while ( i<=col_end && !(found_start && found_end) ) {
		if ( !found_start && ((volume->getSTACKS()[row_start-1][i]->getABS_V() + volume->getSTACKS()[0][0]->getHEIGHT()) >= V0 ) ) {
			// there is a tile in (row_start-1)-th row that overlaps with the subvolume
			row_start--;
			changed_row_start = true; // remember that row_start has been decremented
			found_start = true;
		}
		// 2016-08-30. Giulio. the following test should not be in alternative: eliminated 'else'
		if ( !found_end  && (volume->getSTACKS()[row_end+1][i]->getABS_V() < V1 ) ) {
			// there is a tile in (row_start+1)-th row that overlaps with the subvolume
			row_end++;
			changed_row_end = true; // remember that row_end has been incremented
			found_end = true;
		}
		i++;
	}

	// check along the col_start and col_end columns
	found_start = (col_start == 0) ? true : false;
	found_end   = (col_end == volume->getN_COLS()-1) ? true : false;
	i = changed_row_start ? row_start+1 : row_start; // only originally determied rows have to be checked
	while ( i<=(changed_row_end ? row_end-1 : row_end) && !(found_start && found_end) ) {
		if ( !found_start && ((volume->getSTACKS()[i][col_start-1]->getABS_H() + volume->getSTACKS()[0][0]->getWIDTH()) >= H0 ) ) {
			// there is a tile in (col_start-1)-th column that overlaps with the subvolume
			col_start--;
			found_start = true;
		}
		// 2016-08-30. Giulio. the following test should not be in alternative: eliminated 'else'
		if ( !found_end  && (volume->getSTACKS()[i][col_end+1]->getABS_H() < H1 ) ) {
			// there is a tile in (col_start+1)-th column that overlaps with the subvolume
			col_end++;
			found_end = true;
		}
		i++;
	}

	stitcher->computeVolumeDims(false,row_start,row_end,col_start,col_end,D0+D0_offs,D1+D0_offs); // D indices are mapped from stitched to unstitched volume

	/* WARNING: it is possible that selected tiles do not contain all the subvolume requested 
	 * this should be accepted since it a consequence of how tile submatrices are stitched
	 * the issue must be managed in the calling code that receives a buffer that may not cover the whole subvolume requested
	 * id some data are missing they shoulbe set to zero since it means that the voxel in the missing position is empty
	 */

	// save indices to be used to load selected data
	VV0 = V0 + V0_offs;
	VV1 = V1 + V0_offs;
	HH0 = H0 + H0_offs;
	HH1 = H1 + H0_offs;

	V0 = stitcher->V0;
	V1 = stitcher->V1;
	H0 = stitcher->H0;
	H1 = stitcher->H1;
	D0 = stitcher->D0;
	D1 = stitcher->D1;

	iom::real_t* buffer;								//buffer temporary image data are stored

	if ( !cb->getSubvolume(current_channel,V0,V1,H0,H1,D0,D1,buffer) ) {
	
		//computing VH coordinates of all stripes
		for(int row_index=stitcher->ROW_START; row_index<=stitcher->ROW_END; row_index++)
		{
			stripesCoords[row_index].up_left.V		= stitcher->getStripeABS_V(row_index,true);
			stripesCoords[row_index].up_left.H      = volume->getSTACKS()[row_index][stitcher->COL_START]->getABS_H();
			stripesCoords[row_index].bottom_right.V = stitcher->getStripeABS_V(row_index,false);
			stripesCoords[row_index].bottom_right.H = volume->getSTACKS()[row_index][stitcher->COL_END]->getABS_H()+volume->getStacksWidth();
		}

		// clear stripesCorners
		for ( int i=0; i<volume->getN_ROWS(); i++ ) {
			stripesCorners[i].ups.clear();
			stripesCorners[i].bottoms.clear();
			stripesCorners[i].merged.clear();
		}
	
		//computing stripes corners, i.e. corners that result from the overlap between each pair of adjacent stripes
		for(int row_index=stitcher->ROW_START; row_index<=stitcher->ROW_END; row_index++)
		{
			stripe_corner tmp;

			//for first VirtualStack of every stripe
			tmp.H = volume->getSTACKS()[row_index][stitcher->COL_START]->getABS_H();
			tmp.h = volume->getSTACKS()[row_index][stitcher->COL_START]->getABS_V()-stripesCoords[row_index].up_left.V;
			tmp.up = true;
			stripesCorners[row_index].ups.push_back(tmp);
		
			tmp.h = stripesCoords[row_index].bottom_right.V - volume->getSTACKS()[row_index][stitcher->COL_START]->getABS_V() - volume->getStacksHeight();
			tmp.up = false;
			stripesCorners[row_index].bottoms.push_back(tmp);

			for(int col_index=stitcher->COL_START; col_index<stitcher->COL_END; col_index++)
			{
				if(volume->getSTACKS()[row_index][col_index]->getABS_V() < volume->getSTACKS()[row_index][col_index+1]->getABS_V())
				{
					tmp.H = volume->getSTACKS()[row_index][col_index]->getABS_H() + volume->getStacksWidth();
					tmp.h = volume->getSTACKS()[row_index][col_index+1]->getABS_V() - stripesCoords[row_index].up_left.V;
					tmp.up = true;
					stripesCorners[row_index].ups.push_back(tmp);

					tmp.H = volume->getSTACKS()[row_index][col_index+1]->getABS_H();
					tmp.h = stripesCoords[row_index].bottom_right.V - volume->getSTACKS()[row_index][col_index+1]->getABS_V() - volume->getStacksHeight();
					tmp.up = false;
					stripesCorners[row_index].bottoms.push_back(tmp);
				}
				else
				{
					tmp.H = volume->getSTACKS()[row_index][col_index+1]->getABS_H();
					tmp.h = volume->getSTACKS()[row_index][col_index+1]->getABS_V() - stripesCoords[row_index].up_left.V;
					tmp.up = true;
					stripesCorners[row_index].ups.push_back(tmp);

					tmp.H = volume->getSTACKS()[row_index][col_index]->getABS_H()+volume->getStacksWidth();
					tmp.h = stripesCoords[row_index].bottom_right.V - volume->getSTACKS()[row_index][col_index+1]->getABS_V() - volume->getStacksHeight();
					tmp.up = false;
					stripesCorners[row_index].bottoms.push_back(tmp);
				}
			}

			//for last VirtualStack of every stripe (h is not set because it doesn't matter)
			tmp.H = volume->getSTACKS()[row_index][stitcher->COL_END]->getABS_H() + volume->getStacksWidth();
			tmp.up = true;
			stripesCorners[row_index].ups.push_back(tmp);

			tmp.up = false;
			stripesCorners[row_index].bottoms.push_back(tmp);
		}

		//ordered merging between ups and bottoms corners for every stripe
		for(int row_index = 1; row_index<=(volume->getN_ROWS()-1); row_index++)
		{
			stripesCorners[row_index-1].merged.merge(stripesCorners[row_index-1].bottoms,   compareCorners);
			stripesCorners[row_index-1].merged.merge(stripesCorners[row_index  ].ups,       compareCorners);
		}

		sint64 u_strp_bottom_displ; // bottom displacement of up stripe
		sint64 d_strp_top_displ;    // top displacement of down stripe
		sint64 u_strp_top_displ;    // top displacement of up stripe
		sint64 d_strp_left_displ;
		sint64 u_strp_left_displ;
		sint64 d_strp_width;
		sint64 u_strp_width;
		sint64 dd_strp_top_displ;
		sint64 u_strp_d_strp_overlap = 0; // overlap between up and down stripes; WARNING: check how initialize
		sint64 h_up;
		sint64 h_down;
		sint64 h_overlap;
		iom::real_t *buffer_ptr, *ustripe_ptr, *dstripe_ptr;	

		iom::real_t* stripe_up=NULL, *stripe_down;                                   //will contain up-stripe and down-stripe computed by calling 'getStripe' method
		double angle;								//angle between 0 and PI used to sample overlapping zone in [0,PI]
		double delta_angle;							//angle step

		iom::real_t (*blending)(double& angle, iom::real_t& pixel1, iom::real_t& pixel2);

		//retrieving blending function
		if(blending_algo == S_SINUSOIDAL_BLENDING)
			blending = StackStitcher::sinusoidal_blending;
		else if(blending_algo == S_NO_BLENDING)
			blending = StackStitcher::no_blending;
		else if(blending_algo == S_SHOW_STACK_MARGIN)
			blending = StackStitcher::stack_margin;
		else if(blending_algo == S_ENHANCED_NO_BLENDING)
			blending = StackStitcher::enhanced_no_blending;
		else
 			throw iim::IOException(iom::strprintf("unrecognized blending function"), __iom__current__function__);

		sint64 height;
		sint64 width;
		sint64 depth;  
		int slice_height = -1; 
		int slice_width  = -1;

		width  = H1 - H0;
		height = V1 - V0;
		depth  = D1 - D0;

		slice_height = (int)(slice_height == -1 ? height : slice_height);
		slice_width  = (int)(slice_width  == -1 ? width  : slice_width);

		buffer = new iom::real_t[height*width*depth];
		if ( !buffer )
 			throw iim::IOException(iom::strprintf("cannot allocate the buffer of float (%llu x %llu x %u x %llu = %llu bytes)",
 											height, width, depth, sizeof(iom::real_t), height*width*depth), __iom__current__function__);
		for (int i=0; i<height*width*depth; i++)
			buffer[i]=0;

		sint64 z = D0; // starting slice (offset is index 'k')
		
		// 2017-08-19. Giulio. @ADDED offsets on whole buffer to reduce the amount of data to be copied
		// if 'delta' variables are zero the whole buffer is filled with data returned by getStripe method
		sint64 subvol_V_top_delta    = cb->getENABLED() ? 0 : VV0 - V0;		//top    V(ertical) offset of actual subvolume
		sint64 subvol_V_bottom_delta = cb->getENABLED() ? 0 : V1 - VV1;		//bottom V(ertical) offset of actual subvolume
		
		if ( subvol_V_top_delta < 0 ) {
 			iom::warning(iom::strprintf("V top offset negative: set to 0").c_str(), __iom__current__function__);
 			subvol_V_top_delta = 0;
 		}
		if ( subvol_V_bottom_delta < 0 ) {
 			iom::warning(iom::strprintf("V bottom offset negative: set to 0").c_str(), __iom__current__function__);
 			subvol_V_bottom_delta = 0;
 		}

		for(sint64 k = 0; k < depth; k++)
		{
			//looping on all stripes
			for(int row_index=stitcher->ROW_START; row_index<=stitcher->ROW_END; row_index++)
			{
				//loading down stripe
				if(row_index==stitcher->ROW_START) stripe_up = NULL;

				// 2017-04-12. Giulio. @ADDED release of allocated buffers if an exception is raised in 'getStripe' (prevent further exceptions in the GUI version)
				try {
					// 2017-08-19. Giulio. @CHANGED the method reading only the actually requested data is invoked only if buffering is not enabled
					if ( cb->getENABLED() ) 
						// since 'delta' variables are zero all returned data will be copied in the final buffer
						stripe_down = stitcher->getStripe(row_index,(int)(z+k), restore_direction, stk_rst, blending_algo);
					else
						// only requested data are actually read into the returned buffer and will be copied into the final buffer
						stripe_down = stitcher->getStripe2(row_index,(int)(z+k), VV0, VV1, HH0, HH1, restore_direction, stk_rst, blending_algo);
				}
    			catch( iom::exception& exception) {
    				stitcher->volume->releaseBuffers();
        			throw iom::exception(iom::exception(exception.what()));
    			}

				if(stripe_up) u_strp_bottom_displ	= stripesCoords[row_index-1].bottom_right.V	 - V0;
				d_strp_top_displ					= stripesCoords[row_index  ].up_left.V	     - V0;
				if(stripe_up) u_strp_top_displ      = stripesCoords[row_index-1].up_left.V	     - V0;
				d_strp_left_displ					= stripesCoords[row_index  ].up_left.H		 - H0;
				if(stripe_up) u_strp_left_displ     = stripesCoords[row_index-1].up_left.H		 - H0;
				d_strp_width						= stripesCoords[row_index  ].bottom_right.H - stripesCoords[row_index  ].up_left.H;
				if(stripe_up) u_strp_width			= stripesCoords[row_index-1].bottom_right.H - stripesCoords[row_index-1].up_left.H;
				if(stripe_up) u_strp_d_strp_overlap = u_strp_bottom_displ - d_strp_top_displ;
				if(row_index!=stitcher->ROW_END) 
					dd_strp_top_displ				= stripesCoords[row_index+1].up_left.V		 - V0;
				h_up =  h_down						= u_strp_d_strp_overlap;

/**********************************************************************************************************************************************************************************************************

This comment assumes h_overlap >= 0
The actual code manages also the case h_overlap < 0 (meaning that there are regions uncovered between the up stripe and the down stripe  
The loop copies the down stripe onto the buffer merged with the overlapping zone of the up stripe


                                                                      | index on whole buffer                                        | index on up stripe                           |index on down stripe |
                                                                      |                                                              |                                              |                     |
           -------  -----------------------------------------------   | u_strp_top_displ                                             |                                              |                     |
           |                                                          |                                                              |                                              |                     |
           |                                                          |                                                              |                                              |                     |
           |                                                          |                                                              |                                              |                     |
           |                  already copied in previous cycle        |                                                              |                                              |                     |
           |                                                          |                                                              |                                              |                     |
           /                                                          |                                                              |                                              |                     |
up stripe -                                                           |                                                              |                                              |                     |
           \    --  ...............................................   | d_strp_top_displ                                             | (d_strp_top_displ - u_strp_top_displ)        |                     |
           |    |                      first cycle (only up stripe)   |                                                              |                                              |                     | 
           |    |                      ............................   | d_strp_top_displ+h_up                                        | (d_strp_top_displ + h_up - u_strp_top_displ) | h_up                |
           |    |   OVERLAPPING ZONE   second  cycle (both stripes)   |                                                              |                                              |                     |
           |    |                      ----------------------------   | d_strp_top_displ+h_up+h_overlap                              |                                              | h_up + h_overlap    |
           |    |                      third cycle (only down stripe) |                                                              |                                              |                     |
           ---- /   -----------------------------------------------   | d_strp_top_displ+h_up+h_overlap+h_down = u_strp_bottom_displ | (u_strp_bottom_displ - d_strp_top_displ)     |                     |
down stripe    -                                                      |                                                              |                                              |                     |
                \                                                     |                                                              |                                              |                     |
                |   NON OVERLAPPING ZONE                              | i                                                            | (i - d_strp_top_displ)                       |                     |
                |                                                     |                                                              |                                              |                     |
           ---- |   ___.___.___.___.___.___.___.___.___.___.___.___   | dd_strp_top_displ                                            |                                              |                     |
           |    |                                                     |                                                              |                                              |                     |
           |    |                                                      
           |    --  ...............................................   
           | 
           | 
           /         
dd stripe -                                                           
           \                           next stripe 
           |
           |  
           |  
           |         
           | 
           -------  ___.___.___.___.___.___.___.___.___.___.___.___ 
        
**********************************************************************************************************************************************************************************************************/

				// 2017-08-19. Giulio. @ADDED top offset on down stripe to reduce the amount of data to be copied
				// subvol_V_top_delta = 0 -> d_strp_V_top_delta = 0
				sint64 d_strp_V_top_delta    = (subvol_V_top_delta > d_strp_top_displ) ? (subvol_V_top_delta - d_strp_top_displ) : 0;
							
				//overlapping zone
				if(row_index!=stitcher->ROW_START)
				{	
					// 2017-08-19. Giulio. @ADDED top offset on up stripe to reduce the amount of data to be copied
					// subvol_V_top_delta = 0 -> u_strp_V_top_delta = 0
					sint64 u_strp_V_top_delta    = (subvol_V_top_delta > u_strp_top_displ) ? (subvol_V_top_delta - u_strp_top_displ) : 0;

					std::list<stripe_corner>::iterator cnr_i_next, cnr_i = stripesCorners[row_index-1].merged.begin();
					stripe_corner *cnr_left=&(*cnr_i), *cnr_right;
					cnr_i++;
					cnr_i_next = cnr_i;
					cnr_i_next++;

					while( cnr_i != stripesCorners[row_index-1].merged.end())
					{
						//computing h_up, h_overlap, h_down
						cnr_right = &(*cnr_i);
						if(cnr_i_next == stripesCorners[row_index-1].merged.end())
						{
							h_up =   cnr_left->up ? u_strp_d_strp_overlap : 0;
							h_down = cnr_left->up ? 0                     : u_strp_d_strp_overlap;
						}
						else
							if(cnr_left->up)
								h_up = cnr_left->h;
							else
								h_down = cnr_left->h;
							
						h_overlap = u_strp_d_strp_overlap - h_up - h_down;

						//splitting overlapping zone in sub-regions along H axis
						for(sint64 j= cnr_left->H - H0; j < cnr_right->H - H0; j++)
						{
							delta_angle = PI/(h_overlap-1);
							angle = 0;
							
							//UP stripe zone
							buffer_ptr  = &buffer[k*height*width+std::max<sint64>(d_strp_top_displ,subvol_V_top_delta)*width+j];
							ustripe_ptr = &stripe_up[std::max<sint64>((d_strp_top_displ-u_strp_top_displ),u_strp_V_top_delta)*u_strp_width +j - u_strp_left_displ];
							for(sint64 i=std::max<sint64>(d_strp_top_displ,subvol_V_top_delta); i<std::min<sint64>(d_strp_top_displ+h_up+(h_overlap >= 0 ?  0 : h_overlap),height-subvol_V_bottom_delta); i++, buffer_ptr+=width, ustripe_ptr+= u_strp_width)
								*buffer_ptr = *ustripe_ptr;

							//OVERLAPPING zone
							buffer_ptr  = &buffer[k*height*width+std::max<sint64>((d_strp_top_displ+h_up),subvol_V_top_delta)*width+j];
							ustripe_ptr = &stripe_up[std::max<sint64>((d_strp_top_displ+h_up-u_strp_top_displ),u_strp_V_top_delta)*u_strp_width +j - u_strp_left_displ];
							dstripe_ptr = &stripe_down[std::max<sint64>((d_strp_top_displ+h_up-d_strp_top_displ),d_strp_V_top_delta)*d_strp_width +j - d_strp_left_displ];
							for(sint64 i=std::max<sint64>(d_strp_top_displ+h_up,subvol_V_top_delta); i<std::min<sint64>(d_strp_top_displ+h_up+h_overlap,height-subvol_V_bottom_delta); i++, buffer_ptr+=width, ustripe_ptr+= u_strp_width, dstripe_ptr+=d_strp_width, angle+=delta_angle)
								*buffer_ptr = blending(angle,*ustripe_ptr,*dstripe_ptr);

							//DOWN stripe zone
							buffer_ptr = &buffer[k*height*width+std::max<sint64>((d_strp_top_displ+h_up+(h_overlap >= 0 ? h_overlap : 0)),subvol_V_top_delta)*width+j];
							dstripe_ptr = &stripe_down[std::max<sint64>(((d_strp_top_displ+h_up+(h_overlap >= 0 ? h_overlap : 0))-d_strp_top_displ),d_strp_V_top_delta)*d_strp_width +j - d_strp_left_displ];
							for(sint64 i=std::max<sint64>(d_strp_top_displ+h_up+(h_overlap >= 0 ? h_overlap : 0),subvol_V_top_delta); i<std::min<sint64>(d_strp_top_displ+h_up+h_overlap+h_down,height-subvol_V_bottom_delta); i++, buffer_ptr+=width, dstripe_ptr+=d_strp_width)
								*buffer_ptr = *dstripe_ptr;
						}

						cnr_left = cnr_right;
						cnr_i++;
						if(cnr_i_next != stripesCorners[row_index-1].merged.end())
							cnr_i_next++;
					}
				}

				//non-overlapping zone
				// 2017-08-19. Giulio. @CHANGED only requested data is copied of first stripe if subvol_V_top_delta > 0 and of last stripe if subvol_V_bottom_delta > 0
				buffer_ptr = &buffer[k*height*width+((row_index==stitcher->ROW_START ? subvol_V_top_delta : u_strp_bottom_displ))*width];
				for(sint64 i=(row_index==stitcher->ROW_START ? subvol_V_top_delta : u_strp_bottom_displ); i<(row_index==stitcher->ROW_END? height-subvol_V_bottom_delta : dd_strp_top_displ); i++)
				{
					dstripe_ptr = &stripe_down[std::max<sint64>((i-d_strp_top_displ),d_strp_V_top_delta)*d_strp_width - d_strp_left_displ];
					for(sint64 j=0; j<width; j++, buffer_ptr++, dstripe_ptr++)
						if(j - d_strp_left_displ >= 0 && j - d_strp_left_displ < stripesCoords[row_index].bottom_right.H)
							*buffer_ptr = *dstripe_ptr;
				}

				//moving to bottom stripe_up
				delete stripe_up;
				stripe_up=stripe_down;
			}
			//releasing last stripe_down
			delete stripe_down;
		}

		if ( cb->cacheSubvolume(current_channel,V0,V1,H0,H1,D0,D1,buffer,(height*width*depth)) ) 
			internal_buffer_deallocate = false;
		else
			internal_buffer_deallocate = true;
	}
	else {
		internal_buffer_deallocate = false;
	}

	// 2016-03-23. Giulio.     @ADDED map back from indices on unstitched to indices on stitched volumes
	VV0 = V0 - V0_offs;
	VV1 = V1 - V0_offs;
	HH0 = H0 - H0_offs;
	HH1 = H1 - H0_offs;
	DD0 = D0 - D0_offs;
	DD1 = D1 - D0_offs;
//printf("---> (3) VV0 = %d, V0 = %d, V0_offs = %d\n",VV0,V0,V0_offs);
//printf("---> (3) HH0 = %d, H0 = %d, H0_offs = %d\n",HH0,H0,H0_offs);

	return buffer;
}

//loads given subvolume in a 1-D array of float
real32* UnstitchedVolume::loadSubvolume_to_real32(int V0,int V1, int H0, int H1, int D0, int D1) throw (IOException)
{
	//throw iim::IOException("Not yet implemented", __iom__current__function__);

    /**/iim::debug(iim::LEV3, strprintf("V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d", V0, V1, H0, H1, D0, D1).c_str(), __iim__current__function__);

	if ( DIM_C > 1 && iom::CHANS == iom::ALL ) {
 		throw iim::IOException(iom::strprintf("conversion from multi-channel to intensity images not supported"), __iom__current__function__);
	}

	int VV0, VV1, HH0, HH1, DD0, DD1;
	
	 //initializations
    V0 = V0 < 0 ? 0 : V0;
    H0 = H0 < 0 ? 0 : H0;
    D0 = D0 < 0 ? 0 : D0;
    V1 = (V1 < 0 || V1 > (int)DIM_V) ? DIM_V : V1; 
    H1 = (H1 < 0 || H1 > (int)DIM_H) ? DIM_H : H1; 
    D1 = (D1 < 0 || D1 > (int)DIM_D) ? DIM_D : D1; 

	real32 *buf = internal_loadSubvolume_to_real32(VV0, VV1, HH0, HH1, DD0, DD1, V0, V1, H0, H1, D0, D1);

	if ( VV0 == V0 && HH0 == H0 && DD0 == D0 && VV1 == V1 && HH1 == H1 && DD1 == D1 ) 
		// there is no need to extract the subvolume from the returned buffer
		return buf;

	// extract requested subvolume from the returned buffer

	sint64 s_stridex  = HH1 - HH0;
	sint64 s_stridexy = s_stridex * (VV1 - VV0);

	// 2017-07-15. Giulio. must be introduces in case the returned buffer is smaller than the requested subvolume
	sint64 d_stridex  = H1 - H0;
	sint64 d_stridexy = d_stridex * (V1 - V0);

	sint64 sbv_width = H1 - H0;
	sint64 sbv_height = V1 - V0;
	sint64 sbv_depth = D1 - D0;

    real32 *subvol = new real32[sbv_width * sbv_height * sbv_depth];
	memset(subvol,0,sizeof(real32)*(sbv_width * sbv_height * sbv_depth));

	int i, j, k;
	real32 *ptr_d_xy;
	real32 *ptr_d_x;
	real32 *ptr_d;
	real32 *ptr_s_xy;
	real32 *ptr_s_x;
	real32 *ptr_s;

	// 2017-07-15. Giulio. The following for loop manage both the case the returned buffer is larger than the requested subvolume (typical case)
	// and the case the returned buffer is smaller than the requested subvolume

	for ( k=std::max<int>(D0,DD0), 
			ptr_d_xy=subvol + std::max<int>(0,(VV0 - V0))*d_stridex + std::max<int>(0,(HH0 - H0)), 
			ptr_s_xy=buf + std::max<int>(0,(V0 - VV0))*s_stridex + std::max<int>(0,(H0 - HH0)); 
			k<std::min<int>(D1,DD1); 
			k++, ptr_d_xy+=d_stridexy, ptr_s_xy+=s_stridexy )
		for ( i=std::max<int>(V0,VV0), 
				ptr_d_x=ptr_d_xy, 
				ptr_s_x=ptr_s_xy; 
			    i<std::min<int>(V1,VV1); 
				i++, ptr_d_x+=d_stridex, ptr_s_x+=s_stridex )
			for ( j=std::max<int>(H0,HH0), 
					ptr_d=ptr_d_x, 
					ptr_s=ptr_s_x; 
				    j<std::min<int>(H1,HH1); 
					j++, ptr_d++, ptr_s++ )
				*ptr_d = *ptr_s;

	if ( internal_buffer_deallocate ) 
		delete []buf;

	return subvol;

}

//loads given subvolume in a 1-D array of iim::uint8 while releasing stacks slices memory when they are no longer needed
//---03 nov 2011: added color support
iim::uint8* UnstitchedVolume::loadSubvolume_to_UINT8(int V0,int V1, int H0, int H1, int D0, int D1, int *channels, int ret_type ) throw (IOException, iom::exception)
{
    /**/iim::debug(iim::LEV3, strprintf("V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d, *channels=%d, ret_type=%d", V0, V1, H0, H1, D0, D1, channels ? *channels : -1, ret_type).c_str(), __iim__current__function__);

	int VV0, VV1, HH0, HH1, DD0, DD1;
	
    if ( (ret_type != iim::NATIVE_RTYPE) && (ret_type != iim::DEF_IMG_DEPTH) ) {
		// return type should be converted, but not to 8 bits per channel
        throw iim::IOException(iom::strprintf("non supported return type (%d bits) - native type is %d bits", ret_type, 8*this->BYTESxCHAN), __iim__current__function__); 
	}

	// reduction factor to be applied to the loaded buffer
    int red_factor = (ret_type == iim::NATIVE_RTYPE) ? 1 : ((8 * this->BYTESxCHAN) / ret_type);

	//initializations
    V0 = V0 < 0 ? 0 : V0;
    H0 = H0 < 0 ? 0 : H0;
    D0 = D0 < 0 ? 0 : D0;
    V1 = (V1 < 0 || V1 > (int)DIM_V) ? DIM_V : V1; 
    H1 = (H1 < 0 || H1 > (int)DIM_H) ? DIM_H : H1; 
    D1 = (D1 < 0 || D1 > (int)DIM_D) ? DIM_D : D1; 

	bool chans_interleaved = (plugin_type.compare("3D image-based I/O plugin") == 0) ?
								iom::IOPluginFactory::getPlugin3D(iom::IMIN_PLUGIN)->isChansInterleaved() :
								iom::IOPluginFactory::getPlugin2D(iom::IMIN_PLUGIN)->isChansInterleaved();
	int n_chans = (n_active < DIM_C) ? n_active : DIM_C;
	if ( chans_interleaved )
		if ( n_chans <= 3 ) {
			switch (active[0]) {
				case 0: 
					iom::CHANS = iom::R;
					current_channel = 0;
					break;
				case 1:
					iom::CHANS = iom::G;
					current_channel = 1;
					break;
				case 2:
					iom::CHANS = iom::B;
					current_channel = 2;
					break;
			}
		}
		else {
  			throw iim::IOException(iom::strprintf("pulgins with interleaved channels do not support %d channels", n_chans), __iim__current__function__);
		}
	else { // channels are not interleaved in the returned buffer 'data'
		volume->setACTIVE_CHAN(active[0]);
		current_channel = active[0];
	}

	real32 *buf = internal_loadSubvolume_to_real32(VV0, VV1, HH0, HH1, DD0, DD1, V0, V1, H0, H1, D0, D1);
	// 2017-07-15. Giulio. this check is no more needed: the case the returned buffer is smaller than the requested subvolume is explicitly managed by the code 
	//if ( VV0 > V0 || HH0 > H0 || DD0 > D0 || VV1 < V1 || HH1 < H1 || DD1 < D1 ) {
 // 		throw iim::IOException(iom::strprintf("returned buffer is smaller than the requested subvolume (requested [V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d] -- returned [VV0=%d, VV1=%d, HH0=%d, HH1=%d, DD0=%d, DD1=%d])", 
	//											V0, V1, H0, H1, D0, D1, VV0, VV1, HH0, HH1, DD0, DD1), __iim__current__function__);
	//}

	// change when subvolumes are enabled
	sint64 s_stridex  = HH1 - HH0;
	sint64 s_stridexy = s_stridex * (VV1 - VV0);

	// 2017-07-15. Giulio. must be introduces in case the returned buffer is smaller than the requested subvolume
	sint64 d_stridex  = H1 - H0;
	sint64 d_stridexy = d_stridex * (V1 - V0);

	sint64 sbv_width = H1 - H0;
	sint64 sbv_height = V1 - V0;
	sint64 sbv_depth = D1 - D0;

    uint8 *subvol = new uint8[sbv_width * sbv_height * sbv_depth * n_chans * (BYTESxCHAN/red_factor)];
	if ( !subvol )
 		throw iim::IOException(iom::strprintf("cannot allocate the buffer of uint8 (%llu x %llu x %llu x %u x %u = %llu bytes)",
 			sbv_width, sbv_height, sbv_depth, n_chans, (BYTESxCHAN/red_factor), sbv_width * sbv_height * sbv_depth * n_chans * (BYTESxCHAN/red_factor)), __iom__current__function__);
	memset(subvol,0,sizeof(uint8)*(sbv_width * sbv_height * sbv_depth * n_chans * (BYTESxCHAN/red_factor)));

	// 2017-07-15. Giulio. All the following for loops manage both the case the returned buffer is larger than the requested subvolume (typical case)
	// and the case the returned buffer is smaller than the requested subvolume

	int i, j, k, c;
	real32 *ptr_s_xy;
	real32 *ptr_s_x;
	real32 *ptr_s;
	if ( BYTESxCHAN == 1 ) {

		uint8  *ptr_d_xy;
		uint8  *ptr_d_x;
		uint8  *ptr_d;

		for ( k=std::max<int>(D0,DD0), 
			  ptr_d_xy=subvol + std::max<int>(0,(VV0 - V0))*d_stridex + std::max<int>(0,(HH0 - H0)), 
			  ptr_s_xy=buf + std::max<int>(0,(V0 - VV0))*s_stridex + std::max<int>(0,(H0 - HH0)); 
			  k<std::min<int>(D1,DD1); 
			  k++, ptr_d_xy+=d_stridexy, ptr_s_xy+=s_stridexy )
			for ( i=std::max<int>(V0,VV0), 
				  ptr_d_x=ptr_d_xy, 
				  ptr_s_x=ptr_s_xy; 
			      i<std::min<int>(V1,VV1); 
				  i++, ptr_d_x+=d_stridex, ptr_s_x+=s_stridex )
				for ( j=std::max<int>(H0,HH0), 
					  ptr_d=ptr_d_x, 
					  ptr_s=ptr_s_x; 
				      j<std::min<int>(H1,HH1); 
					  j++, ptr_d++, ptr_s++ )
					*ptr_d = uint8(*ptr_s * 255.0f);

		for ( c=1; c<n_chans; c++ ) { // more than one channel
			if ( internal_buffer_deallocate )
				delete []buf;
			if ( chans_interleaved ) {
				switch (active[c]) {
					case 0: 
						iom::CHANS = iom::R;
						current_channel = 0;
						break;
					case 1:
						iom::CHANS = iom::G;
						current_channel = 1;
						break;
					case 2:
						iom::CHANS = iom::B;
						current_channel = 2;
						break;
				}
			}
			else {
				volume->setACTIVE_CHAN(active[c]);
				current_channel = active[c];
			}
			// loads next channel
			buf = internal_loadSubvolume_to_real32(VV0, VV1, HH0, HH1, DD0, DD1, V0, V1, H0, H1, D0, D1);
			// append next channel to the subvolume buffer
			for ( k=std::max<int>(D0,DD0), 
				  // 2017-07-15. Giulio. ptr_d_xy does not need to be initialized, it is correctly set by previous loops 
				  ptr_s_xy=buf + std::max<int>(0,(V0 - VV0))*s_stridex + std::max<int>(0,(H0 - HH0)); 
				  k<std::min<int>(D1,DD1); 
				  k++, ptr_d_xy+=d_stridexy, ptr_s_xy+=s_stridexy )
				for ( i=std::max<int>(V0,VV0), 
					  ptr_d_x=ptr_d_xy, 
					  ptr_s_x=ptr_s_xy; 
					  i<std::min<int>(V1,VV1); 
					  i++, ptr_d_x+=d_stridex, ptr_s_x+=s_stridex )
					for ( j=std::max<int>(H0,HH0), 
						  ptr_d=ptr_d_x, 
						  ptr_s=ptr_s_x; 
						  j<std::min<int>(H1,HH1); 
						  j++, ptr_d++, ptr_s++ )
						*ptr_d = uint8(*ptr_s * 255.0f);
		}
	}
	else if ( BYTESxCHAN == 2 ) {

		if ( red_factor == 1 ) {

			uint16  *ptr_d_xy;
			uint16  *ptr_d_x;
			uint16  *ptr_d;

			for ( k=std::max<int>(D0,DD0), 
				  ptr_d_xy=(uint16 *)subvol + std::max<int>(0,(VV0 - V0))*d_stridex + std::max<int>(0,(HH0 - H0)), 
				  ptr_s_xy=buf + std::max<int>(0,(V0 - VV0))*s_stridex + std::max<int>(0,(H0 - HH0)); 
				  k<std::min<int>(D1,DD1); 
				  k++, ptr_d_xy+=d_stridexy, ptr_s_xy+=s_stridexy )
				for ( i=std::max<int>(V0,VV0), 
					  ptr_d_x=ptr_d_xy, 
					  ptr_s_x=ptr_s_xy; 
					  i<std::min<int>(V1,VV1); 
					  i++, ptr_d_x+=d_stridex, ptr_s_x+=s_stridex )
					for ( j=std::max<int>(H0,HH0), 
						  ptr_d=ptr_d_x, 
						  ptr_s=ptr_s_x; 
						  j<std::min<int>(H1,HH1); 
						  j++, ptr_d++, ptr_s++ )
						*ptr_d = uint16(*ptr_s * 65535.0F);

			for ( c=1; c<n_chans; c++ ) { // more than one channel
				if ( internal_buffer_deallocate )
					delete []buf;
				if ( chans_interleaved ) {
					switch (active[c]) {
						case 0: 
							iom::CHANS = iom::R;
							current_channel = 0;
							break;
						case 1:
							iom::CHANS = iom::G;
							current_channel = 1;
							break;
						case 2:
							iom::CHANS = iom::B;
							current_channel = 2;
							break;
					}
				}
				else {
					volume->setACTIVE_CHAN(active[c]);
					current_channel = active[c];
				}
				// loads next channel
				buf = internal_loadSubvolume_to_real32(VV0, VV1, HH0, HH1, DD0, DD1, V0, V1, H0, H1, D0, D1);
				// append next channels to the subvolume buffer
				for ( k=std::max<int>(D0,DD0), 
					  // 2017-07-15. Giulio. ptr_d_xy does not need to be initialized, it is correctly set by previous loops 
					  ptr_s_xy=buf + std::max<int>(0,(V0 - VV0))*s_stridex + std::max<int>(0,(H0 - HH0)); 
					  k<std::min<int>(D1,DD1); 
					  k++, ptr_d_xy+=d_stridexy, ptr_s_xy+=s_stridexy )
					for ( i=std::max<int>(V0,VV0), 
						  ptr_d_x=ptr_d_xy, 
						  ptr_s_x=ptr_s_xy; 
						  i<std::min<int>(V1,VV1); 
						  i++, ptr_d_x+=d_stridex, ptr_s_x+=s_stridex )
						for ( j=std::max<int>(H0,HH0), 
							  ptr_d=ptr_d_x, 
							  ptr_s=ptr_s_x; 
							  j<std::min<int>(H1,HH1); 
							  j++, ptr_d++, ptr_s++ )
							*ptr_d = uint16(*ptr_s * 65535.0F);
			}
		}
		// 2015-12-28. Giulio. @ADDED conversion from 16 to 8 bits depth
		else if ( red_factor == 2 ) { 

			uint8  *ptr_d_xy;
			uint8  *ptr_d_x;
			uint8  *ptr_d;   // WARNING: assumes that red_factor = 2 (see check on ret_type)

			// find maximum value in channel R
			real32 valmax = 0;
			for ( k=std::max<int>(D0,DD0), 
				  ptr_s_xy=buf + std::max<int>(0,(V0 - VV0))*s_stridex + std::max<int>(0,(H0 - HH0)); 
				  k<std::min<int>(D1,DD1); 
				  k++, ptr_s_xy+=s_stridexy )
				for ( i=std::max<int>(V0,VV0), 
					  ptr_s_x=ptr_s_xy; 
					  i<std::min<int>(V1,VV1); 
					  i++, ptr_s_x+=s_stridex )
					for ( j=std::max<int>(H0,HH0), 
						  ptr_s=ptr_s_x; 
						  j<std::min<int>(H1,HH1); 
						  j++, ptr_s++ )
						if (*ptr_s > valmax )
							valmax = *ptr_s;
			// normalize and convert to required depth
			for ( k=std::max<int>(D0,DD0), 
				  ptr_d_xy=subvol + std::max<int>(0,(VV0 - V0))*d_stridex + std::max<int>(0,(HH0 - H0)), 
				  ptr_s_xy=buf + std::max<int>(0,(V0 - VV0))*s_stridex + std::max<int>(0,(H0 - HH0)); 
				  k<std::min<int>(D1,DD1); 
				  k++, ptr_d_xy+=d_stridexy, ptr_s_xy+=s_stridexy )
				for ( i=std::max<int>(V0,VV0), 
					  ptr_d_x=ptr_d_xy, 
					  ptr_s_x=ptr_s_xy; 
					  i<std::min<int>(V1,VV1); 
					  i++, ptr_d_x+=d_stridex, ptr_s_x+=s_stridex )
					for ( j=std::max<int>(H0,HH0), 
						  ptr_d=ptr_d_x, 
						  ptr_s=ptr_s_x; 
						  j<std::min<int>(H1,HH1); 
						  j++, ptr_d++, ptr_s++ )
						*ptr_d = uint8((*ptr_s/valmax) * 255.0F);

			for ( c=1; c<n_chans; c++ ) { // more than one channel
				if ( internal_buffer_deallocate )
					delete []buf;
				if ( chans_interleaved ) {
					switch (active[c]) {
						case 0: 
							iom::CHANS = iom::R;
							current_channel = 0;
							break;
						case 1:
							iom::CHANS = iom::G;
							current_channel = 1;
							break;
						case 2:
							iom::CHANS = iom::B;
							current_channel = 2;
							break;
					}
				}
				else {
					volume->setACTIVE_CHAN(active[c]);
					current_channel = active[c];
				}
				// loads next channel
				buf = internal_loadSubvolume_to_real32(VV0, VV1, HH0, HH1, DD0, DD1, V0, V1, H0, H1, D0, D1);
				// append next channels to the subvolume buffer
				// find maximum value in channel c
				for ( k=std::max<int>(D0,DD0), 
					  ptr_s_xy=buf + std::max<int>(0,(V0 - VV0))*s_stridex + std::max<int>(0,(H0 - HH0)); 
					  k<std::min<int>(D1,DD1); 
					  k++, ptr_s_xy+=s_stridexy )
					for ( i=std::max<int>(V0,VV0), 
						  ptr_s_x=ptr_s_xy; 
						  i<std::min<int>(V1,VV1); 
						  i++, ptr_s_x+=s_stridex )
						for ( j=std::max<int>(H0,HH0), 
							  ptr_s=ptr_s_x; 
							  j<std::min<int>(H1,HH1); 
							  j++, ptr_s++ )
							if (*ptr_s > valmax )
								valmax = *ptr_s;
				// normalize and convert to required depth
				for ( k=std::max<int>(D0,DD0), 
					  // 2017-07-15. Giulio. ptr_d_xy does not need to be initialized, it is correctly set by previous loops 
					  ptr_s_xy=buf + std::max<int>(0,(V0 - VV0))*s_stridex + std::max<int>(0,(H0 - HH0)); 
					  k<std::min<int>(D1,DD1); 
					  k++, ptr_d_xy+=d_stridexy, ptr_s_xy+=s_stridexy )
					for ( i=std::max<int>(V0,VV0), 
						  ptr_d_x=ptr_d_xy, 
						  ptr_s_x=ptr_s_xy; 
						  i<std::min<int>(V1,VV1); 
						  i++, ptr_d_x+=d_stridex, ptr_s_x+=s_stridex )
						for ( j=std::max<int>(H0,HH0), 
							  ptr_d=ptr_d_x, 
							  ptr_s=ptr_s_x; 
							  j<std::min<int>(H1,HH1); 
							  j++, ptr_d++, ptr_s++ )
							*ptr_d = uint8((*ptr_s/valmax) * 255.0F);
			}
		}
		else 
  			throw iim::IOException(iom::strprintf("wrong reduction factor (%d) for 16 bits images", red_factor), __iim__current__function__);
	}
	else
  		throw iim::IOException(iom::strprintf("%d bits depth not currently supported", BYTESxCHAN*8), __iim__current__function__);

	if ( internal_buffer_deallocate ) 
		delete []buf;

	if ( channels )
		*channels = n_chans;

	return subvol;
}

iim::uint8* UnstitchedVolume::loadSubvolume_to_UINT8_MT(int V0,int V1, int H0, int H1, int D0, int D1, int *channels, int ret_type ) throw (IOException, iom::exception)
{
    return NULL;
}


#define _MBYTE_   1048576.0
#define _BUFSIZE_   (128.0 * 128.0 * 128.0 / _MBYTE_)

iim::CacheBuffer::CacheBuffer ( UnstitchedVolume *_volume, iim::uint64 _bufSizeMB, bool _enable, bool _printstats ) {

	volume = _volume;

	// set optional parameters
	char *params;
	if ( (params = getenv("__UNST_CACHE__")) ) {
		char *sptr = strstr(params,"enable:");
		if ( sptr ) {
			sscanf(sptr+strlen("enable:"),"%llu",&_bufSizeMB);
			_enable = true;
		}
		sptr = strstr(params,"printstats:");
		if ( sptr ) {
			if ( strncmp(sptr+strlen("printstats:"),"true",strlen("true")) == 0 )
				_printstats = true;
			else
				_printstats = false;
		}
	}
	bufSizeMB = _bufSizeMB;
	enabled = _enable; 
	printstats = _printstats;
	
	caccesses = chits = cmisses = crplcmnts = 0;

	cachedMB    = 0.0;
	maxCachedMB = 0.0;

	N_CHANS = volume->getDIM_C();

	dq_buffers = new deque<bufEntry>[N_CHANS];
	it_active = false;
	timestamp = 0;
}


iim::CacheBuffer::~CacheBuffer () {
	if ( dq_buffers )
		delete []dq_buffers;

	if ( printstats ) {
		printf("Cache statistics: accesses = %llu, hits = %llu, misses = %llu, replacements = %llu, maximum cached (MB) = %f \n",caccesses,chits,cmisses,crplcmnts,maxCachedMB);
	}
}


bool iim::CacheBuffer::scan_sbvID ( int VV0, int VV1, int HH0, int HH1, int DD0, int DD1, bufEntry *e, bool &found ) {
	found = e->buf && (e->VV0 <= VV0) && (VV1 <= e->VV1) && (e->HH0 <= HH0) && (HH1 <= e->HH1) && (e->DD0 <= DD0) && (DD1 <= e->DD1);
	return found || ( (((uint64)(VV1 - VV0)) * ((uint64)(HH1 - HH0)) * ((uint64)(DD1 - DD0))) > (((uint64)(e->VV1 - e->VV0)) * ((uint64)(e->HH1 - e->HH0)) * ((uint64)(e->DD1 - e->DD0))) );
}


bool iim::CacheBuffer::wider_sbvID ( int VV0, int VV1, int HH0, int HH1, int DD0, int DD1, bufEntry *e ) {
	return (e->VV0 >= VV0) && (VV1 >= e->VV1) && (e->HH0 >= HH0) && (HH1 >= e->HH1) && (e->DD0 >= DD0) && (DD1 >= e->DD1);
}


bool iim::CacheBuffer::cacheSubvolume ( int chan, int VV0, int VV1, int HH0, int HH1, int DD0, int DD1, iom::real_t *sbv, iim::sint64 size ) {

	if ( enabled ) {

		if ( sizeof(iom::real_t) * size/_MBYTE_ > bufSizeMB ) {
			// the buffer is too big: cannot be cached
			iom::warning(iom::strprintf("subvolume cannot be cached: too big (%f MB)", sizeof(iom::real_t) * size/_MBYTE_).c_str(),__iom__current__function__);
			return false;
		}

		// PRE: the buffer is not contained in any other buffer in the cache
		// PRE: it_active = true
		// PRE: the buffer must be inserted at dq_it 

		if ( !it_active ) { // the buffer has not been searched before caching
  			throw iim::IOException(iom::strprintf("the buffer has not been searched before caching"), __iim__current__function__);
		}

		// insert buffer in cache
		bufEntry betemp;
		betemp.buf    = sbv;
		betemp.ts     = timestamp;
		betemp.valid  = true;
		betemp.chan   = chan;
		betemp.VV0    = VV0;
		betemp.VV1    = VV1;
		betemp.HH0    = HH0;
		betemp.HH1    = HH1;
		betemp.DD0    = DD0;
		betemp.DD1    = DD1;
		betemp.sizeMB = sizeof(iom::real_t) * size/_MBYTE_;
		dq_it = dq_buffers[chan].insert(dq_it,betemp);
		it_active = false;

		// insert entry in the story
		bufID bidtemp;
		bidtemp.ts         = timestamp;
		bidtemp.valid      = true;
		bidtemp.chan       = chan,
		bidtemp.bufs_entry = &(*dq_it);
		bufStory.push_front(bidtemp);

		dq_it->storyEntry = &bufStory.at(0); // link the buffer entry to his story

		cachedMB += sizeof(iom::real_t) * size/_MBYTE_;
		timestamp++;

		// look for contained buffers to be eliminated (smaller than the current one)
		dq_it++; // must start after last insertion
		while (  dq_it != dq_buffers[chan].end() ) {
			if ( dq_it->valid ) {
				// valid entry
				if ( wider_sbvID(VV0,VV1,HH0,HH1,DD0,DD1,&(*dq_it)) ) { // found a buffer that is contained in the current buffer
					// dispose the found buffer
					delete dq_it->buf;
					cachedMB -= dq_it->sizeMB;
					dq_it->storyEntry->valid = false; // make invalid buffer story
					dq_it = dq_buffers[chan].erase(dq_it);
					crplcmnts++;
				}
				else
					dq_it++;
			}
			else {
				// invalid entry remove it
				dq_it = dq_buffers[chan].erase(dq_it);
			}
		}

		// remobe buffers if the cache is full
		while ( cachedMB > bufSizeMB ) { 
			// cache full some buffers must be disposed
			if ( bufStory.back().valid ) {
				// dispose the oldest buffer and remove it from buffer deque
				// entry of the older buffer is guaranteed to be valid
				delete bufStory.back().bufs_entry->buf;
				bufStory.back().bufs_entry->buf = (iom::real_t *) 0;
				cachedMB -= bufStory.back().bufs_entry->sizeMB;
				bufStory.back().bufs_entry->valid = false; // make invalid buffer entry
				crplcmnts++;
			}
			bufStory.pop_back(); // buffer story must be removed
		}
		
		if ( cachedMB > maxCachedMB )
			maxCachedMB = cachedMB;

		return true;	
	}
	else
		return false;
}


bool iim::CacheBuffer::getSubvolume ( int chan, int &VV0, int &VV1, int &HH0,  int &HH1, int &DD0, int &DD1, iom::real_t *&sbv ) {

	bool stop, found2;

	if ( enabled ) {

		caccesses++;

		dq_it = dq_buffers[chan].begin();
		stop = false;
		found2 = false;
		while ( dq_it != dq_buffers[chan].end() && !stop ) {
			if ( dq_it->valid ) {
				// valid entry 
				if ( scan_sbvID(VV0,VV1,HH0,HH1,DD0,DD1,&(*dq_it),found2) ) {
					stop = true;
				}
				else {
					dq_it++;
				}
			}
			else {
				// invalid entry: remove it
				dq_it = dq_buffers[chan].erase(dq_it);
			}
		}

		if ( found2 ) {
			sbv = dq_it->buf;
			VV0 = dq_it->VV0;
			VV1 = dq_it->VV1;
			HH0 = dq_it->HH0;
			HH1 = dq_it->HH1;
			DD0 = dq_it->DD0;
			DD1 = dq_it->DD1;
			it_active = false; // buffer found: dq_it undefined
			chits++;
			return true;
		}
		else {
			it_active = true; // buffer not found: dq_it identifies the insertion point of the buffer that will be created
			cmisses++;
			return false;
		}
	}
	else
		return false;
}


		

