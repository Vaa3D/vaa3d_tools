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
* 2017-04-17. Giulio.          @ADDED check on the number of channels returned by internal volume
* 2016-04-27. Pierangelo.      @CREATED
*/

#include <iostream>
#include <string>
#include "MappedVolume.h"
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

// 2015-04-15. Alessandro. @ADDED definition for default constructor.
MappedVolume::MappedVolume(void) : VirtualVolume()
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);
    
    reference_system.first = reference_system.second = reference_system.third = iim::axis_invalid;
    VXL_1 = VXL_2 = VXL_3 = 0.0f;

    volume = 0;
    releaseVol = false;
    map    = 0;
}

MappedVolume::MappedVolume(const char* _root_dir, MappingViews * _map, bool overwrite_mdata, bool save_mdata)  throw (IOException)
: VirtualVolume(_root_dir) // iannello ADDED
{
    /**/iim::debug(iim::LEV3, strprintf("_root_dir=%s",_root_dir).c_str(), __iim__current__function__);

    map    = _map;

    volume = VirtualVolume::instance(root_dir);
    releaseVol = true;

	init();

	initChannels();
}

MappedVolume::MappedVolume(VirtualVolume *_vol, MappingViews * _map, bool overwrite_mdata, bool save_mdata)  throw (IOException)
: VirtualVolume(_vol->getROOT_DIR()) // iannello ADDED
{
    /**/iim::debug(iim::LEV3, strprintf("_root_dir=%s",_vol->getROOT_DIR()).c_str(), __iim__current__function__);

    map    = _map;

    volume = _vol;
    releaseVol = false;

	init();

	initChannels();
}

MappedVolume::~MappedVolume(void) throw (iim::IOException)
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

    if ( releaseVol && volume )
    	delete volume;
	map = 0; // map is always passed from outside
}


void MappedVolume::init() throw (IOException)
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

	/* reference system of volumes is always (V,H,D) but versors can be positive or negative
	 * the reference system of the festure volume is inherited
	 */
	reference_system.first  = (volume->getVXL_1() > 0) ? iim::axis(1) : iim::axis(-1);
	reference_system.second = (volume->getVXL_2() > 0) ? iim::axis(2) : iim::axis(-2);
	reference_system.third  = (volume->getVXL_3() > 0) ? iim::axis(3) : iim::axis(-3);;

	/* the voxel must be the same of the feature volume
	 */
	VXL_V = VXL_1 = volume->getVXL_1();
	VXL_H = VXL_2 = volume->getVXL_2();
	VXL_D = VXL_3 = volume->getVXL_3();
	
	/* the volume sizes are mapped from feature to target to compute the correct sizes of the mapped volume
	 */
	map->mapdims(volume->getDIM_V(),volume->getDIM_H(),volume->getDIM_D(),(int *) (&DIM_V),(int *) (&DIM_H),(int *) (&DIM_D));

	Point dummy;
	map->map(Point(volume->getORG_V(),volume->getORG_H()+volume->getDIM_H()*volume->getVXL_H(),volume->getORG_D()+volume->getDIM_D()*volume->getVXL_D()),&dummy);
	ORG_V = (float) dummy.y();
	ORG_H = (float) dummy.x();
	ORG_D = (float) dummy.z();
}


void MappedVolume::initChannels ( ) throw (IOException)
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

    DIM_C = volume->getDIM_C();
	BYTESxCHAN = volume->getBYTESxCHAN();

    n_active = DIM_C;
    active = new iim::uint32[n_active];
    for ( int c=0; c<DIM_C; c++ )
        active[c] = c; // all channels are assumed active
}

//PRINT method
void MappedVolume::print( bool print_stacks )
{
	printf("*** Begin printing MappedVolume object...\n\n");
	printf("\tDirectory:\t%s\n", root_dir);
	printf("\tDimensions:\t\t%d(V) x %d(H) x %d(D)\n", DIM_V, DIM_H, DIM_D);
	printf("\tVoxels:\t\t\t%.4f(V) x %.4f(H) x %.4f(D)\n", VXL_V, VXL_H, VXL_D);
	printf("\tOrigin:\t\t\t%.4f(V) x %.4f(H) x %.4f(D)\n", ORG_V, ORG_H, ORG_D);
	printf("\tChannels:\t\t%d\n", DIM_C);
	printf("\tBytes per channel:\t%d\n", BYTESxCHAN);
	//printf("\tReference system:\tref1=%d, ref2=%d, ref3=%d\n",reference_system.first,reference_system.second,reference_system.third);
	printf("\n*** END printing MappedVolume object...\n\n");
}

//rotate stacks matrix around D axis (accepted values are theta=0,90,180,270)
void MappedVolume::rotate(int theta)
{
    /**/iim::debug(iim::LEV3, strprintf("theta=%d", theta).c_str(), __iim__current__function__);

}

//mirror stacks matrix along mrr_axis (accepted values are mrr_axis=1,2,3)
void MappedVolume::mirror(axis mrr_axis)
{
    /**/iim::debug(iim::LEV3, strprintf("mrr_axis=%d", mrr_axis).c_str(), __iim__current__function__);

}

//extract spatial coordinates (in millimeters) of given Stack object
void MappedVolume::extractCoordinates(Block* blk, int z, int* crd_1, int* crd_2, int* crd_3)
{
    bool found_ABS_X=false;
    bool found_ABS_Y=false;

    //loading estimations for absolute X and Y stack positions
    char * pch;
    char buffer[100];
    strcpy(buffer,&(blk->getDIR_NAME()[0]));
    pch = strtok (buffer,"/_");
    pch = strtok (NULL, "/_");

    while (pch != NULL)
    {
        if(!found_ABS_X)
        {
            if(sscanf(pch, "%d", crd_1) == 1)
                found_ABS_X=true;
        }
        else if(!found_ABS_Y)
        {
            if(sscanf(pch, "%d", crd_2) == 1)
                found_ABS_Y=true;
        }
        else
            break;

        pch = strtok (NULL, "/_");
    }

    if(!found_ABS_X || !found_ABS_Y)
    {
        char msg[200];
        sprintf(msg,"in MappedVolume::extractCoordinates(directory_name=\"%s\"): format 000000_000000 or X_000000_X_000000 not found", blk->getDIR_NAME());
        throw msg;
    }

    //loading estimation for absolute Z stack position
    if(crd_3!= NULL)
    {
        char* first_file_name = blk->getFILENAMES()[z];

        char * pch;
        char lastTokenized[100];
        char buffer[500];
        strcpy(buffer,&(first_file_name[0]));

        pch = strtok (buffer,"_");
        while (pch != NULL)
        {
            strcpy(lastTokenized,pch);
            pch = strtok (NULL, "_");
        }

        pch = strtok (lastTokenized,".");
        strcpy(lastTokenized,pch);

        if(sscanf(lastTokenized, "%d", crd_3) != 1)
        {
            char msg[200];
            sprintf(msg,"in MappedVolume::extractCoordinates(...): unable to extract Z position from filename %s", first_file_name);
            throw msg;
        }
    }
}

//loads given subvolume in a 1-D array of float, (V0,H0,D0) and (V1,H1,D1) identify the transformed volume (target volume)
real32* MappedVolume::loadSubvolume_to_real32(int V0,int V1, int H0, int H1, int D0, int D1) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d", V0, V1, H0, H1, D0, D1).c_str(), __iim__current__function__);

	//int channels;
	//float scale_factor;

    // channels check
	if (DIM_C == 2 || DIM_C > 3) // only monocromatic or RGB images are supported
	{
		char errMsg[2000];
		sprintf(errMsg, "in MappedVolume::loadSubvolume(...): %d channels are not supported.", DIM_C);
		throw IOException(errMsg);
	}

	// boundary check
    V0 = V0 < 0 ? 0 : V0;
    H0 = H0 < 0 ? 0 : H0;
    D0 = D0 < 0 ? 0 : D0;
    V1 = (V1 < 0 || V1 > (int)DIM_V) ? DIM_V : V1; // iannello MODIFIED
    H1 = (H1 < 0 || H1 > (int)DIM_H) ? DIM_H : H1; // iannello MODIFIED
    D1 = (D1 < 0 || D1 > (int)DIM_D) ? DIM_D : D1; // iannello MODIFIED

	// conversion from indices to real spatial coordinates of the target volume vertices
	COORDSTYPE x0, y0, z0, x1, y1, z1;
	y0 = ind2coord_V(V0);
	x0 = ind2coord_H(H0);
	z0 = ind2coord_D(D0);
	y1 = ind2coord_V(V1);
	x1 = ind2coord_H(H1);
	z1 = ind2coord_D(D1);

	Point p0, p1; // declare feature volume vertices
	// map back the target volume vertices to the feature volume vertices (in real coordinates)
	map->mapback(Point(y0,x0,z0),&p0);
	map->mapback(Point(y1,x1,z1),&p1);

	// reusing V0, ... , D1
	// conversion from real coordinates to indices of the feature volume vertices
	V0 = volume->coord2ind_V((float)p0.y());
	H0 = volume->coord2ind_H((float)p0.x());
	D0 = volume->coord2ind_D((float)p0.z());
	V1 = volume->coord2ind_V((float)p1.y());
	H1 = volume->coord2ind_H((float)p1.x());
	D1 = volume->coord2ind_D((float)p1.z());

	rearrange_indices(V0,V1);
	rearrange_indices(H0,H1);
	rearrange_indices(D0,D1);

	// NOTE: buffers are filled with volume data in the order H first, then V, then D
	real32 *tempbuf = volume->loadSubvolume_to_real32(V0,V1,H0,H1,D0,D1); // buffer containing feature volume

	real32 *subvol = map->mapbuffer(tempbuf,(V1-V0),(H1-H0),(D1-D0));  // buffer containing target volume

	delete tempbuf;

	return subvol;
}

//loads given subvolume in a 1-D array of iim::uint8 while releasing stacks slices memory when they are no longer needed
//---03 nov 2011: added color support
iim::uint8* MappedVolume::loadSubvolume_to_UINT8(int V0,int V1, int H0, int H1, int D0, int D1, int *channels, int ret_type ) throw (IOException, iom::exception)
{
    /**/iim::debug(iim::LEV3, strprintf("V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d, *channels=%d, ret_type=%d", V0, V1, H0, H1, D0, D1, channels ? *channels : -1, ret_type).c_str(), __iim__current__function__);

    if ( (ret_type != iim::NATIVE_RTYPE) && (ret_type != iim::DEF_IMG_DEPTH) ) {
		// return type should be converted, but not to 8 bits per channel
        char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"MappedVolume::loadSubvolume_to_UINT8: non supported return type (%d bits) - native type is %d bits",ret_type, 8*this->BYTESxCHAN);
        throw IOException(err_msg);
	}

	// reduction factor to be applied to the loaded buffer
    int red_factor = (ret_type == iim::NATIVE_RTYPE) ? 1 : ((8 * this->BYTESxCHAN) / ret_type);

    //initializations
    V0 = V0 < 0 ? 0 : V0;
    H0 = H0 < 0 ? 0 : H0;
    D0 = D0 < 0 ? 0 : D0;
    V1 = (V1 < 0 || V1 > (int)DIM_V) ? DIM_V : V1; // iannello MODIFIED
    H1 = (H1 < 0 || H1 > (int)DIM_H) ? DIM_H : H1; // iannello MODIFIED
    D1 = (D1 < 0 || D1 > (int)DIM_D) ? DIM_D : D1; // iannello MODIFIED

    iim::uint8 *subvol = 0;

    //checking that the interval is valid
    if(V1-V0 <=0 || H1-H0 <= 0 || D1-D0 <= 0)
        throw IOException("in MappedVolume::loadSubvolume_to_UINT8: invalid subvolume intervals");

	// assume that all channels of volume are active
	// first set as active the channels of this
	iim::uint32 *set_active = new iim::uint32[n_active]; 
	for ( int c=0; c<n_active; c++ )
		set_active[c] = c;
	volume->setActiveChannels(set_active,n_active); // property of set_active is passed to volume

	// conversion from indices to real spatial coordinates of the target volume vertices
	COORDSTYPE x0, y0, z0, x1, y1, z1;
	y0 = ind2coord_V(V0);
	x0 = ind2coord_H(H0);
	z0 = ind2coord_D(D0);
	y1 = ind2coord_V(V1);
	x1 = ind2coord_H(H1);
	z1 = ind2coord_D(D1);

	Point p0, p1; // declare feature volume vertices
	// map back the target volume vertices to the feature volume vertices (in real coordinates)
	map->mapback(Point(y0,x0,z0),&p0);
	map->mapback(Point(y1,x1,z1),&p1);

	// reusing V0, ... , D1
	// conversion from real coordinates to indices of the feature volume vertices
	V0 = volume->coord2ind_V((float)p0.y());
	H0 = volume->coord2ind_H((float)p0.x());
	D0 = volume->coord2ind_D((float)p0.z());
	V1 = volume->coord2ind_V((float)p1.y());
	H1 = volume->coord2ind_H((float)p1.x());
	D1 = volume->coord2ind_D((float)p1.z());

	rearrange_indices(V0,V1);
	rearrange_indices(H0,H1);
	rearrange_indices(D0,D1);

	// NOTE: buffers are filled with volume data in the order H first, then V, then D
	iim::uint8 *tempbuf = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,D0,D1,channels,ret_type); // buffer containing feature volume
	
	if ( n_active != *channels ) {
		char msg[STATIC_STRINGS_SIZE];
		sprintf(msg,"in MappedVolume::loadSubvolume_to_UINT8(...): mismatch between requested channels (%d) and returned channels from internal volume (%d)",n_active,*channels);
		throw IOException(msg);
	}

	// reset all channels of volume as active
	set_active = new iim::uint32[DIM_C];
	for ( int c=0; c<DIM_C; c++ )
		set_active[c] = c;
	volume->setActiveChannels(set_active,DIM_C); // property of set_active is passed to volume

	// map all active channels of subvolume 
	subvol = map->mapbuffer(tempbuf, (V1-V0), (H1-H0), (D1-D0), n_active, BYTESxCHAN);  // buffer containing target volume

	delete tempbuf;

	if ( channels ) 
		*channels = DIM_C; 

	return subvol;
}
		

//releases allocated memory of stacks
void MappedVolume::releaseStacks(int first_file, int last_file)
{
    /**/iim::debug(iim::LEV3, strprintf("first_file=%d, last_file=%d", first_file, last_file).c_str(), __iim__current__function__);

    char msg[STATIC_STRINGS_SIZE];
	sprintf(msg,"in MappedVolume::releaseStacks: not implemented yet");
    throw IOException(msg);

	first_file = (first_file == -1 ? 0		: first_file);
	last_file  = (last_file  == -1 ? DIM_D	: last_file);
	//for(int row_index=0; row_index<N_ROWS; row_index++)
	//	for(int col_index=0; col_index<N_COLS; col_index++)
	//		STACKS[row_index][col_index]->releaseStack(first_file,last_file);
}

