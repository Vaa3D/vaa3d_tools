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


UnstitchedVolume::UnstitchedVolume(const char* _root_dir)  throw (IOException)
: VirtualVolume(_root_dir) 
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

	volume = volumemanager::VirtualVolumeFactory::createFromXML(_root_dir,false);
	stitcher = new StackStitcher(volume);

	VXL_V = volume->getVXL_V();
	VXL_H = volume->getVXL_H();
	VXL_D = volume->getVXL_D();

	ORG_V = volume->getORG_V();
	ORG_H = volume->getORG_H();
	ORG_D = volume->getORG_D();

	DIM_C = volume->getDIM_C();
	BYTESxCHAN = volume->getBYTESxCHAN();

	if ( DIM_C > 3 || BYTESxCHAN > 2 )
 		throw iim::IOException(iom::strprintf("image not supported by UnstitchedVolume (DIM_C=%d, BYTESxCHAN=%d)", DIM_C, BYTESxCHAN), __iom__current__function__);

    active = (iim::uint32 *) new iim::uint32[DIM_C];
    n_active = DIM_C;
	for ( int i=0; i<DIM_C; i++ )
		active[i] = i;

	t0 = t1 = 0;
	DIM_T = 1;

 	stitcher->computeVolumeDims(false);

	DIM_V = stitcher->V1 - stitcher->V0;
	DIM_H = stitcher->H1 - stitcher->H0;
	DIM_D = stitcher->D1 - stitcher->D0;

	stripesCoords = new stripe_2Dcoords[volume->getN_ROWS()];
	stripesCorners = new stripe_2Dcorners[volume->getN_ROWS()];
}

UnstitchedVolume::~UnstitchedVolume(void)
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
	if ( volume ) // stitcher does not deallocate its volume
		delete volume;
	if ( stitcher )
		delete stitcher;
}


void UnstitchedVolume::init()
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);
}

void UnstitchedVolume::initChannels ( ) throw (IOException)
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);
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
	int vxl_i;

	row_start = 0;
	vxl_i = (int) floor( volume->getMEC_V() / volume->getVXL_V() );
	while ( vxl_i < V0 ) {
		row_start++;
		vxl_i += volume->getSTACKS()[row_start][0]->getHEIGHT();
	}
	row_end   = row_start;
	while ( vxl_i < V1 ) {
		row_end++;
		vxl_i += volume->getSTACKS()[row_end][0]->getHEIGHT();
	}

	col_start = 0;
	vxl_i = (int) floor( volume->getMEC_H() / volume->getVXL_H());
	while ( vxl_i < H0 ) {
		col_start++;
		vxl_i += volume->getSTACKS()[0][col_start]->getWIDTH();
	}
	col_end   = col_start;
	while ( vxl_i < H1 ) {
		col_end++;
		vxl_i += volume->getSTACKS()[0][col_end]->getWIDTH();
	}

	stitcher->computeVolumeDims(false,row_start,row_end,col_start,col_end,D0,D1);

	V0 = stitcher->V0;
	V1 = stitcher->V1;
	H0 = stitcher->H0;
	H1 = stitcher->H1;
	D0 = stitcher->D0;
	D1 = stitcher->D1;

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

	sint64 u_strp_bottom_displ;
	sint64 d_strp_top_displ;
	sint64 u_strp_top_displ;
	sint64 d_strp_left_displ;
	sint64 u_strp_left_displ;
	sint64 d_strp_width;
	sint64 u_strp_width;
	sint64 dd_strp_top_displ;
	sint64 u_strp_d_strp_overlap = 0; // WARNING: check how initialize
	sint64 h_up, h_down, h_overlap;
	iom::real_t *buffer_ptr, *ustripe_ptr, *dstripe_ptr;	

	iom::real_t* buffer;								//buffer temporary image data are stored
    iom::real_t* stripe_up=NULL, *stripe_down;                                   //will contain up-stripe and down-stripe computed by calling 'getStripe' method
	double angle;								//angle between 0 and PI used to sample overlapping zone in [0,PI]
	double delta_angle;							//angle step

	iom::real_t (*blending)(double& angle, iom::real_t& pixel1, iom::real_t& pixel2);
	int blending_algo = S_SINUSOIDAL_BLENDING;

	//retrieving blending function
	if(blending_algo == S_SINUSOIDAL_BLENDING)
        blending = StackStitcher::sinusoidal_blending;
	else if(blending_algo == S_NO_BLENDING)
        blending = StackStitcher::no_blending;
	else if(blending_algo == S_SHOW_STACK_MARGIN)
        blending = StackStitcher::stack_margin;
	else
 		throw iim::IOException(iom::strprintf("unrecognized blending function"), __iom__current__function__);

	buffer = new iom::real_t[height*width*depth];
	for (int i=0; i<height*width*depth; i++)
		buffer[i]=0;

	sint64 z = D0;

	for(sint64 k = 0; k < depth; k++)
	{
		//looping on all stripes
		for(int row_index=stitcher->ROW_START; row_index<=stitcher->ROW_END; row_index++)
		{
			//loading down stripe
			if(row_index==stitcher->ROW_START) stripe_up = NULL;
			stripe_down = stitcher->getStripe(row_index,(int)(z+k), restore_direction, stk_rst, blending_algo);

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

			//overlapping zone
			if(row_index!=stitcher->ROW_START)
			{	
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
						buffer_ptr  = &buffer[k*height*width+d_strp_top_displ*width+j];
						ustripe_ptr = &stripe_up[(d_strp_top_displ-u_strp_top_displ)*u_strp_width +j - u_strp_left_displ];
						for(sint64 i=d_strp_top_displ; i<d_strp_top_displ+h_up+(h_overlap >= 0 ?  0 : h_overlap); i++, buffer_ptr+=width, ustripe_ptr+= u_strp_width)
							*buffer_ptr = *ustripe_ptr;

						//OVERLAPPING zone
						buffer_ptr  = &buffer[k*height*width+(d_strp_top_displ+h_up)*width+j];
						ustripe_ptr = &stripe_up[(d_strp_top_displ+h_up-u_strp_top_displ)*u_strp_width +j - u_strp_left_displ];
						dstripe_ptr = &stripe_down[(d_strp_top_displ+h_up-d_strp_top_displ)*d_strp_width +j - d_strp_left_displ];
						for(sint64 i=d_strp_top_displ+h_up; i<d_strp_top_displ+h_up+h_overlap; i++, buffer_ptr+=width, ustripe_ptr+= u_strp_width, dstripe_ptr+=d_strp_width, angle+=delta_angle)
							*buffer_ptr = blending(angle,*ustripe_ptr,*dstripe_ptr);

						//DOWN stripe zone
						buffer_ptr = &buffer[k*height*width+(d_strp_top_displ+h_up+(h_overlap >= 0 ? h_overlap : 0))*width+j];
						dstripe_ptr = &stripe_down[((d_strp_top_displ+h_up+(h_overlap >= 0 ? h_overlap : 0))-d_strp_top_displ)*d_strp_width +j - d_strp_left_displ];
						for(sint64 i=d_strp_top_displ+h_up+(h_overlap >= 0 ? h_overlap : 0); i<d_strp_top_displ+h_up+h_overlap+h_down; i++, buffer_ptr+=width, dstripe_ptr+=d_strp_width)
							*buffer_ptr = *dstripe_ptr;
					}

					cnr_left = cnr_right;
					cnr_i++;
					if(cnr_i_next != stripesCorners[row_index-1].merged.end())
						cnr_i_next++;
				}
			}

			//non-overlapping zone
			buffer_ptr = &buffer[k*height*width+((row_index==stitcher->ROW_START ? 0 : u_strp_bottom_displ))*width];
			for(sint64 i= (row_index==stitcher->ROW_START ? 0 : u_strp_bottom_displ); i<(row_index==stitcher->ROW_END? height : dd_strp_top_displ); i++)
			{
				dstripe_ptr = &stripe_down[(i-d_strp_top_displ)*d_strp_width - d_strp_left_displ];
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

	VV0 = V0;
	VV1 = V1;
	HH0 = H0;
	HH1 = H1;
	DD0 = D0;
	DD1 = D1;

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

	sint64 stridex  = HH1 - HH0;
	sint64 stridexy = stridex * (VV1 - VV0);

	sint64 sbv_width = H1 - H0;
	sint64 sbv_height = V1 - V0;
	sint64 sbv_depth = D1 - D0;

    real32 *subvol = new real32[sbv_width * sbv_height * sbv_depth * DIM_C * BYTESxCHAN];

	int i, j, k;
	real32 *ptr_s_xy;
	real32 *ptr_s_x;
	real32 *ptr_s;
	real32  *ptr_d = subvol;
	for ( k=D0, ptr_s_xy=buf + (V0 - VV0)*stridex + (H0 - HH0); k<D1; k++, ptr_s_xy+=stridexy )
		for ( i=V0, ptr_s_x=ptr_s_xy; i<V1; i++, ptr_s_x+=stridex )
			for ( j=H0, ptr_s=ptr_s_x; j<H1; j++, ptr_d++, ptr_s++ )
				*ptr_d = *ptr_s;

	delete []buf;

	return subvol;

}

//loads given subvolume in a 1-D array of iim::uint8 while releasing stacks slices memory when they are no longer needed
//---03 nov 2011: added color support
iim::uint8* UnstitchedVolume::loadSubvolume_to_UINT8(int V0,int V1, int H0, int H1, int D0, int D1, int *channels, int ret_type ) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d, *channels=%d, ret_type=%d", V0, V1, H0, H1, D0, D1, channels ? *channels : -1, ret_type).c_str(), __iim__current__function__);

	int VV0, VV1, HH0, HH1, DD0, DD1;

	 //initializations
    V0 = V0 < 0 ? 0 : V0;
    H0 = H0 < 0 ? 0 : H0;
    D0 = D0 < 0 ? 0 : D0;
    V1 = (V1 < 0 || V1 > (int)DIM_V) ? DIM_V : V1; 
    H1 = (H1 < 0 || H1 > (int)DIM_H) ? DIM_H : H1; 
    D1 = (D1 < 0 || D1 > (int)DIM_D) ? DIM_D : D1; 

	if ( DIM_C > 1 ){ 
		// load the first channel
		iom::CHANS = iom::R;
	}

	real32 *buf = internal_loadSubvolume_to_real32(VV0, VV1, HH0, HH1, DD0, DD1, V0, V1, H0, H1, D0, D1); 

	if ( VV0 > V0 || HH0 > H0 || DD0 > D0 || VV1 < V1 || HH1 < H1 || DD1 < D1 )
  		throw iim::IOException(iom::strprintf("returned buffer is smaller than the requested subvolume (requested [V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d] -- returned [VV0=%d, VV1=%d, HH0=%d, HH1=%d, DD0=%d, DD1=%d])", 
												V0, V1, H0, H1, D0, D1, VV0, VV1, HH0, HH1, DD0, DD1), __iim__current__function__);

	// change when subvolumes are enabled
	sint64 stridex  = HH1 - HH0;
	sint64 stridexy = stridex * (VV1 - VV0);

	sint64 sbv_width = H1 - H0;
	sint64 sbv_height = V1 - V0;
	sint64 sbv_depth = D1 - D0;

    uint8 *subvol = new uint8[sbv_width * sbv_height * sbv_depth * DIM_C * BYTESxCHAN];

	int i, j, k, c;
	real32 *ptr_s_xy;
	real32 *ptr_s_x;
	real32 *ptr_s;
	if ( BYTESxCHAN == 1 ) {

		uint8  *ptr_d = subvol;
		for ( k=D0, ptr_s_xy=buf + (V0 - VV0)*stridex + (H0 - HH0); k<D1; k++, ptr_s_xy+=stridexy )
			for ( i=V0, ptr_s_x=ptr_s_xy; i<V1; i++, ptr_s_x+=stridex )
				for ( j=H0, ptr_s=ptr_s_x; j<H1; j++, ptr_d++, ptr_s++ )
					*ptr_d = uint8(*ptr_s * 255.0f);

		for ( c=1; c<DIM_C; c++ ) { // more than one channel
			delete []buf;
			iom::CHANS = c==1 ? iom::G : iom::B; // only two more channels are currently supported
			// loads next channel
			buf = internal_loadSubvolume_to_real32(VV0, VV1, HH0, HH1, DD0, DD1, V0, V1, H0, H1, D0, D1);
			// append next channel to the subvolume buffer
			for ( k=D0, ptr_s_xy=buf + (V0 - VV0)*stridex + (H0 - HH0); k<D1; k++, ptr_s_xy+=stridexy )
				for ( i=V0, ptr_s_x=ptr_s_xy; i<V1; i++, ptr_s_x+=stridex )
					for ( j=H0, ptr_s=ptr_s_x; j<H1; j++, ptr_d++, ptr_s++ )
						*ptr_d = uint8(*ptr_s * 255.0f);
		}
	}
	else if ( BYTESxCHAN == 2 ) {

		uint16  *ptr_d = (uint16 *) subvol;
		for ( k=D0, ptr_s_xy=buf + (V0 - VV0)*stridex + (H0 - HH0); k<D1; k++, ptr_s_xy+=stridexy )
			for ( i=V0, ptr_s_x=ptr_s_xy; i<V1; i++, ptr_s_x+=stridex )
				for ( j=H0, ptr_s=ptr_s_x; j<H1; j++, ptr_d++, ptr_s++ )
					*ptr_d = uint16(*ptr_s * 65535.0F);

		for ( c=1; c<DIM_C; c++ ) { // more than one channel
			delete []buf;
			iom::CHANS = c==1 ? iom::G : iom::B; // only two more channels are currently supported
			// loads next channel
			buf = internal_loadSubvolume_to_real32(VV0, VV1, HH0, HH1, DD0, DD1, V0, V1, H0, H1, D0, D1);
			// append next channel to the subvolume buffer
			for ( k=D0, ptr_s_xy=buf + (V0 - VV0)*stridex + (H0 - HH0); k<D1; k++, ptr_s_xy+=stridexy )
				for ( i=V0, ptr_s_x=ptr_s_xy; i<V1; i++, ptr_s_x+=stridex )
					for ( j=H0, ptr_s=ptr_s_x; j<H1; j++, ptr_d++, ptr_s++ )
						*ptr_d = uint16(*ptr_s * 65535.0F);
		}
	}
	else
  		throw iim::IOException(iom::strprintf("%d bits depth not currently supported", BYTESxCHAN*8), __iim__current__function__);

	delete []buf;

	*channels = DIM_C;

	return subvol;
}
		

