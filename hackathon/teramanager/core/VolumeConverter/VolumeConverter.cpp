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
*
*       Bria, A., et al., (2012) "Stitching Terabyte-sized 3D Images Acquired in Confocal Ultramicroscopy", Proceedings of the 9th IEEE International Symposium on Biomedical Imaging.
*       Bria, A., Iannello, G., "TeraStitcher - A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Microscopy Images", submitted for publication, 2012.
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

#include "VolumeConverter.h"
#include "ProgressBar.h"
#include <math.h>

/*******************************************************************************************************
* Volume formats supported:
* 
* SimpleVolume:  simple sequence of slices stored as 2D images in the same directory
* StackedVolume: bidimensional matrix of 3D stacks stored in a hierarchical structure of directories
*
* WARNING: StackedVolume is already included by VolumeConverter.h
*******************************************************************************************************/
#include "SimpleVolume.h"
#include "RawVolume.h"
/******************************************************************************************************/

#include <limits>
#include <list>
#include <stdlib.h>
#include <sstream>
#include <cstdio>


//initialization of class members
double VolumeConverter::time_displ_comp=0;
double VolumeConverter::time_merging=0;
double VolumeConverter::time_stack_desc=0;
double VolumeConverter::time_stack_restore=0;
double VolumeConverter::time_multiresolution=0;


VolumeConverter::VolumeConverter( ) {
	volume = (VirtualVolume *) 0;
}


VolumeConverter::~VolumeConverter() { 
	if(volume)
		delete volume;
}


void VolumeConverter::setSrcVolume(const char* _root_dir, const char* _fmt, const char* _out_fmt) throw (MyException) {

	if ( strcmp(_fmt,STACKED_FORMAT) == 0 ) 
		volume = new StackedVolume(_root_dir,ref_sys(vertical,horizontal,depth),(float)1.0,(float)1.0,(float)1.0);
	else if ( strcmp(_fmt,SIMPLE_FORMAT) == 0 ) 
		volume = new SimpleVolume(_root_dir);
	else if ( strcmp(_fmt,RAW_FORMAT) == 0 ) 
		volume = new RawVolume(_root_dir);
	else {
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::setSrcVolume: unsupported volume format");
		throw MyException(err_msg);
	}

	channels = (volume->getCHANS()>1) ? 3 : 1; // only 1 or 3 channels supported

	if ( strcmp(_out_fmt,REAL_REPRESENTATION) == 0 ) {
		if ( channels > 1 ) {
			fprintf(stderr,"*** warning *** more than 1 channel, the internal representation has been changed\n");
			out_fmt = UINT8x3_REPRESENTATION;
			internal_rep = UINT8_INTERNAL_REP;
		}
		else {
			out_fmt = _out_fmt;
			internal_rep = REAL_INTERNAL_REP;
		}
	}
	else if ( strcmp(_out_fmt,UINT8_REPRESENTATION) == 0 ) { 
		out_fmt = _out_fmt;
		internal_rep = UINT8_INTERNAL_REP;
	}
	else if ( strcmp(_out_fmt,UINT8x3_REPRESENTATION) == 0 ) {
		out_fmt = _out_fmt;
		internal_rep = UINT8_INTERNAL_REP;
	}
	else {
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::setSrcVolume: unsupported output format (%s)",out_fmt);
		throw MyException(err_msg);
	}

	V0 = 0;
	H0 = 0;
	D0 = 0;
	V1 = volume->getDIM_V(); 
	H1 = volume->getDIM_H();
	D1 = volume->getDIM_D();
}


/*************************************************************************************************************
* Method to be called for tile generation. <> parameters are mandatory, while [] are optional.
* <output_path>			: absolute directory path where generted tiles have to be stored.
* [resolutions]			: pointer to an array of S_MAX_MULTIRES  size which boolean entries identify the acti-
*						  vaction/deactivation of the i-th resolution.  If not given, all resolutions will  be
*						  activated.
* [slice_height/width]	: desired dimensions of tiles  slices after merging.  It is actually an upper-bound of
*						  the actual slice dimensions, which will be computed in such a way that all tiles di-
*						  mensions can differ by 1 pixel only along both directions. If not given, the maximum
*						  allowed dimensions will be set, which will result in a volume composed by  one large 
*						  tile only.
* [seed]                : used to initiate random positions of objects
* [show_progress_bar]	: enables/disables progress bar with estimated time remaining.
* [saved_img_format]	: determines saved images format ("png","tif","jpeg", etc.).
* [saved_img_depth]		: determines saved images bitdepth (16 or 8).
**************************************************************************************************************/
void VolumeConverter::generateTiles(std::string output_path, bool* resolutions, 
				int slice_height, int slice_width, int seed, bool show_progress_bar, const char* saved_img_format, 
				int saved_img_depth)	throw (MyException)
{
	//LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
	REAL_T* rbuffer;			//buffer where temporary image data are stored (REAL_INTERNAL_REP)
	uint8** ubuffer;			//array of buffers where temporary image data of channels are stored (UINT8_INTERNAL_REP)
	//uint8*  ubuffer_ch2;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
	//uint8*  ubuffer_ch3;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
	int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
    REAL_T* stripe_up=NULL;		//will contain up-stripe and down-stripe computed by calling 'getStripe' method
	sint64 z_ratio, z_max_res;
    int n_stacks_V[S_MAX_MULTIRES], n_stacks_H[S_MAX_MULTIRES];             //array of number of tiles along V and H directions respectively at i-th resolution
    int **stacks_height[S_MAX_MULTIRES], **stacks_width[S_MAX_MULTIRES];	//array of matrices of tiles dimensions at i-th resolution
    std::stringstream file_path[S_MAX_MULTIRES];                            //array of root directory name at i-th resolution
	int resolutions_size = 0;

	if ( volume == 0 ) {
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTiles: undefined source volume");
		throw MyException(err_msg);
	}

	//initializing the progress bar
	char progressBarMsg[200];
	if(show_progress_bar)
	{
                   ProgressBar::getInstance()->start("Multiresolution tile generation");
                   ProgressBar::getInstance()->update(0,"Initializing...");
                   ProgressBar::getInstance()->show();
	}

	//computing dimensions of volume to be stitched
	//this->computeVolumeDims(exclude_nonstitchable_stacks, _ROW_START, _ROW_END, _COL_START, _COL_END, _D0, _D1);
	width = this->H1-this->H0;
	height = this->V1-this->V0;
	depth = this->D1-this->D0;

	//activating resolutions
        slice_height = (slice_height == -1 ? (int)height : slice_height);
        slice_width  = (slice_width  == -1 ? (int)width  : slice_width);
        if(slice_height < S_MIN_SLICE_DIM || slice_width < S_MIN_SLICE_DIM)
        {
            char err_msg[IM_STATIC_STRINGS_SIZE];
            sprintf(err_msg,"The minimum dimension for both slice width and height is %d", S_MIN_SLICE_DIM);
            throw MyException(err_msg);
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
            n_stacks_V[res_i] = (int) ceil ( (height/POW_INT(2,res_i)) / (float) slice_height );
            n_stacks_H[res_i] = (int) ceil ( (width/POW_INT(2,res_i))  / (float) slice_width  );
            stacks_height[res_i] = new int *[n_stacks_V[res_i]];
            stacks_width[res_i]  = new int *[n_stacks_V[res_i]];
            for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
            {
                stacks_height[res_i][stack_row] = new int[n_stacks_H[res_i]];
                stacks_width [res_i][stack_row] = new int[n_stacks_H[res_i]];
                for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
                {
                    stacks_height[res_i][stack_row][stack_col] = ((int)(height/POW_INT(2,res_i))) / n_stacks_V[res_i] + (stack_row < ((int)(height/POW_INT(2,res_i))) % n_stacks_V[res_i] ? 1:0);
                    stacks_width [res_i][stack_row][stack_col] = ((int)(width/POW_INT(2,res_i)))  / n_stacks_H[res_i] + (stack_col < ((int)(width/POW_INT(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
                }
            }
            //creating volume directory iff current resolution is selected and test mode is disabled
            if(resolutions[res_i] == true)
            {
                //creating directory that will contain image data at current resolution
                file_path[res_i]<<output_path<<"/RES("<<height/POW_INT(2,res_i)<<"x"<<width/POW_INT(2,res_i)<<"x"<<depth/POW_INT(2,res_i)<<")";
                if(make_dir(file_path[res_i].str().c_str())!=0)
                {
                    char err_msg[S_MAX_MULTIRES];
                    sprintf(err_msg, "in generateTiles(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                    throw MyException(err_msg);
                }
            }
	}

	//ALLOCATING  the MEMORY SPACE for image buffer
	z_max_res = POW_INT(2,resolutions_size-1);
	z_ratio=depth/z_max_res;

	//allocated even if not used
	ubuffer = new uint8 *[channels];
	memset(ubuffer,0,channels*sizeof(uint8));
	org_channels = channels; // save for checks

	for(sint64 z = this->D0, z_parts = 1; z < this->D1; z += z_max_res, z_parts++)
	{
		// fill one slice block
		if ( internal_rep == REAL_INTERNAL_REP )
			rbuffer = volume->loadSubvolume_to_REAL_T(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1);
		else { // internal_rep == UINT8_INTERNAL_REP
			ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1,&channels);
			if ( org_channels != channels ) {
				char err_msg[IM_STATIC_STRINGS_SIZE];
				sprintf(err_msg,"The volume contains images with a different number of channels (%d,%d)", org_channels, channels);
				throw MyException(err_msg);
			}
		
			if ( channels == 3 ) {
				// offsets are to be computed taking into account that buffer size along D may be different
				ubuffer[1] = ubuffer[0] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)));
				ubuffer[2] = ubuffer[1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)));
			}
			else if ( channels != 1 ) { // unsupported number of channels
				char err_msg[IM_STATIC_STRINGS_SIZE];
				sprintf(err_msg,"Unsupported number of channels (%d)", channels);
				throw MyException(err_msg);
			}
		}

		//updating the progress bar
		if(show_progress_bar)
		{	
			sprintf(progressBarMsg, "Generating slices from %d to %d og %d",((uint32)(z-D0)),((uint32)(z-D0+z_max_res-1)),(uint32)depth);
                        ProgressBar::getInstance()->update(((float)(z-D0+z_max_res-1)*100/(float)depth), progressBarMsg);
                        ProgressBar::getInstance()->show();
		}

		//saving current buffer data at selected resolutions and in multitile format
		for(int i=0; i< resolutions_size; i++)
		{
			if(show_progress_bar)
			{
				sprintf(progressBarMsg, "Generating resolution %d of %d",i+1,ISR_MAX(resolutions_size, resolutions_size));
                                ProgressBar::getInstance()->updateInfo(progressBarMsg);
                                ProgressBar::getInstance()->show();
			}

			//buffer size along D is different when the remainder of the subdivision by z_max_res is considered
			sint64 z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

			//halvesampling resolution if current resolution is not the deepest one
			if(i!=0)	
				if ( internal_rep == REAL_INTERNAL_REP )
					VirtualVolume::halveSample(rbuffer,(int)height/(POW_INT(2,i-1)),(int)width/(POW_INT(2,i-1)),(int)z_size/(POW_INT(2,i-1)));
				else // internal_rep == UINT8_INTERNAL_REP
					VirtualVolume::halveSample_UINT8(ubuffer,(int)height/(POW_INT(2,i-1)),(int)width/(POW_INT(2,i-1)),(int)z_size/(POW_INT(2,i-1)),channels);

			//saving at current resolution if it has been selected and iff buffer is at least 1 voxel (Z) deep
			if(resolutions[i] && (z_size/(POW_INT(2,i))) > 0)
			{
				if(show_progress_bar)
				{
					sprintf(progressBarMsg, "Saving to disc resolution %d",i+1);
                                        ProgressBar::getInstance()->updateInfo(progressBarMsg);
                                        ProgressBar::getInstance()->show();
				}

				//storing in 'base_path' the absolute path of the directory that will contain all stacks
				std::stringstream base_path;
                                base_path << output_path << "/RES(" << (int)(height/POW_INT(2,i)) << "x" << (int)(width/POW_INT(2,i)) << "x" << (int)(depth/POW_INT(2,i)) << ")/";

				//looping on new stacks
				for(int stack_row = 0, start_height = 0, end_height = 0; stack_row < n_stacks_V[i]; stack_row++)
				{
					//incrementing end_height
					end_height = start_height + stacks_height[i][stack_row][0]-1;
						
					//computing V_DIR_path and creating the directory the first time it is needed
					std::stringstream V_DIR_path;
					V_DIR_path << base_path.str() << this->getMultiresABS_V_string(i,start_height);
					if(z==D0 && make_dir(V_DIR_path.str().c_str())!=0)
					{
						char err_msg[S_STATIC_STRINGS_SIZE];
						sprintf(err_msg, "in mergeTiles(...): unable to create V_DIR = \"%s\"\n", V_DIR_path.str().c_str());
						throw MyException(err_msg);
					}

					for(int stack_column = 0, start_width=0, end_width=0; stack_column < n_stacks_H[i]; stack_column++)
					{
						end_width  = start_width  + stacks_width [i][stack_row][stack_column]-1;
							
						//computing H_DIR_path and creating the directory the first time it is needed
						std::stringstream H_DIR_path;
						H_DIR_path << V_DIR_path.str() << "/" << this->getMultiresABS_V_string(i,start_height) << "_" << this->getMultiresABS_H_string(i,start_width);
						if(z==D0 && make_dir(H_DIR_path.str().c_str())!=0)
						{
							char err_msg[S_STATIC_STRINGS_SIZE];
							sprintf(err_msg, "in mergeTiles(...): unable to create H_DIR = \"%s\"\n", H_DIR_path.str().c_str());
							throw MyException(err_msg);
						}

						//saving HERE
						for(int buffer_z=0; buffer_z<z_size/(POW_INT(2,i)); buffer_z++)
						{
							std::stringstream img_path;
							std::stringstream abs_pos_z;
							abs_pos_z.width(6);
							abs_pos_z.fill('0');
							abs_pos_z << (int)(POW_INT(2,i)*buffer_z+z);
							img_path << H_DIR_path.str() << "/" 
										<< this->getMultiresABS_V_string(i,start_height) << "_" 
										<< this->getMultiresABS_H_string(i,start_width) << "_"
										<< abs_pos_z.str(); 
							if ( internal_rep == REAL_INTERNAL_REP )
								VirtualVolume::saveImage(img_path.str(), 
									rbuffer + buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i)),
									(int)height/(POW_INT(2,i)),(int)width/(POW_INT(2,i)),
									start_height,end_height,start_width,end_width, 
									saved_img_format, saved_img_depth);
							else // internal_rep == UINT8_INTERNAL_REP
								if ( channels == 1 )
									VirtualVolume::saveImage_from_UINT8(img_path.str(), 
										ubuffer[0] + buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i)),
										(uint8 *) 0,
										(uint8 *) 0,
										(int)height/(POW_INT(2,i)),(int)width/(POW_INT(2,i)),
										start_height,end_height,start_width,end_width, 
										saved_img_format, saved_img_depth);
								else // channels = 3
									VirtualVolume::saveImage_from_UINT8(img_path.str(), 
										ubuffer[0] + buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i)),
										ubuffer[1] + buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i)),
										ubuffer[2] + buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i)),
										(int)height/(POW_INT(2,i)),(int)width/(POW_INT(2,i)),
										start_height,end_height,start_width,end_width, 
										saved_img_format, saved_img_depth);
						}
						start_width  += stacks_width [i][stack_row][stack_column];
					}
					start_height += stacks_height[i][stack_row][0];
				}
			}
		}

		//releasing allocated memory
		if ( internal_rep == REAL_INTERNAL_REP )
			delete rbuffer;
		else // internal_rep == UINT8_INTERNAL_REP
			delete ubuffer[0]; // other buffer pointers are only offsets
	}

	// reloads created volumes to generate .bin file descriptors at all resolutions
        for(int res_i=0; res_i< resolutions_size; res_i++)
            if(resolutions[res_i])
            {
                //---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
                //one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
                //is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
                //system.
                StackedVolume temp_vol(file_path[res_i].str().c_str(),ref_sys(axis(1),axis(2),axis(3)),
                                       volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));

//                StackedVolume temp_vol(file_path[res_i].str().c_str(),ref_sys(axis(1),axis(2),axis(3)), volume->getVXL_V()*(res_i+1),
//                                       volume->getVXL_H()*(res_i+1),volume->getVXL_D()*(res_i+1));
            }


	// ubuffer allocated anyway
	delete ubuffer;
}


/*************************************************************************************************************
* Functions used to obtain absolute coordinates at different resolutions from relative coordinates
**************************************************************************************************************/
int VolumeConverter::getMultiresABS_V(int res, int REL_V)
{
	if(volume->getVXL_V() > 0)
		return (int) ROUND(volume->getABS_V( V0 + REL_V*POW_INT(2,res) )*10);
	else
		return (int) ROUND(volume->getABS_V( V0 - 1 + REL_V*POW_INT(2,res))*10 + volume->getVXL_V()*POW_INT(2,res)*10);
}
std::string VolumeConverter::getMultiresABS_V_string(int res, int REL_V)	
{
	std::stringstream multires_merging_x_pos;
	multires_merging_x_pos.width(6);
	multires_merging_x_pos.fill('0');
	multires_merging_x_pos << this->getMultiresABS_V(res, REL_V);
	return multires_merging_x_pos.str();
}
int VolumeConverter::getMultiresABS_H(int res, int REL_H)
{
	if(volume->getVXL_H() > 0)
		return (int) ROUND(volume->getABS_H( H0 + REL_H*POW_INT(2,res) )*10);
	else
		return (int) ROUND(volume->getABS_H( H0 - 1 + REL_H*POW_INT(2,res))*10  + volume->getVXL_H()*POW_INT(2,res)*10);
}
std::string VolumeConverter::getMultiresABS_H_string(int res, int REL_H)	
{
	std::stringstream multires_merging_y_pos;
	multires_merging_y_pos.width(6);
	multires_merging_y_pos.fill('0');
	multires_merging_y_pos << this->getMultiresABS_H(res, REL_H);
	return multires_merging_y_pos.str();
}
