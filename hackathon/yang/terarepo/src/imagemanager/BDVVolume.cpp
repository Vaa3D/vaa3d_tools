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
* 2015-12-29. Giulio.     @ADDED 'loadSubvolume_to_UINT8' now check the ret_type parameter to determine voxel depth to be returned 
* 2015-12-29. Giulio.     @FIXED 'loadSubvolume_to_UINT8' did not check the ret_type parameter to determine voxel depth to be returned 
* 2015-12-29. Giulio.     @FIXED active channel management in 'loadSubvolume_to_UINT8'
* 2015-11-30. Giulio.     @CREATED
*/

#include <iostream>
#include <string>
#include "BDVVolume.h"
#include "HDF5Mngr.h"

#ifdef _WIN32
#include "dirent_win.h"
#else
#include <dirent.h>
#endif
//#include <cxcore.h>
//#include <cv.h>
//#include <highgui.h>
#include <list>
#include <fstream>
#include "ProgressBar.h"

using namespace std;
using namespace iim;

BDVVolume::BDVVolume(void) : VirtualVolume()
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);
    
	BDV_descr  = (void *) 0;
	HDF5_descr = (void *) 0;

    VXL_V = VXL_2 ;
    VXL_H = VXL_1 ;
    VXL_D = VXL_3 ;

	n_active = 0;
	active = (uint32 *) 0;

}

BDVVolume::BDVVolume(const char* _root_dir, int res, int tp, void *_BDV_descr )  throw (IOException)
: VirtualVolume() // _root_dir can be NULL
{
	if ( _root_dir ) {
		/**/iim::debug(iim::LEV3, strprintf("_root_dir=%s", _root_dir).c_str(), __iim__current__function__);
		root_dir = new char[strlen(_root_dir)+1];
		strcpy(this->root_dir, _root_dir);
		if ( !_BDV_descr ) { // file name is given, BDV descriptor is local
			BDV_HDF5init(std::string(_root_dir),BDV_descr);
			BDV_HDF5getVolumeInfo(BDV_descr, tp, res, HDF5_descr, VXL_1, VXL_2, VXL_3, ORG_V, ORG_H, ORG_D, 
																 DIM_V, DIM_H, DIM_D, DIM_C, BYTESxCHAN, DIM_T, t0, t1 );
		}
		else
			throw iim::IOException(iim::strprintf("A BDV descriptor cannot be passed if a file name is specified (_root_dir=%s)", _root_dir).c_str(),__iim__current__function__);
	}
	else // a file name is not given 
	if ( _BDV_descr ) { // BDV is given and not local
		BDV_descr = (void *) 0;
		BDV_HDF5getVolumeInfo(_BDV_descr, tp, res, HDF5_descr, VXL_1, VXL_2, VXL_3, ORG_V, ORG_H, ORG_D, DIM_V, DIM_H, DIM_D, DIM_C, BYTESxCHAN, DIM_T, t0, t1 );
	}
	else
		throw iim::IOException(iim::strprintf("A HDF5 descriptor must be passed if a file name is not specified").c_str(),__iim__current__function__);
		 

    VXL_V = VXL_2 ;
    VXL_H = VXL_1 ;
    VXL_D = VXL_3 ;

	n_active = DIM_C;
	active = new uint32[n_active];
	for ( int c=0; c<DIM_C; c++ )
		active[c] = c; // all channels are assumed active
}

BDVVolume::~BDVVolume(void) throw (iim::IOException)
{
	if ( BDV_descr )
		BDV_HDF5close(BDV_descr);
	if ( HDF5_descr )
		BDV_HDF5closeVolume(HDF5_descr);
}


//PRINT method
//void BDVVolume::print( bool print_stacks )
//{
//	printf("*** Begin printing BDVVolume object...\n\n");
//	printf("\tDirectory:\t%s\n", root_dir);
//	printf("\tDimensions:\t\t%d(V) x %d(H) x %d(D)\n", DIM_V, DIM_H, DIM_D);
//	printf("\tVoxels:\t\t\t%.4f(V) x %.4f(H) x %.4f(D)\n", VXL_V, VXL_H, VXL_D);
//	printf("\tOrigin:\t\t\t%.4f(V) x %.4f(H) x %.4f(D)\n", ORG_V, ORG_H, ORG_D);
//	printf("\tChannels:\t\t%d\n", DIM_C);
//	printf("\tBytes per channel:\t%d\n", BYTESxCHAN);
//	printf("\tReference system:\tref1=%d, ref2=%d, ref3=%d\n",reference_system.first,reference_system.second,reference_system.third);
//	printf("\tChannels:\n");
//	//for ( int c=0; c<DIM_C; c++ ) {
//	//	printf("\t\tChannel: %d\n",c);
//	//	vol_ch[c]->print(print_stacks);
//	//}
//	printf("\n*** END printing BDVVolume object...\n\n");
//}


//loads given subvolume in a 1-D array of float
real32* BDVVolume::loadSubvolume(int V0,int V1, int H0, int H1, int D0, int D1, list<Block*> *involved_blocks, bool release_blocks) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d, %s", V0, V1, H0, H1, D0, D1, (involved_blocks? ", involved_stacks" : "")).c_str(), __iim__current__function__);

	char msg[STATIC_STRINGS_SIZE];
	sprintf(msg,"in BDVVolume::loadSubvolume: not completed yet");
    throw IOException(msg);

	//initializations
	V0 = (V0 == -1 ? 0	     : V0);
	V1 = (V1 == -1 ? DIM_V   : V1);
	H0 = (H0 == -1 ? 0	     : H0);
	H1 = (H1 == -1 ? DIM_H   : H1);
	D0 = (D0 == -1 ? 0		 : D0);
	D1 = (D1 == -1 ? DIM_D	 : D1);

	//allocation
	sint64 sbv_height = V1 - V0;
	sint64 sbv_width  = H1 - H0;
	sint64 sbv_depth  = D1 - D0;
    real32 *subvol = new real32[sbv_height * sbv_width * sbv_depth];

	//scanning of stacks matrix for data loading and storing into subvol
	Rect_t subvol_area;
	subvol_area.H0 = H0;
	subvol_area.V0 = V0;
	subvol_area.H1 = H1;
	subvol_area.V1 = V1;

	return subvol;
}

//loads given subvolume in a 1-D array of uint8 while releasing stacks slices memory when they are no longer needed
//---03 nov 2011: added color support
uint8* BDVVolume::loadSubvolume_to_UINT8(int V0,int V1, int H0, int H1, int D0, int D1, int *channels, int ret_type) throw (IOException, iom::exception)
{
    /**/iim::debug(iim::LEV3, strprintf("V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d, *channels=%d, ret_type=%d", V0, V1, H0, H1, D0, D1, channels ? *channels : -1, ret_type).c_str(), __iim__current__function__);

     if ( (ret_type != iim::NATIVE_RTYPE) && (ret_type != iim::DEF_IMG_DEPTH) ) {
		// return type should be converted, but not to 8 bits per channel
        throw iim::IOException(iim::strprintf("non supported return type (%d bits) - native type is %d bits", ret_type, 8*this->BYTESxCHAN), __iim__current__function__); 
	}

	// reduction factor to be applied to the loaded buffer
    int red_factor = (ret_type == iim::NATIVE_RTYPE) ? 1 : ((8 * this->BYTESxCHAN) / ret_type);

	//char *err_rawfmt;

    //initializations
    V0 = V0 < 0 ? 0 : V0;
    H0 = H0 < 0 ? 0 : H0;
    D0 = D0 < 0 ? 0 : D0;
    V1 = (V1 < 0 || V1 > (int)DIM_V) ? DIM_V : V1; // iannello MODIFIED
    H1 = (H1 < 0 || H1 > (int)DIM_H) ? DIM_H : H1; // iannello MODIFIED
    D1 = (D1 < 0 || D1 > (int)DIM_D) ? DIM_D : D1; // iannello MODIFIED

    //checking that the interval is valid
    if(V1-V0 <=0 || H1-H0 <= 0 || D1-D0 <= 0)
        throw IOException(iim::strprintf("in BDVVolume::loadSubvolume_to_UINT8: invalid subvolume intervals"), __iim__current__function__);

    //computing dimensions
    sint64 sbv_height = V1 - V0;
    sint64 sbv_width  = H1 - H0;
    sint64 sbv_depth  = D1 - D0;

	sint64 sbv_ch_dim = sbv_height * sbv_width * sbv_depth  * (BYTESxCHAN/red_factor);

    uint8 *subvol   = new uint8[n_active * sbv_ch_dim];
	//uint8 *subvol_ch;

	for ( int c=0; c<n_active; c++ ) {
		BDV_HDF5getSubVolume(HDF5_descr,V0,V1,H0,H1,D0,D1,active[c],(subvol + c*sbv_ch_dim),red_factor); // 2015-12-29 Giulio. @FIXED active channels management
	}

	//returning outputs
    if(channels)
        *channels = (int)n_active;

    return subvol;
}



