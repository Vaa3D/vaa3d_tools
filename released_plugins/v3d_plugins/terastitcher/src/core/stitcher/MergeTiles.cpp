//------------------------------------------------------------------------------------------------
// Copyright (c) 2013  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).  
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
*
*       Bria, A., Iannello, G., "TeraStitcher - A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Microscopy Images", (2012) BMC Bioinformatics, 13 (1), art. no. 316.
*
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
* 2014-12-06. Giulio    . @ADDED par_mode parameter in method mergeTilesVaa3DRaw controlling the execution when multiple instances of the function are launched.
* 2014-12-06. Giulio    . @ADDED createDirectoryHiererchy method.
* 2014-11-03. Giulio.     @FIXED stop and resume facility should be inactive when in test mode
* 2014-10-29. Giulio.     @ADDED stop and resume facility - saved_img_format has been used to check if resume parameters have not been changed
* 2014-09-10. Alessandro. @FIXED 'mergeTilesVaa3DRaw' method: set 'imagemanager' module to silent mode.
* 2014-09-10. Alessandro. @CHANGED 'saved_img_format' interpretation in 'mergeTilesVaa3DRaw()' method.
* 2014-09-09. Alessandro. @FIXED missing buffer initialization and reset in 'mergeTiles()' method.
*/

# include "StackStitcher.h"

#include <iostream>
#include <string>
#include <sstream>
#include <limits>
#include <list>
#include <ctime>
#include "vmVirtualStack.h"
#include "S_config.h"
#include "StackRestorer.h"
#include "volumemanager.config.h"

#include "RawFmtMngr.h"
#include "Tiff3DMngr.h"
#include "../imagemanager/IM_config.h"
#include "../imagemanager/VirtualVolume.h"
#include "../imagemanager/TiledVolume.h"


#include "../iomanager/ProgressBar.h"

#include "resumer.h" // GI_141029: added stop and resume facility

using namespace iomanager;
using namespace volumemanager;

struct coord_2D{int V,H;};
struct stripe_2Dcoords{coord_2D up_left, bottom_right;};
struct stripe_corner{int h,H; bool up;};
struct stripe_2Dcorners{std::list<stripe_corner> ups, bottoms, merged;};
bool compareCorners (stripe_corner first, stripe_corner second);

//typedef long V3DLONG;


void StackStitcher::mergeTilesVaa3DRaw(std::string output_path, int block_height, int block_width, int block_depth, bool* resolutions, 
							   bool exclude_nonstitchable_stacks, int _ROW_START, int _ROW_END, int _COL_START,
							   int _COL_END, int _D0, int _D1, bool restoreSPIM, int restore_direction,
							   int blending_algo, bool test_mode, bool show_progress_bar, 
                               const char* saved_img_format, int saved_img_depth, bool par_mode)			throw (iom::exception)
{
#if S_VERBOSE > 2
	printf("......in StackStitcher::mergeTilesVaa3DRaw(output_path=\"%s\", block_height=%d, block_width=%d, block_depth=%d, exclude_nonstitchable_stacks = %s, "
		"_ROW_START=%d, _ROW_END=%d, _COL_START=%d, _COL_END=%d, _D0=%d, _D1=%d, restoreSPIM = %s, restore_direction = %d, test_mode = %s, resolutions = { ",
		output_path.c_str(), slice_height, slice_width, slice_depth, (exclude_nonstitchable_stacks ? "true" : "false"), _ROW_START, _ROW_END,
		_COL_START, _COL_END, _D0, _D1, (restoreSPIM ? "ENABLED" : "disabled"), restore_direction, (test_mode ? "ENABLED" : "disabled"));
	for(int i=0; i<S_MAX_MULTIRES && resolutions; i++)
		printf("%d ", resolutions[i]);
	printf("}\n");
#endif

	//LOCAL VARIABLES
	sint64 height, width, depth;                                            //height, width and depth of the whole volume that covers all stacks
	iom::real_t* buffer;								//buffer temporary image data are stored
	iom::real_t* stripe_up=NULL, *stripe_down;                                   //will contain up-stripe and down-stripe computed by calling 'getStripe' method
	double angle;								//angle between 0 and PI used to sample overlapping zone in [0,PI]
	double delta_angle;							//angle step
	int z_ratio, z_max_res;
    int n_stacks_V[S_MAX_MULTIRES], n_stacks_H[S_MAX_MULTIRES], n_stacks_D[S_MAX_MULTIRES];             //array of number of tiles along V and H directions respectively at i-th resolution
    int ***stacks_height[S_MAX_MULTIRES], ***stacks_width[S_MAX_MULTIRES], ***stacks_depth[S_MAX_MULTIRES];	//array of matrices of tiles dimensions at i-th resolution
	sint64 u_strp_bottom_displ;
	sint64 d_strp_top_displ;
	sint64 u_strp_top_displ;
	sint64 d_strp_left_displ;
	sint64 u_strp_left_displ;
	sint64 d_strp_width;
	sint64 u_strp_width;
	sint64 dd_strp_top_displ;
	sint64 u_strp_d_strp_overlap;
	sint64 h_up, h_down, h_overlap;
	stripe_2Dcoords  *stripesCoords;
	stripe_2Dcorners *stripesCorners;
	int resolutions_size = 0;
	StackRestorer *stk_rst = NULL;
	iom::real_t *buffer_ptr, *ustripe_ptr, *dstripe_ptr;	
	iom::real_t (*blending)(double& angle, iom::real_t& pixel1, iom::real_t& pixel2);

	std::stringstream file_path[S_MAX_MULTIRES];

				/* DEFINITIONS OF VARIABILES THAT MANAGE TILES (BLOCKS) ALONG D-direction
	 * In the following the term 'group' means the groups of slices that are 
	 * processed together to generate slices of all resolution requested
	 */

	/* stack_block[i] is the index of current block along z (it depends on the resolution i)
	 * current block is the block in which falls the first slice of the group
	 * of slices that is currently being processed, i.e. from z to z+z_max_res-1
	 */
    int stack_block[S_MAX_MULTIRES];

	/* these arrays are the indices of first and last slice of current block at resolution i
	 * WARNING: the slice index refers to the index of the slice in the volume at resolution i 
	 */
    int slice_start[S_MAX_MULTIRES];
    int slice_end[S_MAX_MULTIRES];

	/* the number of slice of already processed groups at current resolution
	 * the index of the slice to be saved at current resolution is:
	 *
	 *      n_slices_pred + z_buffer
	 */
	sint64 n_slices_pred;       

	//retrieving blending function
	if(blending_algo == S_SINUSOIDAL_BLENDING)
            blending = sinusoidal_blending;
	else if(blending_algo == S_NO_BLENDING)
            blending = no_blending;
	else if(blending_algo == S_SHOW_STACK_MARGIN)
            blending = stack_margin;
	else
            throw iom::exception("in StackStitcher::getStripe(...): unrecognized blending function");

	//initializing the progress bar
	char progressBarMsg[200];

	if ( par_mode ) // in parallel mode never show the progress bar
		show_progress_bar = false;

	if(show_progress_bar)
	{
		ProgressBar::instance()->start("Multiresolution tile merging");
		ProgressBar::instance()->update(0,"Initializing...");
		ProgressBar::instance()->show();
	}

	//initializing <StackRestorer> object if restoring is enabled
	if(restoreSPIM)
	{
            char desc_file_path[S_STATIC_STRINGS_SIZE];
            sprintf(desc_file_path, "%s/%s", volume->getSTACKS_DIR(), S_DESC_FILE_NAME);
            if(volumemanager::VirtualVolume::fileExists(desc_file_path) && restore_direction != axis_invalid)
                stk_rst = new StackRestorer(this->volume, desc_file_path);
            else if (!(volumemanager::VirtualVolume::fileExists(desc_file_path)) && restore_direction != axis_invalid)
                printf("\n\nWARNING! Restoring is enabled but can't find %s file at %s.\n\t--> Restoring has been DISABLED.\n\n", S_DESC_FILE_NAME, desc_file_path);
	}

	//computing dimensions of volume to be stitched
	this->computeVolumeDims(exclude_nonstitchable_stacks, _ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
	width = this->H1-this->H0;
	height = this->V1-this->V0;
	depth = this->D1-this->D0;

	//activating resolutions
    block_height = (block_height == -1 ? (int)height : block_height);
    block_width  = (block_width  == -1 ? (int)width  : block_width);
    block_depth  = (block_depth  == -1 ? (int)depth  : block_depth);
    if(block_height < S_MIN_SLICE_DIM || block_width < S_MIN_SLICE_DIM /* 2014-10-29. Giulio. @REMOVED (|| block_depth < S_MIN_SLICE_DIM) */)
    {
        char err_msg[5000];
        sprintf(err_msg,"The minimum dimension for block width, height and depth is %d", S_MIN_SLICE_DIM);
        throw iom::exception(err_msg);
    }
	if(resolutions == NULL)
	{
        resolutions = new bool;
        *resolutions = true;
        resolutions_size = 1;
	}
	else
        for(int i=0; i<S_MAX_MULTIRES; i++)
            if(resolutions[i])
                resolutions_size = ISR_MAX(resolutions_size, i+1);

	//computing tiles dimensions at each resolution and initializing volume directories
	for(int res_i=0; res_i< resolutions_size; res_i++)
	{
        n_stacks_V[res_i] = static_cast<int>(ceil ( (height/POW_INT(2,res_i)) / (float) block_height ));
        n_stacks_H[res_i] = static_cast<int>(ceil ( (width/POW_INT(2,res_i))  / (float) block_width  ));
        n_stacks_D[res_i] = static_cast<int>(ceil ( (depth/POW_INT(2,res_i))  / (float) block_depth  ));
        stacks_height[res_i] = new int **[n_stacks_V[res_i]];
        stacks_width[res_i]  = new int **[n_stacks_V[res_i]];
        stacks_depth[res_i]  = new int **[n_stacks_V[res_i]];
        for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
        {
            stacks_height[res_i][stack_row] = new int *[n_stacks_H[res_i]];
            stacks_width [res_i][stack_row] = new int *[n_stacks_H[res_i]];
            stacks_depth [res_i][stack_row] = new int *[n_stacks_H[res_i]];
            for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
            {
				stacks_height[res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
				stacks_width [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
				stacks_depth [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
				for(int stack_sli = 0; stack_sli < n_stacks_D[res_i]; stack_sli++)
				{
                	stacks_height[res_i][stack_row][stack_col][stack_sli] = 
                		((int)(height/POW_INT(2,res_i))) / n_stacks_V[res_i] + (stack_row < ((int)(height/POW_INT(2,res_i))) % n_stacks_V[res_i] ? 1:0);
                	stacks_width [res_i][stack_row][stack_col][stack_sli] = 
                		((int)(width/POW_INT(2,res_i)))  / n_stacks_H[res_i] + (stack_col < ((int)(width/POW_INT(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
					stacks_depth[res_i][stack_row][stack_col][stack_sli] = 
                		((int)(depth/POW_INT(2,res_i)))  / n_stacks_D[res_i] + (stack_sli < ((int)(depth/POW_INT(2,res_i)))  % n_stacks_D[res_i] ? 1:0);
				}
            }
        }
        //creating volume directory iff current resolution is selected and test mode is disabled
        if(resolutions[res_i] == true && !test_mode)
        {
			if ( par_mode ) { // uses all slices to generate the directory name
				file_path[res_i]<<output_path<<"/RES("<<height/POW_INT(2,res_i)<<"x"<<width/POW_INT(2,res_i)<<"x"<<(volume->getN_SLICES())/POW_INT(2,res_i)<<")";
			}
			else { 
				//creating directory that will contain image data at current resolution
				file_path[res_i]<<output_path<<"/RES("<<height/POW_INT(2,res_i)<<"x"<<width/POW_INT(2,res_i)<<"x"<<depth/POW_INT(2,res_i)<<")";
				if(!make_dir(file_path[res_i].str().c_str()))
				{
					char err_msg[S_STATIC_STRINGS_SIZE];
					sprintf(err_msg, "in mergeTiles(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
					throw iom::exception(err_msg);
				}

				//Alessandro - 23/03/2013: saving original volume XML descriptor into each folder
				char xmlPath[S_STATIC_STRINGS_SIZE];
				sprintf(xmlPath, "%s/original_volume_desc.xml", file_path[res_i].str().c_str());
				volume->saveXML(0, xmlPath);
			}
        }
	}

	//computing VH coordinates of all stripes
	stripesCoords = new stripe_2Dcoords[volume->getN_ROWS()];
	for(int row_index=ROW_START; row_index<=ROW_END; row_index++)
	{
        stripesCoords[row_index].up_left.V		= getStripeABS_V(row_index,true);
        stripesCoords[row_index].up_left.H      = volume->getSTACKS()[row_index][COL_START]->getABS_H();
        stripesCoords[row_index].bottom_right.V = getStripeABS_V(row_index,false);
        stripesCoords[row_index].bottom_right.H = volume->getSTACKS()[row_index][COL_END]->getABS_H()+volume->getStacksWidth();
	}

	//computing stripes corners, i.e. corners that result from the overlap between each pair of adjacent stripes
	stripesCorners = new stripe_2Dcorners[volume->getN_ROWS()];
	for(int row_index=ROW_START; row_index<=ROW_END; row_index++)
	{
		stripe_corner tmp;

		//for first VirtualStack of every stripe
		tmp.H = volume->getSTACKS()[row_index][COL_START]->getABS_H();
		tmp.h = volume->getSTACKS()[row_index][COL_START]->getABS_V()-stripesCoords[row_index].up_left.V;
		tmp.up = true;
		stripesCorners[row_index].ups.push_back(tmp);
		
		tmp.h = stripesCoords[row_index].bottom_right.V - volume->getSTACKS()[row_index][COL_START]->getABS_V() - volume->getStacksHeight();
		tmp.up = false;
		stripesCorners[row_index].bottoms.push_back(tmp);

		for(int col_index=COL_START; col_index<COL_END; col_index++)
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
		tmp.H = volume->getSTACKS()[row_index][COL_END]->getABS_H() + volume->getStacksWidth();
		tmp.up = true;
		stripesCorners[row_index].ups.push_back(tmp);

		tmp.up = false;
		stripesCorners[row_index].bottoms.push_back(tmp);
	}

	//ordered merging between ups and bottoms corners for every stripe
	for(int row_index = ROW_START+1; row_index<=ROW_END; row_index++)
	{
		stripesCorners[row_index-1].merged.merge(stripesCorners[row_index-1].bottoms,   compareCorners);
		stripesCorners[row_index-1].merged.merge(stripesCorners[row_index  ].ups,       compareCorners);
	}

	z_max_res = POW_INT(2,resolutions_size-1);
	z_ratio= static_cast<int>(depth/z_max_res);
	buffer = new iom::real_t[height*width*z_max_res];

	// 2014-10-29. Giulio. @DELETED 
	////slice_start and slice_end of current block depend on the resolution
	//for(int res_i=0; res_i< resolutions_size; res_i++) {
	//	stack_block[res_i] = 0;
	//	//slice_start[res_i] = this->D0; 
	//	slice_start[res_i] = 0; // indices must start from 0 because they should have relative meaning 
	//	slice_end[res_i] = slice_start[res_i] + stacks_depth[res_i][0][0][0] - 1;
	//}

	// 2014-10-29. Giulio. @ADDED stop and resume facility
	FILE *fhandle;
	sint64 z;
	sint64 z_parts;

	// 2014-11-03. Giulio. @FIXED stop and resume facility should inactive in test mode
	if ( !test_mode ) {
		// WARNING: uses saved_img_format to check that the operation has been resumed withe the sae parameters
		// resume option not used in parallel mode
		if ( !par_mode && initResumer(saved_img_format,output_path.c_str(),resolutions_size,resolutions,block_height,block_width,block_depth,HALVE_BY_MEAN,saved_img_format,saved_img_depth,fhandle) ) {
			readResumerState(fhandle,output_path.c_str(),resolutions_size,stack_block,slice_start,slice_end,z,z_parts);
		}
		else {
			//slice_start and slice_end of current block depend on the resolution
			for(int res_i=0; res_i< resolutions_size; res_i++) {
				stack_block[res_i] = 0;
				slice_start[res_i] = 0; // indices must start from 0 because they should have relative meaning 
				slice_end[res_i] = slice_start[res_i] + stacks_depth[res_i][0][0][0] - 1;
			}
			// z must begin from D0 (absolute index into the volume) since it is used to compute tha file names (containing the absolute position along D)
			z = this->D0;
			z_parts = 1;
		}
	}
	else { // test mode
		z = this->D0;
		z_parts = 1;
	}

	#ifdef S_TIME_CALC
	double proc_time;
	#endif

	// z must begin from D0 (absolute index into the volume) since it is used to compute tha file names (containing the absolute position along D)
	for( /* 2014-10-29. Giulio. @DELETED (sint64 z = this->D0, z_parts = 1) */; z < this->D1; z += z_max_res, z_parts++)
	{
		// 2014-09-09. Alessandro. @FIXED missing buffer initialization and reset in 'mergeTiles()' method.
		for(int i=0; i<height*width*z_max_res; i++)
			buffer[i]=0;

		for(sint64 k = 0; k < ( z_parts <= z_ratio ? z_max_res : depth%z_max_res ); k++)
		{
			//updating the progress bar
			if(show_progress_bar)
			{	
				sprintf(progressBarMsg, "Merging slice %d of %d",((uint32)(z-D0+k+1)),(uint32)depth);
                                ProgressBar::instance()->update(((float)(z-D0+k+1)*100/(float)depth), progressBarMsg);
                                ProgressBar::instance()->show();
			}

			//looping on all stripes
			for(int row_index=ROW_START; row_index<=ROW_END; row_index++)
			{
				//loading down stripe
				if(row_index==ROW_START) stripe_up = NULL;
				if ( z+k == 250 ) {
					int a = 0;
				}
				stripe_down = this->getStripe(row_index,(int)(z+k), restore_direction, stk_rst, blending_algo);

				#ifdef S_TIME_CALC
				proc_time = -TIME(0);
				#endif

				if(stripe_up) u_strp_bottom_displ	= stripesCoords[row_index-1].bottom_right.V	 - V0;
				d_strp_top_displ					= stripesCoords[row_index  ].up_left.V	     - V0;
				if(stripe_up) u_strp_top_displ      = stripesCoords[row_index-1].up_left.V	     - V0;
				d_strp_left_displ					= stripesCoords[row_index  ].up_left.H		 - H0;
				if(stripe_up) u_strp_left_displ     = stripesCoords[row_index-1].up_left.H		 - H0;
				d_strp_width						= stripesCoords[row_index  ].bottom_right.H - stripesCoords[row_index  ].up_left.H;
				if(stripe_up) u_strp_width			= stripesCoords[row_index-1].bottom_right.H - stripesCoords[row_index-1].up_left.H;
				if(stripe_up) u_strp_d_strp_overlap = u_strp_bottom_displ - d_strp_top_displ;
				if(row_index!=ROW_END) 
					dd_strp_top_displ				= stripesCoords[row_index+1].up_left.V		 - V0;
				h_up =  h_down						= u_strp_d_strp_overlap;

				//overlapping zone
				if(row_index!=ROW_START)
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
				buffer_ptr = &buffer[k*height*width+((row_index==ROW_START ? 0 : u_strp_bottom_displ))*width];
				for(sint64 i= (row_index==ROW_START ? 0 : u_strp_bottom_displ); i<(row_index==ROW_END? height : dd_strp_top_displ); i++)
				{
					dstripe_ptr = &stripe_down[(i-d_strp_top_displ)*d_strp_width - d_strp_left_displ];
					for(sint64 j=0; j<width; j++, buffer_ptr++, dstripe_ptr++)
						if(j - d_strp_left_displ >= 0 && j - d_strp_left_displ < stripesCoords[row_index].bottom_right.H)
							*buffer_ptr = *dstripe_ptr;
				}

				//moving to bottom stripe_up
				delete stripe_up;
				stripe_up=stripe_down;

				#ifdef S_TIME_CALC
				proc_time += TIME(0);
				StackStitcher::time_merging+=proc_time;
				#endif
			}
			//releasing last stripe_down
			delete stripe_down;
		}
		
		//saving current buffer data at selected resolutions and in multitile format
		for(int i=0; i< resolutions_size; i++)
		{
			if(show_progress_bar)
			{
				sprintf(progressBarMsg, "Generating resolution %d of %d",i+1,ISR_MAX(resolutions_size, resolutions_size));
                                ProgressBar::instance()->updateInfo(progressBarMsg);
                                ProgressBar::instance()->show();
			}

			// check if current block is changed
			// D0 must be subtracted because z is an absolute index in volume while slice index should be computed on a relative basis (i.e. starting form 0)
            if ( ((z - this->D0) / POW_INT(2,i)) > slice_end[i] ) {
				stack_block[i]++;
				slice_start[i] = slice_end[i] + 1;
				slice_end[i] += stacks_depth[i][0][0][stack_block[i]];
			}

			// find abs_pos_z at resolution i
			std::stringstream abs_pos_z;
			abs_pos_z.width(6);
			abs_pos_z.fill('0');
			abs_pos_z << (int)(this->getMultiresABS_D(i,0) + // all stacks start at the same D position
								//- D0 * volume->getVXL_D() * 10 + // WARNING: D0 is counted twice,both in getMultiresABS_D and in slice_start
                                (POW_INT(2,i)*slice_start[i]) * volume->getVXL_D() * 10);

			//compute the number of slice of previous groups at resolution i
			//note that z_parts in the number and not an index (starts from 1)
            n_slices_pred  = (z_parts - 1) * z_max_res / POW_INT(2,i);

			//buffer size along D is different when the remainder of the subdivision by z_max_res is considered
			sint64 z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

			//halvesampling resolution if current resolution is not the deepest one
			if(i!=0)	
				StackStitcher::halveSample(buffer,(int)(height/(POW_INT(2,i-1))),(int)(width/(POW_INT(2,i-1))),(int)(z_size/(POW_INT(2,i-1))));

			//saving at current resolution if it has been selected and iff buffer is at least 1 voxel (Z) deep
			if(resolutions[i] && (z_size/(POW_INT(2,i))) > 0)
			{
				if(show_progress_bar)
				{
					sprintf(progressBarMsg, "Saving to disc resolution %d",i+1);
                                        ProgressBar::instance()->updateInfo(progressBarMsg);
                                        ProgressBar::instance()->show();
				}

				//storing in 'base_path' the absolute path of the directory that will contain all stacks
				std::stringstream base_path;
				if ( par_mode ) // directory name depends on the total number of slices in the volume
					base_path << output_path << "/RES(" << (int)(height/POW_INT(2,i)) << "x" << 
						(int)(width/POW_INT(2,i)) << "x" << (int)(volume->getN_SLICES()/POW_INT(2,i)) << ")/";
				else 
					base_path << output_path << "/RES(" << (int)(height/POW_INT(2,i)) << "x" << 
						(int)(width/POW_INT(2,i)) << "x" << (int)(depth/POW_INT(2,i)) << ")/";

				//looping on new stacks
				for(int stack_row = 0, start_height = 0, end_height = 0; stack_row < n_stacks_V[i]; stack_row++)
				{
					//incrementing end_height
					end_height = start_height + stacks_height[i][stack_row][0][0]-1;
						
					//computing V_DIR_path and creating the directory the first time it is needed
					std::stringstream V_DIR_path;
					V_DIR_path << base_path.str() << this->getMultiresABS_V_string(i,start_height);
					if(!test_mode && z==D0 && !make_dir(V_DIR_path.str().c_str()))
					{
						char err_msg[S_STATIC_STRINGS_SIZE];
						sprintf(err_msg, "in mergeTiles(...): unable to create V_DIR = \"%s\"\n", V_DIR_path.str().c_str());
						throw iom::exception(err_msg);
					}

					for(int stack_column = 0, start_width=0, end_width=0; stack_column < n_stacks_H[i]; stack_column++)
					{
						end_width  = start_width  + stacks_width [i][stack_row][stack_column][0]-1;
							
						//computing H_DIR_path and creating the directory the first time it is needed
						std::stringstream H_DIR_path;
						H_DIR_path << V_DIR_path.str() << "/" << this->getMultiresABS_V_string(i,start_height) << "_" << this->getMultiresABS_H_string(i,start_width);
						if ( z==D0 ) {
							if(!test_mode && !make_dir(H_DIR_path.str().c_str()))
							{
								char err_msg[S_STATIC_STRINGS_SIZE];
								sprintf(err_msg, "in mergeTiles(...): unable to create H_DIR = \"%s\"\n", H_DIR_path.str().c_str());
								throw iom::exception(err_msg);
							}
							else { // the directory has been created for the first time
								   // initialize block files
								V3DLONG *sz = new V3DLONG[4];
								int datatype;
								char *err_rawfmt;

								sz[0] = stacks_width[i][stack_row][stack_column][0];
								sz[1] = stacks_height[i][stack_row][stack_column][0];
								sz[3] = 1; // onle one channel for now

								if ( saved_img_depth == 16 )
									datatype = 2;
								else if ( saved_img_depth == 8 ) 
									datatype = 1;
								else {
									char err_msg[S_STATIC_STRINGS_SIZE];
									sprintf(err_msg, "in mergeTilesVaa3DRaw(...): unknown image depth (%d)", saved_img_depth);
									throw iom::exception(err_msg);
								}

								int slice_start_temp = 0;
								for ( int j=0; j < n_stacks_D[i]; j++ ) {
									sz[2] = stacks_depth[i][stack_row][stack_column][j];

									std::stringstream abs_pos_z_temp;
									abs_pos_z_temp.width(6);
									abs_pos_z_temp.fill('0');
									abs_pos_z_temp << (int)(this->getMultiresABS_D(i,0) + // all stacks start at the same D position
                                       (POW_INT(2,i)*(slice_start_temp)) * volume->getVXL_D() * 10);

									std::stringstream img_path_temp;
									img_path_temp << H_DIR_path.str() << "/" 
												  << this->getMultiresABS_V_string(i,start_height) << "_" 
												  << this->getMultiresABS_H_string(i,start_width) << "_"
												  << abs_pos_z_temp.str();

									// 2014-09-10. Alessandro. @CHANGED 'saved_img_format' interpretation in 'mergeTilesVaa3DRaw()' method.
									if( strcmp(saved_img_format, "tif") == 0 || strcmp(saved_img_format, "tiff") == 0 || strcmp(saved_img_format, "TIF") == 0 || strcmp(saved_img_format, "TIFF") == 0)
										err_rawfmt = initTiff3DFile((char *)img_path_temp.str().c_str(),(uint32)sz[0],(uint32)sz[1],(uint32)sz[2],(uint32)sz[3],datatype);
									else if(strcmp(saved_img_format, "v3draw") == 0 || strcmp(saved_img_format, "raw") == 0)
										err_rawfmt = initRawFile((char *)img_path_temp.str().c_str(),sz,datatype);
									else
										throw iom::exception(vm::strprintf("in StackStitcher::mergeTilesVaa3DRaw(): unsupported image format \"%s\"", saved_img_format));
									if(err_rawfmt != 0)
										throw iom::exception(vm::strprintf("in StackStitcher::mergeTilesVaa3DRaw(): error in initializing block file (%s)", err_rawfmt));
								
									//if ( ( !strcmp(saved_img_format,"Tiff3D") ? // format can be only "Tiff3D" or "Vaa3DRaw"
									//			( (err_rawfmt = initTiff3DFile((char *)img_path_temp.str().c_str(),(uint32)sz[0],(uint32)sz[1],(uint32)sz[2],(uint32)sz[3],datatype)) != 0 ) : 
									//			( (err_rawfmt = initRawFile((char *)img_path_temp.str().c_str(),sz,datatype)) != 0 ) ) ) {
									//	char err_msg[S_STATIC_STRINGS_SIZE];
									//	sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: error in initializing block file - %s", err_rawfmt);
									//          throw iom::exception(err_msg);
									//};
									//fclose(tmp_file); // TEMP

									slice_start_temp += (int)sz[2];
								}
								delete [] sz;
							}
						}

						//saving HERE
						for(int buffer_z=0; buffer_z<z_size/(POW_INT(2,i)); buffer_z++)
						{
							int slice_ind; 
							std::stringstream img_path;
 							std::stringstream abs_pos_z_next;

							int rel_pos_z = (int)(POW_INT(2,i)*buffer_z+z-D0);		// Alessandro, 23/03/2013 - see below. This is the relative Z pixel coordinate in the 
																					// highest resolution image space. '-D0' is necessary to make it relative, since
																					// getMultiresABS_D_string(...) accepts relative coordinates only.

							/*std::stringstream abs_pos_z;
							abs_pos_z.width(6);
							abs_pos_z.fill('0');
							abs_pos_z << (int)(POW_INT(2,i)*buffer_z+z);*/	// Alessandro, 23/03/2013 - bug found: we are saving the image space coordinate (in pixels) 
																			// instead of the volume space coordinate (in tenths of microns)
							img_path << H_DIR_path.str() << "/" 
										<< this->getMultiresABS_V_string(i,start_height) << "_" 
										<< this->getMultiresABS_H_string(i,start_width)  << "_";
										
							// D0 must be subtracted because z is an absolute index in volume while slice index should be computed on a relative basis (i.e. starting form 0)
		                    if ( ((z - this->D0) / POW_INT(2,i)+buffer_z) > slice_end[i] ) { // start a new block along z !!! GI_140427 THIS HAS NOT BE CHECKED YET
								abs_pos_z_next.width(6);
								abs_pos_z_next.fill('0');
								//abs_pos_z_next << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
								abs_pos_z_next << (int)(this->getMultiresABS_D(i,0) + // all stacks start at the same D position
                                        (POW_INT(2,i)*(slice_end[i]+1)) * volume->getVXL_D() * 10);
								img_path << abs_pos_z_next.str();
								slice_ind = (int)(n_slices_pred - (slice_end[i]+1)) + buffer_z;
							}
							else {
								//img_path
									// << abs_pos_z.str();							// Alessandro, 23/03/2013 - bug found: see above
									//<< this->getMultiresABS_D_string(0, rel_pos_z);	// Alessandro, 23/03/2013 - we pass '0' because rel_pos_z is the relative Z
																					// pixel coordinate in the HIGHEST (i=0) resolution image space (see above).
								img_path << abs_pos_z.str(); 
								slice_ind = (int)(n_slices_pred - slice_start[i]) + buffer_z;
							}

							if(test_mode)
							{
								img_path.str("");
								img_path << volume->getSTACKS_DIR() << "/test_middle_slice";
							}

                            // @FIXED by Alessandro on 2014-06-25: iim::IOException objects must be caught here
                            try
                            {
								// 2014-09-10. Alessandro. @CHANGED 'saved_img_format' interpretation in 'mergeTilesVaa3DRaw()' method.
								std::string iim_format;
								if( strcmp(saved_img_format, "tif") == 0 || strcmp(saved_img_format, "tiff") == 0 || strcmp(saved_img_format, "TIF") == 0 || strcmp(saved_img_format, "TIFF") == 0)
									iim_format  = "Tiff3D";
								else if(strcmp(saved_img_format, "v3draw") == 0 || strcmp(saved_img_format, "raw") == 0)
									iim_format = "Vaa3DRaw";
								else
									throw iom::exception(vm::strprintf("in StackStitcher::mergeTilesVaa3DRaw(): unsupported image format \"%s\"", saved_img_format));

								// 2014-09-10. Alessandro. @FIXED 'mergeTilesVaa3DRaw' method: set 'imagemanager' module to silent mode.
								iim::DEBUG = iim::NO_DEBUG;

								// 2014-09-10. Alessandro. @CHANGED 'saved_img_format' interpretation in 'mergeTilesVaa3DRaw()' method.
                                iim::VirtualVolume::saveImage_to_Vaa3DRaw(
                                    slice_ind,
                                    img_path.str(),
                                    buffer + buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i)), // adds the stride
                                    (int)height/(POW_INT(2,i)),
                                    (int)width/(POW_INT(2,i)),
                                    start_height,end_height,start_width,end_width,
                                    iim_format.c_str(), saved_img_depth
                                );
                            }
                            catch( iim::IOException& exception)
                            {
                                throw iom::exception(exception.what());
                            }
						}
						start_width  += stacks_width [i][stack_row][stack_column][0];
					}
					start_height += stacks_height[i][stack_row][0][0];
				}
			}
		}

		// 2014-10-29. Giulio. @ADDED save next group data
		if ( !test_mode && !par_mode )
			saveResumerState(fhandle,resolutions_size,stack_block,slice_start,slice_end,z+z_max_res,z_parts+1);
	}

	int n_err = 0; // used to trigger exception in case the .bin file cannot be generated

	if ( !test_mode && !par_mode ) {
		// 2014-10-29. Giulio. @ADDED close resume 
		closeResumer(fhandle,output_path.c_str());

		// reloads created volumes to generate .bin file descriptors at all resolutions
		ref_sys temp = volume->getREF_SYS();  // required by clang compiler
		iim::ref_sys reference = *((iim::ref_sys *) &temp); // the cast is needed because there are two ref_sys in different name spaces
		for(int res_i=0; res_i< resolutions_size; res_i++)
		{
			if(resolutions[res_i])
			{
				//---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
				//one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
				//is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
				//system.
				try 
				{
					iim::DEBUG = iim::NO_DEBUG;
					TiledVolume temp_vol(file_path[res_i].str().c_str(),reference,
							volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));
				}
				catch (iim::IOException & ex)
				{
					printf("n StackStitcher::generateTilesVaa3DRaw: cannot create file mdata.bin in %s [reason: %s]\n\n",file_path[res_i].str().c_str(), ex.what());
					n_err++;
				}
				catch ( ... )
				{
					printf("in StackStitcher::generateTilesVaa3DRaw: cannot create file mdata.bin in %s [no reason available]\n\n",file_path[res_i].str().c_str());
					n_err++;
				}
			}
		}
	}

	// deallocate memory
    for(int res_i=0; res_i< resolutions_size; res_i++)
	{
		for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
		{
			for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
			{
				delete []stacks_height[res_i][stack_row][stack_col];
				delete []stacks_width [res_i][stack_row][stack_col];
				delete []stacks_depth [res_i][stack_row][stack_col];
			}
			delete []stacks_height[res_i][stack_row];
			delete []stacks_width [res_i][stack_row];
			delete []stacks_depth [res_i][stack_row];
		}
		delete []stacks_height[res_i];
		delete []stacks_width[res_i]; 
		delete []stacks_depth[res_i]; 
	}

	//releasing allocated memory
	delete buffer;
	if(stk_rst)
		delete stk_rst;

	if ( n_err ) { // errors in mdat.bin creation
		char err_msg[2000];
		sprintf(err_msg,"StackStitcher::generateTilesVaa3DRaw: %d errors in creating mdata.bin files", n_err);
        throw iom::exception(err_msg);
	}
}


void StackStitcher::createDirectoryHierarchy(std::string output_path, int block_height, int block_width, int block_depth, bool* resolutions, 
							   bool exclude_nonstitchable_stacks, int _ROW_START, int _ROW_END, int _COL_START,
							   int _COL_END, int _D0, int _D1, bool restoreSPIM, int restore_direction,
							   int blending_algo, bool test_mode, bool show_progress_bar, 
                               const char* saved_img_format, int saved_img_depth, bool par_mode)			throw (iom::exception)
{
#if S_VERBOSE > 2
	printf("......in StackStitcher::createDirectoryHierarchy(output_path=\"%s\", block_height=%d, block_width=%d, block_depth=%d, exclude_nonstitchable_stacks = %s, "
		"_ROW_START=%d, _ROW_END=%d, _COL_START=%d, _COL_END=%d, _D0=%d, _D1=%d, restoreSPIM = %s, restore_direction = %d, test_mode = %s, resolutions = { ",
		output_path.c_str(), slice_height, slice_width, slice_depth, (exclude_nonstitchable_stacks ? "true" : "false"), _ROW_START, _ROW_END,
		_COL_START, _COL_END, _D0, _D1, (restoreSPIM ? "ENABLED" : "disabled"), restore_direction, (test_mode ? "ENABLED" : "disabled"));
	for(int i=0; i<S_MAX_MULTIRES && resolutions; i++)
		printf("%d ", resolutions[i]);
	printf("}\n");
#endif

	//LOCAL VARIABLES
	sint64 height, width, depth;                                            //height, width and depth of the whole volume that covers all stacks
	iom::real_t* stripe_up=NULL;                                   //will contain up-stripe and down-stripe computed by calling 'getStripe' method
    int n_stacks_V[S_MAX_MULTIRES], n_stacks_H[S_MAX_MULTIRES], n_stacks_D[S_MAX_MULTIRES];             //array of number of tiles along V and H directions respectively at i-th resolution
    int ***stacks_height[S_MAX_MULTIRES], ***stacks_width[S_MAX_MULTIRES], ***stacks_depth[S_MAX_MULTIRES];	//array of matrices of tiles dimensions at i-th resolution
	int resolutions_size = 0;
	StackRestorer *stk_rst = NULL;

	std::stringstream file_path[S_MAX_MULTIRES];

	/* DEFINITIONS OF VARIABILES THAT MANAGE TILES (BLOCKS) ALONG D-direction
	 * In the following the term 'group' means the groups of slices that are 
	 * processed together to generate slices of all resolution requested
	 */

	/* these arrays are the indices of first and last slice of current block at resolution i
	 * WARNING: the slice index refers to the index of the slice in the volume at resolution i 
	 */
    int slice_start[S_MAX_MULTIRES];

	/* the number of slice of already processed groups at current resolution
	 * the index of the slice to be saved at current resolution is:
	 *
	 *      n_slices_pred + z_buffer
	 */

	//computing dimensions of volume to be stitched
	this->computeVolumeDims(exclude_nonstitchable_stacks, _ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
	width = this->H1-this->H0;
	height = this->V1-this->V0;
	depth = this->D1-this->D0;

	//activating resolutions
    block_height = (block_height == -1 ? (int)height : block_height);
    block_width  = (block_width  == -1 ? (int)width  : block_width);
    block_depth  = (block_depth  == -1 ? (int)depth  : block_depth);
    if(block_height < S_MIN_SLICE_DIM || block_width < S_MIN_SLICE_DIM /* 2014-10-29. Giulio. @REMOVED (|| block_depth < S_MIN_SLICE_DIM) */)
    {
        char err_msg[5000];
        sprintf(err_msg,"The minimum dimension for block width, height and depth is %d", S_MIN_SLICE_DIM);
        throw iom::exception(err_msg);
    }
	if(resolutions == NULL)
	{
        resolutions = new bool;
        *resolutions = true;
        resolutions_size = 1;
	}
	else
        for(int i=0; i<S_MAX_MULTIRES; i++)
            if(resolutions[i])
                resolutions_size = ISR_MAX(resolutions_size, i+1);

	//computing tiles dimensions at each resolution and initializing volume directories
	for(int res_i=0; res_i< resolutions_size; res_i++)
	{
        n_stacks_V[res_i] = static_cast<int>(ceil ( (height/POW_INT(2,res_i)) / (float) block_height ));
        n_stacks_H[res_i] = static_cast<int>(ceil ( (width/POW_INT(2,res_i))  / (float) block_width  ));
        n_stacks_D[res_i] = static_cast<int>(ceil ( (depth/POW_INT(2,res_i))  / (float) block_depth  ));
        stacks_height[res_i] = new int **[n_stacks_V[res_i]];
        stacks_width[res_i]  = new int **[n_stacks_V[res_i]];
        stacks_depth[res_i]  = new int **[n_stacks_V[res_i]];
        for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
        {
            stacks_height[res_i][stack_row] = new int *[n_stacks_H[res_i]];
            stacks_width [res_i][stack_row] = new int *[n_stacks_H[res_i]];
            stacks_depth [res_i][stack_row] = new int *[n_stacks_H[res_i]];
            for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
            {
				stacks_height[res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
				stacks_width [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
				stacks_depth [res_i][stack_row][stack_col] = new int[n_stacks_D[res_i]];
				for(int stack_sli = 0; stack_sli < n_stacks_D[res_i]; stack_sli++)
				{
                	stacks_height[res_i][stack_row][stack_col][stack_sli] = 
                		((int)(height/POW_INT(2,res_i))) / n_stacks_V[res_i] + (stack_row < ((int)(height/POW_INT(2,res_i))) % n_stacks_V[res_i] ? 1:0);
                	stacks_width [res_i][stack_row][stack_col][stack_sli] = 
                		((int)(width/POW_INT(2,res_i)))  / n_stacks_H[res_i] + (stack_col < ((int)(width/POW_INT(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
					stacks_depth[res_i][stack_row][stack_col][stack_sli] = 
                		((int)(depth/POW_INT(2,res_i)))  / n_stacks_D[res_i] + (stack_sli < ((int)(depth/POW_INT(2,res_i)))  % n_stacks_D[res_i] ? 1:0);
				}
            }
        }
        //creating volume directory iff current resolution is selected and test mode is disabled
        if(resolutions[res_i] == true && !test_mode)
        {
            //creating directory that will contain image data at current resolution
            file_path[res_i]<<output_path<<"/RES("<<height/POW_INT(2,res_i)<<"x"<<width/POW_INT(2,res_i)<<"x"<<depth/POW_INT(2,res_i)<<")";
            if(!make_dir(file_path[res_i].str().c_str()))
            {
                char err_msg[S_STATIC_STRINGS_SIZE];
                sprintf(err_msg, "in StackStitcher::createDirectoryHierarchy(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                throw iom::exception(err_msg);
            }

			//Alessandro - 23/03/2013: saving original volume XML descriptor into each folder
			char xmlPath[S_STATIC_STRINGS_SIZE];
			sprintf(xmlPath, "%s/original_volume_desc.xml", file_path[res_i].str().c_str());
			volume->saveXML(0, xmlPath);
        }
	}


	//saving current buffer data at selected resolutions and in multitile format
	for(int i=0; i< resolutions_size; i++)
	{
		// find abs_pos_z at resolution i
		std::stringstream abs_pos_z;
		abs_pos_z.width(6);
		abs_pos_z.fill('0');
		abs_pos_z << (int)(this->getMultiresABS_D(i,0) + // all stacks start at the same D position
							//- D0 * volume->getVXL_D() * 10 + // WARNING: D0 is counted twice,both in getMultiresABS_D and in slice_start
                            (POW_INT(2,i)*slice_start[i]) * volume->getVXL_D() * 10);

		//saving at current resolution if it has been selected and iff buffer is at least 1 voxel (Z) deep
		if(resolutions[i])
		{
			//storing in 'base_path' the absolute path of the directory that will contain all stacks
			std::stringstream base_path;
            base_path << output_path << "/RES(" << (int)(height/POW_INT(2,i)) << "x" << 
                (int)(width/POW_INT(2,i)) << "x" << (int)(depth/POW_INT(2,i)) << ")/";

			//looping on new stacks
			for(int stack_row = 0, start_height = 0, end_height = 0; stack_row < n_stacks_V[i]; stack_row++)
			{
				//incrementing end_height
				end_height = start_height + stacks_height[i][stack_row][0][0]-1;
						
				//computing V_DIR_path and creating the directory the first time it is needed
				std::stringstream V_DIR_path;
				V_DIR_path << base_path.str() << this->getMultiresABS_V_string(i,start_height);
				if(!test_mode && !make_dir(V_DIR_path.str().c_str()))
				{
					char err_msg[S_STATIC_STRINGS_SIZE];
					sprintf(err_msg, "in StackStitcher::createDirectoryHierarchy(...): unable to create V_DIR = \"%s\"\n", V_DIR_path.str().c_str());
					throw iom::exception(err_msg);
				}

				for(int stack_column = 0, start_width=0, end_width=0; stack_column < n_stacks_H[i]; stack_column++)
				{
					end_width  = start_width  + stacks_width [i][stack_row][stack_column][0]-1;
							
					//computing H_DIR_path and creating the directory the first time it is needed
					std::stringstream H_DIR_path;
					H_DIR_path << V_DIR_path.str() << "/" << this->getMultiresABS_V_string(i,start_height) << "_" << this->getMultiresABS_H_string(i,start_width);
					if(!test_mode && !make_dir(H_DIR_path.str().c_str()))
					{
						char err_msg[S_STATIC_STRINGS_SIZE];
						sprintf(err_msg, "in StackStitcher::createDirectoryHierarchy(...): unable to create H_DIR = \"%s\"\n", H_DIR_path.str().c_str());
						throw iom::exception(err_msg);
					}
					start_width  += stacks_width [i][stack_row][stack_column][0];
				}
				start_height += stacks_height[i][stack_row][0][0];
			}
		}
	}

}


void StackStitcher::mdataGenerator (std::string output_path, int block_height, int block_width, int block_depth, bool* resolutions, 
							   bool exclude_nonstitchable_stacks, int _ROW_START, int _ROW_END, int _COL_START,
							   int _COL_END, int _D0, int _D1, bool restoreSPIM, int restore_direction,
							   int blending_algo, bool test_mode, bool show_progress_bar, 
                               const char* saved_img_format, int saved_img_depth, bool par_mode)			throw (iom::exception) 
{
#if S_VERBOSE > 2
	printf("......in StackStitcher::mdataGenerator(output_path=\"%s\", block_height=%d, block_width=%d, block_depth=%d, exclude_nonstitchable_stacks = %s, "
		"_ROW_START=%d, _ROW_END=%d, _COL_START=%d, _COL_END=%d, _D0=%d, _D1=%d, restoreSPIM = %s, restore_direction = %d, test_mode = %s, resolutions = { ",
		output_path.c_str(), slice_height, slice_width, slice_depth, (exclude_nonstitchable_stacks ? "true" : "false"), _ROW_START, _ROW_END,
		_COL_START, _COL_END, _D0, _D1, (restoreSPIM ? "ENABLED" : "disabled"), restore_direction, (test_mode ? "ENABLED" : "disabled"));
	for(int i=0; i<S_MAX_MULTIRES && resolutions; i++)
		printf("%d ", resolutions[i]);
	printf("}\n");
#endif

	//LOCAL VARIABLES
	sint64 height, width, depth;                                            //height, width and depth of the whole volume that covers all stacks
	int resolutions_size = 0;

	std::stringstream file_path[S_MAX_MULTIRES];

	//computing dimensions of volume to be stitched
	this->computeVolumeDims(exclude_nonstitchable_stacks, _ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
	width = this->H1-this->H0;
	height = this->V1-this->V0;
	depth = this->D1-this->D0;

	if(resolutions == NULL)
	{
        resolutions = new bool;
        *resolutions = true;
        resolutions_size = 1;
	}
	else
        for(int i=0; i<S_MAX_MULTIRES; i++)
            if(resolutions[i])
                resolutions_size = ISR_MAX(resolutions_size, i+1);

	//computing tiles dimensions at each resolution and initializing volume directories
	for(int res_i=0; res_i< resolutions_size; res_i++)
	{
        //creating volume directory iff current resolution is selected and test mode is disabled
        if(resolutions[res_i] == true && !test_mode)
        {
			//creating directory that will contain image data at current resolution
			file_path[res_i]<<output_path<<"/RES("<<height/POW_INT(2,res_i)<<"x"<<width/POW_INT(2,res_i)<<"x"<<depth/POW_INT(2,res_i)<<")";
        }
	}

	int n_err = 0; // used to trigger exception in case the .bin file cannot be generated

	if ( !test_mode ) {
		// reloads created volumes to generate .bin file descriptors at all resolutions
		ref_sys temp = volume->getREF_SYS();  // required by clang compiler
		iim::ref_sys reference = *((iim::ref_sys *) &temp); // the cast is needed because there are two ref_sys in different name spaces
		for(int res_i=0; res_i< resolutions_size; res_i++)
		{
			if(resolutions[res_i])
			{
				//---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
				//one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
				//is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
				//system.
				try 
				{
					iim::DEBUG = iim::NO_DEBUG;
					TiledVolume temp_vol(file_path[res_i].str().c_str(),reference,
							volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));
				}
				catch (iim::IOException & ex)
				{
					printf("n StackStitcher::mdataGenerator: cannot create file mdata.bin in %s [reason: %s]\n\n",file_path[res_i].str().c_str(), ex.what());
					n_err++;
				}
				catch ( ... )
				{
					printf("in StackStitcher::mdataGenerator: cannot create file mdata.bin in %s [no reason available]\n\n",file_path[res_i].str().c_str());
					n_err++;
				}
			}
		}
	}


	//releasing allocated memory
	if ( n_err ) { // errors in mdat.bin creation
		char err_msg[2000];
		sprintf(err_msg,"StackStitcher::mdataGenerator: %d errors in creating mdata.bin files", n_err);
        throw iom::exception(err_msg);
	}
}