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
* 2017-02-10.  Giulio.     @ADDED in merge methods added a parameter to specify the blending algorithm to be used for layers 
*/


# include "StackStitcher2.h"

#include <iostream>
#include <string>
#include <sstream>
#include <limits>
#include <list>
#include <ctime>

#include "ProgressBar.h"
#include "S_config.h"

#include "RawFmtMngr.h"
#include "Tiff3DMngr.h"
#include "IM_config.h"
#include "VirtualVolume.h"
#include "TiledVolume.h"
#include "UnstitchedVolume.h" // 2017-02-10. Giulio. @ADDED
#include "my_defs.h"

using namespace IconImageManager;
using namespace iomanager;
using namespace volumemanager;

//struct coord_2D{int V,H;};
//struct stripe_2Dcoords{coord_2D up_left, bottom_right;};
//struct stripe_corner{int h,H; bool up;};
//struct stripe_2Dcorners{std::list<stripe_corner> ups, bottoms, merged;};
//bool compareCorners (stripe_corner first, stripe_corner second);

//typedef long V3DLONG;


void StackStitcher2::mergeTilesVaa3DRaw(std::string output_path, int block_height, int block_width, int block_depth, bool* resolutions, 
							   int _ROW_START, int _ROW_END, int _COL_START,
							   int _COL_END, int _D0, int _D1, int blending_algo, int intralayer_blending_algo, bool test_mode, bool show_progress_bar, 
							   const char* saved_img_format, int saved_img_depth)			throw (IOException)
{
	//FILE *tmp_file; // TEMP

        #if S_VERBOSE > 2
        printf("......in StackStitcher::mergeTiles(output_path=\"%s\", block_height=%d, block_width=%d, block_depth=%d, exclude_nonstitchable_stacks = %s, "
               "_ROW_START=%d, _ROW_END=%d, _COL_START=%d, _COL_END=%d, _D0=%d, _D1=%d, restoreSPIM = %s, restore_direction = %d, test_mode = %s, resolutions = { ",
                output_path.c_str(), slice_height, slice_width, slice_depth, (exclude_nonstitchable_stacks ? "true" : "false"), _ROW_START, _ROW_END,
                _COL_START, _COL_END, _D0, _D1, (restoreSPIM ? "ENABLED" : "disabled"), restore_direction, (test_mode ? "ENABLED" : "disabled"));
		for(int i=0; i<S_MAX_MULTIRES && resolutions; i++)
            printf("%d ", resolutions[i]);
        printf("}\n");
        #endif

	//LOCAL VARIABLES
	iim::sint64 height, width, depth;                                            //height, width and depth of the whole volume that covers all stacks
	iim::real32* buffer;								//buffer temporary image data are stored
	iim::real32* stripe_up=NULL;                                   //will contain up-stripe and down-stripe computed by calling 'getStripe' method
	double angle;								//angle between 0 and PI used to sample overlapping zone in [0,PI]
	double delta_angle;							//angle step
	int z_ratio, z_max_res;
    int n_stacks_V[S_MAX_MULTIRES], n_stacks_H[S_MAX_MULTIRES], n_stacks_D[S_MAX_MULTIRES];             //array of number of tiles along V and H directions respectively at i-th resolution
    int ***stacks_height[S_MAX_MULTIRES], ***stacks_width[S_MAX_MULTIRES], ***stacks_depth[S_MAX_MULTIRES];	//array of matrices of tiles dimensions at i-th resolution
	//iim::iim::sint64 u_strp_bottom_displ;
	//iim::iim::sint64 d_strp_top_displ;
	//iim::iim::sint64 u_strp_top_displ;
	//iim::iim::sint64 d_strp_left_displ;
	//iim::iim::sint64 u_strp_left_displ;
	//iim::iim::sint64 d_strp_width;
	//iim::iim::sint64 u_strp_width;
	//iim::iim::sint64 dd_strp_top_displ;
	//iim::iim::sint64 u_strp_d_strp_overlap;
	//iim::iim::sint64 h_up, h_down, h_overlap;
	//stripe_2Dcoords  *stripesCoords;
	//stripe_2Dcorners *stripesCorners;
	int resolutions_size = 0;
	//StackRestorer *stk_rst = NULL;
	//iim::real32 *buffer_ptr, *ustripe_ptr, *dstripe_ptr;	
	iim::real32 (*blending)(double& angle, iim::real32& pixel1, iim::real32& pixel2);

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
	iim::sint64 n_slices_pred;       

	//retrieving intra layers blending function
	if(blending_algo == S_SINUSOIDAL_BLENDING)
            blending = sinusoidal_blending;
	else if(blending_algo == S_NO_BLENDING)
            blending = no_blending;
	else if(blending_algo == S_SHOW_STACK_MARGIN)
            blending = stack_margin;
	else if(blending_algo == S_ENHANCED_NO_BLENDING)
            blending = enhanced_no_blending;
	else if(blending_algo == S_TOPLAYER_OVERWRITE)
            blending = (iim::real32 (*)(double& angle, iim::real32& pixel1, iim::real32& pixel2)) 0;
	else
            throw IOException("in StackStitcher::mergeTilesVaa3DRaw(...): unrecognized blending function");

	// 2017-02-10. Giulio. @ADDED setting of the blending function to be used for intralayers merging
	if ( dynamic_cast<UnstitchedVolume *>((volume->getLAYER(0))) ) { // volumes of multilayer volume are unstitched 
		for ( int i=0; i<volume->getN_LAYERS(); i++ )
			((UnstitchedVolume *) (volume->getLAYER(i)))->setBLENDING_ALGO(intralayer_blending_algo);
	}

	// 2015-05-14. Giulio. @ADDED selection of IO plugin if not provided.
	if(iom::IMOUT_PLUGIN.compare("empty") == 0)
	{
		iom::IMOUT_PLUGIN = "tiff3D";
	}

	//initializing the progress bar
	char progressBarMsg[200];
    ts::ProgressBar::instance();
	if(show_progress_bar)
	{
            ts::ProgressBar::instance()->start("Multiresolution tile merging");
            ts::ProgressBar::instance()->setProgressValue(0,"Initializing...");
            ts::ProgressBar::instance()->display();
	}

	//computing dimensions of volume to be stitched
	this->computeVolumeDims(_ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
	width = this->H1-this->H0;
	height = this->V1-this->V0;
	depth = this->D1-this->D0;

	//activating resolutions
    block_height = (block_height == -1 ? (int)height : block_height);
    block_width  = (block_width  == -1 ? (int)width  : block_width);
    block_depth  = (block_depth  == -1 ? (int)depth  : block_depth);
    if(block_height < S_MIN_SLICE_DIM || block_width < S_MIN_SLICE_DIM || block_depth < S_MIN_SLICE_DIM)
    {
        char err_msg[5000];
        sprintf(err_msg,"The minimum dimension for block width, height and depth is %d", S_MIN_SLICE_DIM);
        throw IOException(err_msg);
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
        n_stacks_V[res_i] = (int) ceil ( (height/POW_INT(2,res_i)) / (float) block_height );
        n_stacks_H[res_i] = (int) ceil ( (width/POW_INT(2,res_i))  / (float) block_width  );
        n_stacks_D[res_i] = (int) ceil ( (depth/POW_INT(2,res_i))  / (float) block_depth  );
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
            if(!vm::make_dir(file_path[res_i].str().c_str()))
            {
                char err_msg[S_STATIC_STRINGS_SIZE];
                sprintf(err_msg, "in mergeTiles(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                throw IOException(err_msg);
            }

			//Alessandro - 23/03/2013: saving original volume XML descriptor into each folder
			char xmlPath[S_STATIC_STRINGS_SIZE];
			sprintf(xmlPath, "%s/original_volume_desc.xml", file_path[res_i].str().c_str());
			volume->saveXML(0, xmlPath);
        }
	}

	//ALLOCATING  the MEMORY SPACE for image buffer
	z_max_res = POW_INT(2,resolutions_size-1);
	z_ratio=(int)depth/z_max_res;
	buffer = new iim::real32[height*width*z_max_res];

	// a check should be added to verify that a buffer cannot involve more than two layers
	// i.e. that z_max_res <= MIN(volume->getLAYER_DIM(i,2), for i=0, ... , volume->N_LAYERS()
	for ( int layer=1; layer<(volume->getN_LAYERS()-1); layer++ ) {
		if ( z_max_res > (volume->getLAYER_COORDS(layer+1,2) - volume->getLAYER_COORDS(layer,2)) )
    	{
        	char err_msg[5000];
        	sprintf(err_msg,"The buffer size is too large and may involve more than two layers");
        	throw IOException(err_msg);
    	}
	}
	
	//slice_start and slice_end of current block depend on the resolution
	for(int res_i=0; res_i< resolutions_size; res_i++) {
		stack_block[res_i] = 0;
		//slice_start[res_i] = this->D0; 
		slice_start[res_i] = 0; // indices must start from 0 because they should have relative meaning 
		slice_end[res_i] = slice_start[res_i] + stacks_depth[res_i][0][0][0] - 1;
	}

	//#ifdef S_TIME_CALC
	//double proc_time;
	//#endif


	/********************************************************************************** 
	* WARNING: in the following code all slice intervals are represented by two slice 
	* indices:
	*   - the index of the first slice of the interval (start)
	*   - the index of of the slice after the last slice of the interval (end) 
	* The code also assumes that a buffer cannot involve more than two layers
	**********************************************************************************/

	// variables needed by buffer generation
	iim::real32 *buf1 = (iim::real32 *) 0;
	iim::real32 *buf2 = (iim::real32 *) 0;
	iim::sint64 z_next; // first slice after the last of the buffer
	int cur_layer = 0;  // index of current layer
	int cur_layer_start, cur_layer_end;
	int second_layer_start, second_layer_end;
	int overlap_start, overlap_end;
	iim::sint64 offs, offs2;
	bool blending_flag = false;
	iim::real32 (*saved_blending)(double& angle, iim::real32& pixel1, iim::real32& pixel2) = blending; // to temporarily disable blending

	for(iim::sint64 z = this->D0, z_parts = 1; z < this->D1; z += z_max_res, z_parts++)
	{
		blending = saved_blending; // re-enable blending in case it has been disabled

		// INV: z <= (volume->getLAYER_COORDS(cur_layer,2) + volume->getLAYER_DIM(cur_layer,2))

		// empty the buffer
		memset(buffer,0,height*width*z_max_res*sizeof(iim::real32));
		//for ( int hh=0; hh<(height*width*z_max_res); hh++ )
		//	buffer[hh] = 1.0;

		// set z_next
		z_next = z + (z_parts <= z_ratio ? z_max_res : depth%z_max_res);

		// compute interval of current layer to be put in the buffer
		cur_layer_start = (int) MAX(z,volume->getLAYER_COORDS(cur_layer,2)); 
		cur_layer_end   = (int) MIN(z_next,(volume->getLAYER_COORDS(cur_layer,2) + volume->getLAYER_DIM(cur_layer,2)));

		// INV: z <= cur_layer_end 

		/* if there are gaps between layers it can happen that z < volume->getLAYER_COORDS(cur_layer,2)
		 * in this case (volume->getLAYER_COORDS(cur_layer,2) - z) slices should be left empty
		 */
		if ( cur_layer_start < cur_layer_end ) { // there are data to be loaded from current layer
			// load data from current layer and copies it to buffer
			buf1 = volume->getSUBVOL(cur_layer,-1,-1,-1,-1,(cur_layer_start - volume->getLAYER_COORDS(cur_layer,2)),(cur_layer_end - volume->getLAYER_COORDS(cur_layer,2)));
			// skip (cur_layer_start - z) empty slices if any
			offs = (cur_layer_start - z) * width * height + (volume->getLAYER_COORDS(cur_layer,0) * width + volume->getLAYER_COORDS(cur_layer,1));
			copyBlock2SubBuf(buf1,buffer+offs,
							 volume->getLAYER_DIM(cur_layer,0),volume->getLAYER_DIM(cur_layer,1),(cur_layer_end - cur_layer_start),
							 volume->getLAYER_DIM(cur_layer,1),volume->getLAYER_DIM(cur_layer,0) * volume->getLAYER_DIM(cur_layer,1),width,height * width);
		}
		else { // the current layer does not contribute to fill the buffer
			blending = (iim::real32 (*)(double& angle, iim::real32& pixel1, iim::real32& pixel2)) 0; // blending is termporarily disabled
		}

		// check if there is a second buffer involved 
		// (current layer is not the last) && the first slice after the buffer is greater than the first slice of next layer (a layer deep 'cut_depth' us is discarded)
		if ( (cur_layer < (volume->getN_LAYERS() - 1)) && (z_next > (volume->getLAYER_COORDS(cur_layer+1,2) + volume->getCUT_DEPTH_PXL())) ) /* exploits short-circuit evaluation of boolean expressions */ {
			second_layer_start = (int) MAX(z,(volume->getLAYER_COORDS(cur_layer+1,2) + volume->getCUT_DEPTH_PXL()));
			second_layer_end   = (int) z_next;
			// second_layer_start < second_layer_end = z_next
			overlap_start      = second_layer_start;
			overlap_end        = MAX(second_layer_start,cur_layer_end); // the overlap region cannot end before the second layer (it would mean it is empty and only the copy step has to be performed)
			if ( blending_algo == S_SINUSOIDAL_BLENDING || blending_algo == S_SHOW_STACK_MARGIN || blending_algo == S_TOPLAYER_OVERWRITE ) {
				// check if flag is already set
				if ( !blending_flag ) {
					// beginning of blending region
					blending_flag = true;
					delta_angle = iim::PI/(volume->getLAYER_COORDS(cur_layer,2) + volume->getLAYER_DIM(cur_layer,2) - volume->getLAYER_COORDS(cur_layer+1,2) - volume->getCUT_DEPTH_PXL()); // a layers deep 'cut_depth' us has been discarded
					angle = 0;
				}

				buf2 = volume->getSUBVOL(cur_layer+1,-1,-1,-1,-1,(second_layer_start - volume->getLAYER_COORDS(cur_layer+1,2)),(second_layer_end - volume->getLAYER_COORDS(cur_layer+1,2)));
				// add the first part with blending (or overwriting previous buffer if blending in a null pointer, i.e. blending_algo == S_TOPLAYER_OVERWRITE)
				for ( int s=0; s<(overlap_end - overlap_start); s++, angle+=delta_angle ) {
					offs = ((height * width * (overlap_start-cur_layer_start+s)) + (volume->getLAYER_COORDS(cur_layer+1,0) * width + volume->getLAYER_COORDS(cur_layer+1,1)));
					offs2 = (volume->getLAYER_DIM(cur_layer+1,0) * volume->getLAYER_DIM(cur_layer+1,1) * s);
					addBlock2SubBuf(buf2+offs2,buffer+offs,
									volume->getLAYER_DIM(cur_layer+1,0),volume->getLAYER_DIM(cur_layer+1,1),1, // merge just one slice
	 								volume->getLAYER_DIM(cur_layer+1,1),volume->getLAYER_DIM(cur_layer+1,0) * volume->getLAYER_DIM(cur_layer+1,1),width,height * width,
									blending,angle);
				}
				if ( overlap_end < second_layer_end ) {
					// add the second part 
					offs = ((height * width * (overlap_end-cur_layer_start)) + (volume->getLAYER_COORDS(cur_layer+1,0) * width + volume->getLAYER_COORDS(cur_layer+1,1)));
					offs2 = (volume->getLAYER_DIM(cur_layer+1,0) * volume->getLAYER_DIM(cur_layer+1,1) * (overlap_end - overlap_start));
					copyBlock2SubBuf(buf2+offs2,buffer+offs,
									 volume->getLAYER_DIM(cur_layer+1,0),volume->getLAYER_DIM(cur_layer+1,1),(second_layer_end - overlap_end),
									 volume->getLAYER_DIM(cur_layer+1,1),volume->getLAYER_DIM(cur_layer+1,0) * volume->getLAYER_DIM(cur_layer+1,1),width,height * width);
				}
				delete buf2;
			}
			else if ( overlap_end < second_layer_end ) { // blending_algo == S_NO_BLENDING || blending_algo == S_ENHANCED_NO_BLENDING
				// there is a non overlapping interval and only this has to be loaded and copied
				buf2 = volume->getSUBVOL(cur_layer+1,-1,-1,-1,-1,(overlap_end - volume->getLAYER_COORDS(cur_layer+1,2)),(second_layer_end - volume->getLAYER_COORDS(cur_layer+1,2)));
				copyBlock2SubBuf(buf2,buffer+(volume->getLAYER_COORDS(cur_layer+1,0) * width + volume->getLAYER_COORDS(cur_layer+1,1))+(height * width * (overlap_end-cur_layer_start)),
								 volume->getLAYER_DIM(cur_layer+1,0),volume->getLAYER_DIM(cur_layer+1,1),(second_layer_end - overlap_end),
								 volume->getLAYER_DIM(cur_layer+1,1),volume->getLAYER_DIM(cur_layer+1,0) * volume->getLAYER_DIM(cur_layer+1,1),width,height * width);
				delete buf2;
			}
		}
		else if ( blending_flag ) { // there is not a second buffer involved
			// end of blending region
			blending_flag = false;
		}

		delete buf1;

		// check if current layer must be changed
		if ( cur_layer_end == (volume->getLAYER_COORDS(cur_layer,2) + volume->getLAYER_DIM(cur_layer,2)) ) 
			cur_layer++;

		//updating the progress bar
		if(show_progress_bar)
		{	
			sprintf(progressBarMsg, "Merging slice %d of %d",((iim::uint32)(z_next-D0)),(iim::uint32)depth);
					ts::ProgressBar::instance()->setProgressValue(((float)(z_next-D0)*100/(float)depth), progressBarMsg);
					ts::ProgressBar::instance()->display();
		}

		//saving current buffer data at selected resolutions and in multitile format
		for(int i=0; i< resolutions_size; i++)
		{
			if(show_progress_bar)
			{
				sprintf(progressBarMsg, "Generating resolution %d of %d",i+1,ISR_MAX(resolutions_size, resolutions_size));
                                ts::ProgressBar::instance()->setProgressInfo(progressBarMsg);
                                ts::ProgressBar::instance()->display();
			}

			// check if current block is changed
            if ( (z / POW_INT(2,i)) > slice_end[i] ) {
				stack_block[i]++;
				slice_start[i] = slice_end[i] + 1;
				slice_end[i] += stacks_depth[i][0][0][stack_block[i]];
			}

			// find abs_pos_z at resolution i
			std::stringstream abs_pos_z;
			abs_pos_z.width(6);
			abs_pos_z.fill('0');
			abs_pos_z << (int)(this->getMultiresABS_D(i,0) + // all stacks start at the same D position
     						- D0 * volume->getVXL_D() * 10 + // WARNING: D0 is counted twice,both in getMultiresABS_D and in slice_start
                        	(POW_INT(2,i)*slice_start[i]) * volume->getVXL_D() * 10);

			//compute the number of slice of previous groups at resolution i
			//note that z_parts in the number and not an index (starts from 1)
            n_slices_pred  = (z_parts - 1) * z_max_res / POW_INT(2,i);

			//buffer size along D is different when the remainder of the subdivision by z_max_res is considered
			iim::sint64 z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

			//halvesampling resolution if current resolution is not the deepest one
			if(i!=0)	
				StackStitcher2::halveSample(buffer,(int)height/(POW_INT(2,i-1)),(int)width/(POW_INT(2,i-1)),(int)z_size/(POW_INT(2,i-1)));

			//saving at current resolution if it has been selected and iff buffer is at least 1 voxel (Z) deep
			if(resolutions[i] && (z_size/(POW_INT(2,i))) > 0)
			{
				if(show_progress_bar)
				{
					sprintf(progressBarMsg, "Saving to disc resolution %d",i+1);
                                        ts::ProgressBar::instance()->setProgressInfo(progressBarMsg);
                                        ts::ProgressBar::instance()->display();
				}

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
					if(!test_mode && z==D0 && !vm::make_dir(V_DIR_path.str().c_str()))
					{
						char err_msg[S_STATIC_STRINGS_SIZE];
						sprintf(err_msg, "in mergeTiles(...): unable to create V_DIR = \"%s\"\n", V_DIR_path.str().c_str());
						throw IOException(err_msg);
					}

					for(int stack_column = 0, start_width=0, end_width=0; stack_column < n_stacks_H[i]; stack_column++)
					{
						end_width  = start_width  + stacks_width [i][stack_row][stack_column][0]-1;

						//computing H_DIR_path and creating the directory the first time it is needed
						std::stringstream H_DIR_path;
						H_DIR_path << V_DIR_path.str() << "/" << this->getMultiresABS_V_string(i,start_height) << "_" << this->getMultiresABS_H_string(i,start_width);
						if ( z==D0 ) {
							if(!test_mode && !vm::make_dir(H_DIR_path.str().c_str()))
							{
								char err_msg[S_STATIC_STRINGS_SIZE];
								sprintf(err_msg, "in mergeTiles(...): unable to create H_DIR = \"%s\"\n", H_DIR_path.str().c_str());
								throw IOException(err_msg);
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
									throw IOException(err_msg);
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

									if ( ( !strcmp(saved_img_format,"Tiff3D") ? // format can be only "Tiff3D" or "Vaa3DRaw"
												( (err_rawfmt = initTiff3DFile((char *)img_path_temp.str().c_str(),(unsigned int)sz[0],(unsigned int)sz[1],(unsigned int)sz[2],(unsigned int)sz[3],datatype)) != 0 ) : 
												( (err_rawfmt = initRawFile((char *)img_path_temp.str().c_str(),sz,datatype)) != 0 ) ) ) {
										char err_msg[S_STATIC_STRINGS_SIZE];
										sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: error in initializing block file - %s", err_rawfmt);
                                        throw IOException(err_msg);
									};
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

							int rel_pos_z = (int) (POW_INT(2,i)*buffer_z+z-D0);		// Alessandro, 23/03/2013 - see below. This is the relative Z pixel coordinate in the 
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
										
		                    if ( (z/POW_INT(2,i)+buffer_z) > slice_end[i] ) { // start a new block along z !!! GI_140427 THIS HAS NOT BE CHECKED YET
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
								img_path << volume->getLAYERS_DIR() << "/test_middle_slice";
							}

							iim::VirtualVolume::saveImage_to_Vaa3DRaw(
								slice_ind,
								img_path.str(), 
                    	        buffer + buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i)), // adds the stride
                                (int)height/(POW_INT(2,i)),
								(int)width/(POW_INT(2,i)),
								start_height,end_height,start_width,end_width, 
								saved_img_format, saved_img_depth
							);
						}
						start_width  += stacks_width [i][stack_row][stack_column][0];
					}
					start_height += stacks_height[i][stack_row][0][0];
				}
			}
		}
	}

	int n_err = 0; // used to trigger exception in case the .bin file cannot be generated

	// 2016-04-10. Giulio. @ADDED the TiledVolume constructor may change the input plugin if it is wrong
	std::string save_imin_plugin = iom::IMIN_PLUGIN; // save current input plugin
	// 2016-04-28. Giulio. Now the generated image should be read: use the output plugin
	iom::IMIN_PLUGIN = iom::IMOUT_PLUGIN;

	if ( !test_mode ) {
		// reloads created volumes to generate .bin file descriptors at all resolutions
		//iim::ref_sys temp = volume->getREF_SYS();  // required by clang compiler
		//iim::ref_sys reference = *((iim::ref_sys *) &temp); // the cast is needed because there are two ref_sys in different name spaces
		for(int res_i=0; res_i< resolutions_size; res_i++)
		{
			if(resolutions[res_i])
			{
				//---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
				//one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
				//is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
				//system.
				try {
					TiledVolume temp_vol(file_path[res_i].str().c_str(),volume->getREF_SYS(), //reference,
							volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));
				}
				catch (iim::IOException & ex)
				{
					printf("n VolumeConverter::generateTilesVaa3DRaw: cannot create file mdata.bin in %s [reason: %s]\n\n",file_path[res_i].str().c_str(), ex.what());
					n_err++;
				}
				catch ( ... )
				{
					printf("in VolumeConverter::generateTilesVaa3DRaw: cannot create file mdata.bin in %s [no reason available]\n\n",file_path[res_i].str().c_str());
					n_err++;
				}
			}
		}
	}

	// restore input plugin
	iom::IMIN_PLUGIN = save_imin_plugin;

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
	//if(stk_rst)
	//	delete stk_rst;

	if ( n_err ) { // errors in mdat.bin creation
		char err_msg[2000];
		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: %d errors in creating mdata.bin files", n_err);
        throw IOException(err_msg);
	}
}

