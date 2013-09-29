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

#include "VolumeConverter.h"
#include "../ImageManager/ProgressBar.h"
#include <math.h>

/*******************************************************************************************************
* Volume formats supported:
* 
* SimpleVolume:  simple sequence of slices stored as 2D images in the same directory
* StackedVolume: bidimensional matrix of 3D stacks stored in a hierarchical structure of directories
*
* WARNING: StackedVolume is already included by VolumeConverter.h
*******************************************************************************************************/
#include "../ImageManager/SimpleVolume.h"
#include "../ImageManager/SimpleVolumeRaw.h"
#include "../ImageManager/RawVolume.h"
#include "../ImageManager/TiledVolume.h"
#include "../ImageManager/TiledMCVolume.h"
/******************************************************************************************************/

#include <limits>
#include <list>
#include <stdlib.h>
#include <sstream>
#include <cstdio>

#include "check_and_makedir.h" //added by Hanchuan Peng, 20130915

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
	else if ( strcmp(_fmt,TILED_FORMAT) == 0 ) 
		volume = new TiledVolume(_root_dir,ref_sys(vertical,horizontal,depth),(float)1.0,(float)1.0,(float)1.0);
	else if ( strcmp(_fmt,TILED_MC_FORMAT) == 0 ) 
		volume = new TiledMCVolume(_root_dir,ref_sys(vertical,horizontal,depth),(float)1.0,(float)1.0,(float)1.0);
	else if ( strcmp(_fmt,SIMPLE_FORMAT) == 0 ) 
		volume = new SimpleVolume(_root_dir);
	else if ( strcmp(_fmt,SIMPLE_RAW_FORMAT) == 0 ) 
		volume = new SimpleVolumeRaw(_root_dir);
	else if ( strcmp(_fmt,RAW_FORMAT) == 0 ) 
		volume = new RawVolume(_root_dir);
	else {
        char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::setSrcVolume: unsupported volume format");
		throw MyException(err_msg);
	}

	//channels = (volume->getCHANS()>1) ? 3 : 1; // only 1 or 3 channels supported
	channels = volume->getCHANS();

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
* [show_progress_bar]	: enables/disables progress bar with estimated time remaining.
* [saved_img_format]	: determines saved images format ("png","tif","jpeg", etc.).
* [saved_img_depth]		: determines saved images bitdepth (16 or 8).
**************************************************************************************************************/
void VolumeConverter::generateTiles(std::string output_path, bool* resolutions, 
				int slice_height, int slice_width, int method, bool show_progress_bar, const char* saved_img_format, 
				int saved_img_depth)	throw (MyException)
{
    printf("in VolumeConverter::generateTiles(path = \"%s\", resolutions = ", output_path.c_str());
    for(int i=0; i< S_MAX_MULTIRES; i++)
        printf("%d", resolutions[i]);
    printf(", slice_height = %d, slice_width = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d)\n",
           slice_height, slice_width, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth);

	if ( saved_img_depth == 0 ) // Tiff2DStck currently supports only 8 bits depth 
		saved_img_depth = 8;
		
	if ( saved_img_depth != 8 ) {
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: mismatch between bits per channel of source (%d) and destination (%d)",
			volume->getBYTESxCHAN() * 8, saved_img_depth);
		throw MyException(err_msg);
	}
	
	if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) {
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: mismatch between bits per channel of source (%d) and destination (%d)",
			volume->getBYTESxCHAN() * 8, saved_img_depth);
		throw MyException(err_msg);
	}
	
	//LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
	REAL_T* rbuffer;			//buffer where temporary image data are stored (REAL_INTERNAL_REP)
	uint8** ubuffer;			//array of buffers where temporary image data of channels are stored (UINT8_INTERNAL_REP)
	int bytes_chan = volume->getBYTESxCHAN();
	//uint8*  ubuffer_ch2;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
	//uint8*  ubuffer_ch3;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
	int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
	int supported_channels;     //channels to be supported (stacks of tiffs 2D only supports 1 or 3 channels)
    // REAL_T* stripe_up=NULL;		//will contain up-stripe and down-stripe computed by calling 'getStripe' method (unused)
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
                //if(make_dir(file_path[res_i].str().c_str())!=0)
                if(!check_and_make_dir(file_path[res_i].str().c_str())) // HP 130914
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

	// check the number of channels
	if ( channels > 3 ) {
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"The volume contains too many channels (%d)", channels);
		throw MyException(err_msg);
	}

	//allocated even if not used
	org_channels = channels; // save for checks
	supported_channels = (channels>1) ? 3 : 1; // only 1 or 3 channels supported if output format is stacks of tiffs 2D
	ubuffer = new uint8 *[supported_channels];
	memset(ubuffer,0,supported_channels*sizeof(uint8 *)); // initializes to null pointers

	for(sint64 z = this->D0, z_parts = 1; z < this->D1; z += z_max_res, z_parts++)
	{
		// fill one slice block
		if ( internal_rep == REAL_INTERNAL_REP )
			rbuffer = volume->loadSubvolume_to_REAL_T(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1);
		else { // internal_rep == UINT8_INTERNAL_REP
			ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1,&channels,IM_NATIVE_RTYPE);
			// WARNING: next code assumes that channels is 1 or 3, but implementations of loadSubvolume_to_UINT8 do not guarantee this condition
			if ( org_channels != channels ) {
				char err_msg[IM_STATIC_STRINGS_SIZE];
				sprintf(err_msg,"The volume contains images with a different number of channels (%d,%d)", org_channels, channels);
				throw MyException(err_msg);
			}
		
			// code has been changed because the load operation can return 1, 2 or 3 channels
			/*
			if ( supported_channels == 3 ) {
				// offsets are to be computed taking into account that buffer size along D may be different
				ubuffer[1] = ubuffer[0] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)));
				ubuffer[2] = ubuffer[1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)));
			}
			*/
			// elements of ubuffer not set are null pointers
			for ( int c=1; c<channels; c++ )
				ubuffer[c] = ubuffer[c-1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)) * bytes_chan);
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
					VirtualVolume::halveSample(rbuffer,(int)height/(POW_INT(2,i-1)),(int)width/(POW_INT(2,i-1)),(int)z_size/(POW_INT(2,i-1)),method);
				else // internal_rep == UINT8_INTERNAL_REP
					VirtualVolume::halveSample_UINT8(ubuffer,(int)height/(POW_INT(2,i-1)),(int)width/(POW_INT(2,i-1)),(int)z_size/(POW_INT(2,i-1)),channels,method,bytes_chan);

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
                    if(z==D0 && !check_and_make_dir(V_DIR_path.str().c_str()))
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
                        if(z==D0 && !check_and_make_dir(H_DIR_path.str().c_str()))
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
							abs_pos_z << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
												(POW_INT(2,i)*buffer_z+z) * volume->getVXL_D());
							img_path << H_DIR_path.str() << "/" 
										<< this->getMultiresABS_V_string(i,start_height) << "_" 
										<< this->getMultiresABS_H_string(i,start_width) << "_"
										<< abs_pos_z.str(); 
							if ( internal_rep == REAL_INTERNAL_REP )
								VirtualVolume::saveImage(img_path.str(), 
									rbuffer + buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i)), // adds the stride
									(int)height/(POW_INT(2,i)),(int)width/(POW_INT(2,i)),
									start_height,end_height,start_width,end_width, 
									saved_img_format, saved_img_depth);
							else // internal_rep == UINT8_INTERNAL_REP
								if ( channels == 1 )
									VirtualVolume::saveImage_from_UINT8(img_path.str(), 
										ubuffer[0] + buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i)), // adds the stride
										(uint8 *) 0,
										(uint8 *) 0,
										(int)height/(POW_INT(2,i)),(int)width/(POW_INT(2,i)),
										start_height,end_height,start_width,end_width, 
										saved_img_format, saved_img_depth);
								else if ( channels == 2 ) 
									VirtualVolume::saveImage_from_UINT8(img_path.str(), 
										ubuffer[0] + buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i))*bytes_chan, // stride to be added for slice buffer_z
										ubuffer[1] + buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i))*bytes_chan, // stride to be added for slice buffer_z
										(uint8 *) 0,
										(int)height/(POW_INT(2,i)),(int)width/(POW_INT(2,i)),
										start_height,end_height,start_width,end_width, 
										saved_img_format, saved_img_depth);
								else // channels = 3
									VirtualVolume::saveImage_from_UINT8(img_path.str(), 
										ubuffer[0] + buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i))*bytes_chan, // stride to be added for slice buffer_z
										ubuffer[1] + buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i))*bytes_chan, // stride to be added for slice buffer_z
										ubuffer[2] + buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i))*bytes_chan, // stride to be added for slice buffer_z
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
	ref_sys reference(axis(1),axis(2),axis(3));
	StackedVolume *probe = dynamic_cast<StackedVolume *>(volume);
	if ( probe ) {
		reference.first  = probe->getAXS_1();
		reference.second = probe->getAXS_2();
		reference.third  = probe->getAXS_3();
	}
	for(int res_i=0; res_i< resolutions_size; res_i++) {
		if(resolutions[res_i])
        {
            //---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
            //one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
            //is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
            //system.
			//---- Giulio 2013-08-23 fixed
			StackedVolume temp_vol(file_path[res_i].str().c_str(),reference,
							volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));

//			StackedVolume temp_vol(file_path[res_i].str().c_str(),ref_sys(axis(1),axis(2),axis(3)), volume->getVXL_V()*(res_i+1),
//							volume->getVXL_H()*(res_i+1),volume->getVXL_D()*(res_i+1));
        }
	}


	// ubuffer allocated anyway
	delete ubuffer;

	// deallocate memory
    for(int res_i=0; res_i< resolutions_size; res_i++)
	{
		for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
		{
			delete []stacks_height[res_i][stack_row];
			delete []stacks_width [res_i][stack_row];
		}
		delete []stacks_height[res_i];
		delete []stacks_width[res_i]; 
	}
}


/*************************************************************************************************************
* Method to be called for tile generation in Vaa3D raw format. <> parameters are mandatory, while [] are optional.
* <output_path>			: absolute directory path where generted tiles have to be stored.
* [resolutions]			: pointer to an array of S_MAX_MULTIRES  size which boolean entries identify the acti-
*						  vaction/deactivation of the i-th resolution.  If not given, all resolutions will  be
*						  activated.
* [block_height]	    : desired dimensions of tiled  blocks after merging.  It is actually an upper-bound of
* [block_width]			  the actual slice dimensions, which will be computed in such a way that all tiles di-
* [block_depth]			  mensions can differ by 1 pixel only along both directions. If not given, the maximum
*						  allowed dimensions will be set, which will result in a volume composed by  one large 
*						  tile only.
* [show_progress_bar]	: enables/disables progress bar with estimated time remaining.
* [saved_img_format]	: determines saved images format ("png","tif","jpeg", etc.).
* [saved_img_depth]		: determines saved images bitdepth (16 or 8).
**************************************************************************************************************/
void VolumeConverter::generateTilesVaa3DRaw(std::string output_path, bool* resolutions, 
				int block_height, int block_width, int block_depth, int method, 
				bool show_progress_bar, const char* saved_img_format, 
				int saved_img_depth)	throw (MyException)
{
    printf("in VolumeConverter::generateTilesVaa3DRaw(path = \"%s\", resolutions = ", output_path.c_str());
    for(int i=0; i< S_MAX_MULTIRES; i++)
        printf("%d", resolutions[i]);
    printf(", block_height = %d, block_width = %d, block_depth = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d)\n",
           block_height, block_width, block_depth, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth);

	if ( saved_img_depth == 0 ) // default is to generate an image with the same depth of the source
		saved_img_depth = volume->getBYTESxCHAN() * 8;
		
	if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) {
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: mismatch between bits per channel of source (%d) and destination (%d)",
			volume->getBYTESxCHAN() * 8, saved_img_depth);
		throw MyException(err_msg);
	}

	//LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
	REAL_T* rbuffer;			//buffer where temporary image data are stored (REAL_INTERNAL_REP)
	uint8** ubuffer;			//array of buffers where temporary image data of channels are stored (UINT8_INTERNAL_REP)
	int bytes_chan = volume->getBYTESxCHAN();
	//uint8*  ubuffer_ch2;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
	//uint8*  ubuffer_ch3;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
	int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
    //REAL_T* stripe_up=NULL;		//will contain up-stripe and down-stripe computed by calling 'getStripe' method (unused)
	sint64 z_ratio, z_max_res;
    int n_stacks_V[S_MAX_MULTIRES];        //arrays of number of tiles along V, H and D directions respectively at i-th resolution
    int n_stacks_H[S_MAX_MULTIRES];
	int n_stacks_D[S_MAX_MULTIRES];  
    int ***stacks_height[S_MAX_MULTIRES];   //array of matrices of tiles dimensions at i-th resolution
	int ***stacks_width[S_MAX_MULTIRES];	
	int ***stacks_depth[S_MAX_MULTIRES];
    std::stringstream file_path[S_MAX_MULTIRES];  //array of root directory name at i-th resolution
	int resolutions_size = 0;

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

	if ( volume == 0 ) {
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: undefined source volume");
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

	// code for testing
	//uint8 *temp = volume->loadSubvolume_to_UINT8(
	//	10,height-10,10,width-10,10,depth-10,
	//	&channels);

	//activating resolutions
    block_height = (block_height == -1 ? (int)height : block_height);
    block_width  = (block_width  == -1 ? (int)width  : block_width);
    block_depth  = (block_depth  == -1 ? (int)depth  : block_depth);
    if(block_height < S_MIN_SLICE_DIM || block_width < S_MIN_SLICE_DIM || block_depth < S_MIN_SLICE_DIM)
    { 
        char err_msg[IM_STATIC_STRINGS_SIZE];
        sprintf(err_msg,"The minimum dimension for block height, width, and depth is %d", S_MIN_SLICE_DIM);
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
						stacks_width[res_i][stack_row][stack_col][stack_sli] = 
							((int)(width/POW_INT(2,res_i)))  / n_stacks_H[res_i] + (stack_col < ((int)(width/POW_INT(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
						stacks_depth[res_i][stack_row][stack_col][stack_sli] = 
							((int)(depth/POW_INT(2,res_i)))  / n_stacks_D[res_i] + (stack_sli < ((int)(depth/POW_INT(2,res_i)))  % n_stacks_D[res_i] ? 1:0);
					}
                }
            }
            //creating volume directory iff current resolution is selected and test mode is disabled
            if(resolutions[res_i] == true)
            {
                //creating directory that will contain image data at current resolution
                file_path[res_i]<<output_path<<"/RES("<<height/POW_INT(2,res_i)<<"x"<<width/POW_INT(2,res_i)<<"x"<<depth/POW_INT(2,res_i)<<")";
                //if(make_dir(file_path[res_i].str().c_str())!=0)
                if(!check_and_make_dir(file_path[res_i].str().c_str())) // HP 130914
                {
                    char err_msg[S_MAX_MULTIRES];
                    sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
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

	//slice_start and slice_end of current block depend on the resolution
	for(int res_i=0; res_i< resolutions_size; res_i++) {
		stack_block[res_i] = 0;
		slice_start[res_i] = this->D0; 
		slice_end[res_i] = slice_start[res_i] + stacks_depth[res_i][0][0][0] - 1;
	}

	for(sint64 z = this->D0, z_parts = 1; z < this->D1; z += z_max_res, z_parts++)
	{
		// fill one slice block
		if ( internal_rep == REAL_INTERNAL_REP )
			rbuffer = volume->loadSubvolume_to_REAL_T(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1);
		else { // internal_rep == UINT8_INTERNAL_REP
			ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1,&channels,IM_NATIVE_RTYPE);
			if ( org_channels != channels ) {
				char err_msg[IM_STATIC_STRINGS_SIZE];
				sprintf(err_msg,"The volume contains images with a different number of channels (%d,%d)", org_channels, channels);
				throw MyException(err_msg);
			}
		
			for (int i=1; i<channels; i++ ) { // WARNING: assume 1-byte pixels
				// offsets are to be computed taking into account that buffer size along D may be different
				// WARNING: the offset must be of tipe sint64 
				ubuffer[i] = ubuffer[i-1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)) * bytes_chan);
			}
		}
		// WARNING: should check that buffer has been actually allocated

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
			abs_pos_z << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
								(POW_INT(2,i)*slice_start[i]) * volume->getVXL_D());

			//compute the number of slice of previous groups at resolution i
			//note that z_parts in the number and not an index (starts from 1)
			n_slices_pred  = (z_parts - 1) * z_max_res / POW_INT(2,i);

			//buffer size along D is different when the remainder of the subdivision by z_max_res is considered
			sint64 z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

			//halvesampling resolution if current resolution is not the deepest one
			if(i!=0)	
				if ( internal_rep == REAL_INTERNAL_REP )
					VirtualVolume::halveSample(rbuffer,(int)height/(POW_INT(2,i-1)),(int)width/(POW_INT(2,i-1)),(int)z_size/(POW_INT(2,i-1)),method);
				else // internal_rep == UINT8_INTERNAL_REP
					VirtualVolume::halveSample_UINT8(ubuffer,(int)height/(POW_INT(2,i-1)),(int)width/(POW_INT(2,i-1)),(int)z_size/(POW_INT(2,i-1)),channels,method,bytes_chan);

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
					end_height = start_height + stacks_height[i][stack_row][0][0]-1; 
						
					//computing V_DIR_path and creating the directory the first time it is needed
					std::stringstream V_DIR_path;
					V_DIR_path << base_path.str() << this->getMultiresABS_V_string(i,start_height);
                    if(z==D0 && !check_and_make_dir(V_DIR_path.str().c_str()))
					{
						char err_msg[S_STATIC_STRINGS_SIZE];
						sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create V_DIR = \"%s\"\n", V_DIR_path.str().c_str());
						throw MyException(err_msg);
					}

					for(int stack_column = 0, start_width=0, end_width=0; stack_column < n_stacks_H[i]; stack_column++)
					{
						end_width  = start_width  + stacks_width [i][stack_row][stack_column][0]-1; 
							
						//computing H_DIR_path and creating the directory the first time it is needed
						std::stringstream H_DIR_path;
						H_DIR_path << V_DIR_path.str() << "/" << this->getMultiresABS_V_string(i,start_height) << "_" << this->getMultiresABS_H_string(i,start_width);
						if ( z==D0 ) {
                            if (!check_and_make_dir(H_DIR_path.str().c_str()))
							{
								char err_msg[S_STATIC_STRINGS_SIZE];
								sprintf(err_msg, "in generateTilesVaa3DRaw(...): unable to create H_DIR = \"%s\"\n", H_DIR_path.str().c_str());
								throw MyException(err_msg);
							}
							else { // the directory has been created for the first time
								   // initialize block files
								V3DLONG *sz = new V3DLONG[4];
								int datatype;
								char *err_rawfmt;

								sz[0] = stacks_width[i][stack_row][stack_column][0];
								sz[1] = stacks_height[i][stack_row][stack_column][0];
								sz[3] = channels;

								if ( internal_rep == REAL_INTERNAL_REP )
									datatype = 4;
								else if ( internal_rep == UINT8_INTERNAL_REP ) {
									if ( saved_img_depth == 16 )
										datatype = 2;
									else if ( saved_img_depth == 8 ) 
										datatype = 1;
									else {
										char err_msg[S_STATIC_STRINGS_SIZE];
										sprintf(err_msg, "in generateTilesVaa3DRaw(...): unknown image depth (%d)", saved_img_depth);
										throw MyException(err_msg);
									}
								}
								else {
									char err_msg[S_STATIC_STRINGS_SIZE];
									sprintf(err_msg, "in generateTilesVaa3DRaw(...): unknown internal representation (%d)", internal_rep);
									throw MyException(err_msg);
								}

								int slice_start_temp = 0;
								for ( int j=0; j < n_stacks_D[i]; j++ ) {
									sz[2] = stacks_depth[i][stack_row][stack_column][j];

									std::stringstream abs_pos_z_temp;
									abs_pos_z_temp.width(6);
									abs_pos_z_temp.fill('0');
									abs_pos_z_temp << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
										(POW_INT(2,i)*(slice_start_temp)) * volume->getVXL_D());

									std::stringstream img_path_temp;
									img_path_temp << H_DIR_path.str() << "/" 
												  << this->getMultiresABS_V_string(i,start_height) << "_" 
												  << this->getMultiresABS_H_string(i,start_width) << "_"
												  << abs_pos_z_temp.str();

									if ( (err_rawfmt = initRawFile((char *)img_path_temp.str().c_str(),sz,datatype)) != 0 ) {
										char err_msg[IM_STATIC_STRINGS_SIZE];
										sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: error in initializing block file - %s", err_rawfmt);
										throw MyException(err_msg);
									};

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
							img_path << H_DIR_path.str() << "/" 
										<< this->getMultiresABS_V_string(i,start_height) << "_" 
										<< this->getMultiresABS_H_string(i,start_width) << "_";
							if ( (z/POW_INT(2,i)+buffer_z) > slice_end[i] ) { // start a new block along z
								abs_pos_z_next.width(6);
								abs_pos_z_next.fill('0');
								abs_pos_z_next << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
										(POW_INT(2,i)*(slice_end[i]+1)) * volume->getVXL_D());
								img_path << abs_pos_z_next.str();
								slice_ind = (int)(n_slices_pred - (slice_end[i]+1)) + buffer_z;
							}
							else {
								img_path << abs_pos_z.str(); 
								slice_ind = (int)(n_slices_pred - slice_start[i]) + buffer_z;
							}

							if ( internal_rep == REAL_INTERNAL_REP )
								VirtualVolume::saveImage_to_Vaa3DRaw(
									slice_ind,
									img_path.str(), 
									rbuffer + buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i)), // adds the stride
									(int)height/(POW_INT(2,i)),(int)width/(POW_INT(2,i)),
									start_height,end_height,start_width,end_width, 
									saved_img_format, saved_img_depth
								);
							else // internal_rep == UINT8_INTERNAL_REP
								VirtualVolume::saveImage_from_UINT8_to_Vaa3DRaw(
									slice_ind,
									img_path.str(), 
									ubuffer,
									channels,
									buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i))*bytes_chan,  // stride to be added for slice buffer_z
									(int)height/(POW_INT(2,i)),(int)width/(POW_INT(2,i)),
									start_height,end_height,start_width,end_width, 
									saved_img_format, saved_img_depth);
					}
						start_width  += stacks_width [i][stack_row][stack_column][0]; // WARNING TO BE CHECKED FOR CORRECTNESS
					}
					start_height += stacks_height[i][stack_row][0][0]; // WARNING TO BE CHECKED FOR CORRECTNESS
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
	ref_sys reference(axis(1),axis(2),axis(3));
	TiledVolume   *tprobe;
	StackedVolume *sprobe;
	int n_err = 0; 
	sprobe = dynamic_cast<StackedVolume *>(volume);
	if ( sprobe ) {
		reference.first  = sprobe->getAXS_1();
		reference.second = sprobe->getAXS_2();
		reference.third  = sprobe->getAXS_3();
	}
	else {
		tprobe = dynamic_cast<TiledVolume *>(volume);
		if ( tprobe ) {
			reference.first  = tprobe->getAXS_1();
			reference.second = tprobe->getAXS_2();
			reference.third  = tprobe->getAXS_3();
		}
	}
    for(int res_i=0; res_i< resolutions_size; res_i++)
	{
        if(resolutions[res_i])
        {
            //---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
            //one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
            //is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
            //system.
			try {
				TiledVolume temp_vol(file_path[res_i].str().c_str(),reference,
						volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));
			}
			catch ( ... ) {
				printf("\t\t\t\tin VolumeConverter::generateTilesVaa3DRaw: cannot be created file mdata.bin in %s\n\n",file_path[res_i].str().c_str());
				n_err++;
			}

//          StackedVolume temp_vol(file_path[res_i].str().c_str(),ref_sys(axis(1),axis(2),axis(3)), volume->getVXL_V()*(res_i+1),
//                      volume->getVXL_H()*(res_i+1),volume->getVXL_D()*(res_i+1));
        }
	}


	// ubuffer allocated anyway
	delete ubuffer;

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

	if ( n_err ) { // errors in mdat.bin creation
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: %d errors in creating mdata.bin files", n_err);
		throw MyException(err_msg);
	}
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
int VolumeConverter::getMultiresABS_D(int res)
{
	if(volume->getVXL_D() > 0)
		return (int) ROUND(volume->getABS_D(D0)*10);
	else
		return (int) ROUND(volume->getABS_D((int)(D0 - 1 + volume->getVXL_D()*POW_INT(2,res)*10)));
}



/*************************************************************************************************************
* NEW TILED FORMAT SUPPORTING MULTIPLE CHANNELS
**************************************************************************************************************/

# ifdef RES_IN_CHANS // resolutions directories in channels directories

void VolumeConverter::generateTilesVaa3DRawMC ( std::string output_path, bool* resolutions, 
				int block_height, int block_width, int block_depth, int method, 
				bool show_progress_bar, const char* saved_img_format, 
				int saved_img_depth )	throw (MyException)
{
    printf("in VolumeConverter::generateTilesVaa3DRawMC(path = \"%s\", resolutions = ", output_path.c_str());
    for(int i=0; i< S_MAX_MULTIRES; i++)
        printf("%d", resolutions[i]);
    printf(", block_height = %d, block_width = %d, block_depth = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d)\n",
           block_height, block_width, block_depth, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth);

	if ( saved_img_depth == 0 ) // default is to generate an image with the same depth of the source
		saved_img_depth = volume->getBYTESxCHAN() * 8;
		
	if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) {
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: mismatch between bits per channel of source (%d) and destination (%d)",
			volume->getBYTESxCHAN() * 8, saved_img_depth);
		throw MyException(err_msg);
	}

	//LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
	REAL_T* rbuffer;			//buffer where temporary image data are stored (REAL_INTERNAL_REP)
	uint8** ubuffer;			//array of buffers where temporary image data of channels are stored (UINT8_INTERNAL_REP)
	int bytes_chan = volume->getBYTESxCHAN();
	//uint8*  ubuffer_ch2;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
	//uint8*  ubuffer_ch3;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
	int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
    //REAL_T* stripe_up=NULL;		//will contain up-stripe and down-stripe computed by calling 'getStripe' method (unused)
	sint64 z_ratio, z_max_res;
    int n_stacks_V[S_MAX_MULTIRES];        //arrays of number of tiles along V, H and D directions respectively at i-th resolution
    int n_stacks_H[S_MAX_MULTIRES];
	int n_stacks_D[S_MAX_MULTIRES];  
    int ***stacks_height[S_MAX_MULTIRES];   //array of matrices of tiles dimensions at i-th resolution
	int ***stacks_width[S_MAX_MULTIRES];	
	int ***stacks_depth[S_MAX_MULTIRES];
	std::string *chans_dir;
	std::string resolution_dir;
    std::stringstream file_path[S_MAX_MULTIRES];                            //array of root directory name at i-th resolution
	int resolutions_size = 0;

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

	if ( volume == 0 ) {
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRawMC: undefined source volume");
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

	// code for testing
	//uint8 *temp = volume->loadSubvolume_to_UINT8(
	//	10,height-10,10,width-10,10,depth-10,
	//	&channels);

	//activating resolutions
    block_height = (block_height == -1 ? (int)height : block_height);
    block_width  = (block_width  == -1 ? (int)width  : block_width);
    block_depth  = (block_depth  == -1 ? (int)depth  : block_depth);
    if(block_height < S_MIN_SLICE_DIM || block_width < S_MIN_SLICE_DIM || block_depth < S_MIN_SLICE_DIM)
    { 
        char err_msg[IM_STATIC_STRINGS_SIZE];
        sprintf(err_msg,"The minimum dimension for block height, width, and depth is %d", S_MIN_SLICE_DIM);
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

	//computing tiles dimensions at each resolution
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
					stacks_width[res_i][stack_row][stack_col][stack_sli] = 
						((int)(width/POW_INT(2,res_i)))  / n_stacks_H[res_i] + (stack_col < ((int)(width/POW_INT(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
					stacks_depth[res_i][stack_row][stack_col][stack_sli] = 
						((int)(depth/POW_INT(2,res_i)))  / n_stacks_D[res_i] + (stack_sli < ((int)(depth/POW_INT(2,res_i)))  % n_stacks_D[res_i] ? 1:0);
				}
			}
		}
	}

	// computing resolutions directory names
	for(int res_i=0; res_i< resolutions_size; res_i++) {
		//creating volume directory iff current resolution is selected and test mode is disabled
		if(resolutions[res_i] == true) {
			file_path[res_i] << "/RES("<<height/POW_INT(2,res_i) 
							 << "x" << width/POW_INT(2,res_i) 
							 << "x" << depth/POW_INT(2,res_i) << ")";
		}
	}

	// computing channel directory names
	chans_dir = new std::string[channels];
	int n_digits = 1;
	int _channels = channels / 10;	
	while ( _channels ) {
		n_digits++;
		_channels /= 10;
	}
	for ( int c=0; c<channels; c++ ) {
 		std::stringstream dir_name;
		dir_name.width(n_digits);
		dir_name.fill('0');
		dir_name << c;
		chans_dir[c] = output_path + "/" + IM_CHANNEL_PREFIX + dir_name.str();
		//if(make_dir(chans_dir[c].c_str())!=0) 
		if(!check_and_make_dir(chans_dir[c].c_str())) { // HP 130914
		{
			char err_msg[S_MAX_MULTIRES];
			sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create DIR = \"%s\"\n", chans_dir[c].c_str());
			throw MyException(err_msg);
		}
		for(int res_i=0; res_i< resolutions_size; res_i++) {
			//creating volume directory iff current resolution is selected and test mode is disabled
			if(resolutions[res_i] == true) {
				//creating directory that will contain image data at current resolution
				//resolution_dir = chans_dir[c] + file_path[res_i].str();
				resolution_dir = file_path[res_i].str() + chans_dir[c];
				//if(make_dir(resolution_dir.c_str())!=0)
                if(!check_and_make_dir(resolution_dir.c_str())) // HP 130914
				{
					char err_msg[S_MAX_MULTIRES];
					sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
					throw MyException(err_msg);
				}
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

	//slice_start and slice_end of current block depend on the resolution
	for(int res_i=0; res_i< resolutions_size; res_i++) {
		stack_block[res_i] = 0;
		slice_start[res_i] = this->D0; 
		slice_end[res_i] = slice_start[res_i] + stacks_depth[res_i][0][0][0] - 1;
	}

	for(sint64 z = this->D0, z_parts = 1; z < this->D1; z += z_max_res, z_parts++)
	{
		// fill one slice block
		if ( internal_rep == REAL_INTERNAL_REP )
			rbuffer = volume->loadSubvolume_to_REAL_T(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1);
		else { // internal_rep == UINT8_INTERNAL_REP
			ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1,&channels,IM_NATIVE_RTYPE);
			if ( org_channels != channels ) {
				char err_msg[IM_STATIC_STRINGS_SIZE];
				sprintf(err_msg,"The volume contains images with a different number of channels (%d,%d)", org_channels, channels);
				throw MyException(err_msg);
			}
		
			for (int i=1; i<channels; i++ ) { // WARNING: assume 1-byte pixels
				// offsets are to be computed taking into account that buffer size along D may be different
				// WARNING: the offset must be of tipe sint64 
				ubuffer[i] = ubuffer[i-1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)) * bytes_chan);
			}
		}
		// WARNING: should check that buffer has been actually allocated

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
			abs_pos_z << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
								(POW_INT(2,i)*slice_start[i]) * volume->getVXL_D());

			//compute the number of slice of previous groups at resolution i
			//note that z_parts in the number and not an index (starts from 1)
			n_slices_pred  = (z_parts - 1) * z_max_res / POW_INT(2,i);

			//buffer size along D is different when the remainder of the subdivision by z_max_res is considered
			sint64 z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

			//halvesampling resolution if current resolution is not the deepest one
			if(i!=0)	
				if ( internal_rep == REAL_INTERNAL_REP )
					VirtualVolume::halveSample(rbuffer,(int)height/(POW_INT(2,i-1)),(int)width/(POW_INT(2,i-1)),(int)z_size/(POW_INT(2,i-1)),method);
				else // internal_rep == UINT8_INTERNAL_REP
					VirtualVolume::halveSample_UINT8(ubuffer,(int)height/(POW_INT(2,i-1)),(int)width/(POW_INT(2,i-1)),(int)z_size/(POW_INT(2,i-1)),channels,method,bytes_chan);

			//saving at current resolution if it has been selected and iff buffer is at least 1 voxel (Z) deep
			if(resolutions[i] && (z_size/(POW_INT(2,i))) > 0)
			{
				if(show_progress_bar)
				{
					sprintf(progressBarMsg, "Saving to disc resolution %d",i+1);
                                        ProgressBar::getInstance()->updateInfo(progressBarMsg);
                                        ProgressBar::getInstance()->show();
				}

				for ( int c=0; c<channels; c++ ) {

					//storing in 'base_path' the absolute path of the directory that will contain all stacks
					std::stringstream base_path;
									base_path << chans_dir[c] << "/RES(" << (int)(height/POW_INT(2,i)) << "x" << (int)(width/POW_INT(2,i)) << "x" << (int)(depth/POW_INT(2,i)) << ")/";

					//looping on new stacks
					for(int stack_row = 0, start_height = 0, end_height = 0; stack_row < n_stacks_V[i]; stack_row++)
					{
						//incrementing end_height
						end_height = start_height + stacks_height[i][stack_row][0][0]-1; 
							
						//computing V_DIR_path and creating the directory the first time it is needed
						std::stringstream V_DIR_path;
						V_DIR_path << base_path.str() << this->getMultiresABS_V_string(i,start_height);
                        if(z==D0 && !check_and_make_dir(V_DIR_path.str().c_str()))
						{
							char err_msg[S_STATIC_STRINGS_SIZE];
							sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create V_DIR = \"%s\"\n", V_DIR_path.str().c_str());
							throw MyException(err_msg);
						}

						for(int stack_column = 0, start_width=0, end_width=0; stack_column < n_stacks_H[i]; stack_column++)
						{
							end_width  = start_width  + stacks_width [i][stack_row][stack_column][0]-1; 
								
							//computing H_DIR_path and creating the directory the first time it is needed
							std::stringstream H_DIR_path;
							H_DIR_path << V_DIR_path.str() << "/" << this->getMultiresABS_V_string(i,start_height) << "_" << this->getMultiresABS_H_string(i,start_width);
							if ( z==D0 ) {
                                if (!check_and_make_dir(H_DIR_path.str().c_str()))
								{
									char err_msg[S_STATIC_STRINGS_SIZE];
									sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create H_DIR = \"%s\"\n", H_DIR_path.str().c_str());
									throw MyException(err_msg);
								}
								else { // the directory has been created for the first time
									   // initialize block files
									V3DLONG *sz = new V3DLONG[4];
									int datatype;
									char *err_rawfmt;

									sz[0] = stacks_width[i][stack_row][stack_column][0];
									sz[1] = stacks_height[i][stack_row][stack_column][0];
									sz[3] = 1; // single channel files

									if ( internal_rep == REAL_INTERNAL_REP )
										datatype = 4;
									else if ( internal_rep == UINT8_INTERNAL_REP ) {
										if ( saved_img_depth == 16 )
											datatype = 2;
										else if ( saved_img_depth == 8 ) 
											datatype = 1;
										else {
											char err_msg[S_STATIC_STRINGS_SIZE];
											sprintf(err_msg, "in generateTilesVaa3DRaw(...): unknown image depth (%d)", saved_img_depth);
											throw MyException(err_msg);
										}
									}
									else {
										char err_msg[S_STATIC_STRINGS_SIZE];
										sprintf(err_msg, "in generateTilesVaa3DRaw(...): unknown internal representation (%d)", internal_rep);
										throw MyException(err_msg);
									}

									int slice_start_temp = 0;
									for ( int j=0; j < n_stacks_D[i]; j++ ) {
										sz[2] = stacks_depth[i][stack_row][stack_column][j];

										std::stringstream abs_pos_z_temp;
										abs_pos_z_temp.width(6);
										abs_pos_z_temp.fill('0');
										abs_pos_z_temp << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
											(POW_INT(2,i)*(slice_start_temp)) * volume->getVXL_D());

										std::stringstream img_path_temp;
										img_path_temp << H_DIR_path.str() << "/" 
													  << this->getMultiresABS_V_string(i,start_height) << "_" 
													  << this->getMultiresABS_H_string(i,start_width) << "_"
													  << abs_pos_z_temp.str();

										if ( (err_rawfmt = initRawFile((char *)img_path_temp.str().c_str(),sz,datatype)) != 0 ) {
											char err_msg[IM_STATIC_STRINGS_SIZE];
											sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRawMC: error in initializing block file - %s", err_rawfmt);
											throw MyException(err_msg);
										};

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
								img_path << H_DIR_path.str() << "/" 
											<< this->getMultiresABS_V_string(i,start_height) << "_" 
											<< this->getMultiresABS_H_string(i,start_width) << "_";
								if ( (z/POW_INT(2,i)+buffer_z) > slice_end[i] ) { // start a new block along z
									abs_pos_z_next.width(6);
									abs_pos_z_next.fill('0');
									abs_pos_z_next << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
											(POW_INT(2,i)*(slice_end[i]+1)) * volume->getVXL_D());
									img_path << abs_pos_z_next.str();
									slice_ind = (int)(n_slices_pred - (slice_end[i]+1)) + buffer_z;
								}
								else {
									img_path << abs_pos_z.str(); 
									slice_ind = (int)(n_slices_pred - slice_start[i]) + buffer_z;
								}

								if ( internal_rep == REAL_INTERNAL_REP )
									VirtualVolume::saveImage_to_Vaa3DRaw(
										slice_ind,
										img_path.str(), 
										rbuffer + buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i)), // adds the stride
										(int)height/(POW_INT(2,i)),(int)width/(POW_INT(2,i)),
										start_height,end_height,start_width,end_width, 
										saved_img_format, saved_img_depth
									);
								else // internal_rep == UINT8_INTERNAL_REP
									VirtualVolume::saveImage_from_UINT8_to_Vaa3DRaw(
										slice_ind,
										img_path.str(), 
										ubuffer + c,
										1,
										buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i))*bytes_chan,  // stride to be added for slice buffer_z
										(int)height/(POW_INT(2,i)),(int)width/(POW_INT(2,i)),
										start_height,end_height,start_width,end_width, 
										saved_img_format, saved_img_depth);
							}
							start_width  += stacks_width [i][stack_row][stack_column][0]; // WARNING TO BE CHECKED FOR CORRECTNESS
						}
						start_height += stacks_height[i][stack_row][0][0]; // WARNING TO BE CHECKED FOR CORRECTNESS
					}
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
	ref_sys reference(axis(1),axis(2),axis(3));
	TiledVolume   *tprobe;
	StackedVolume *sprobe;
	sprobe = dynamic_cast<StackedVolume *>(volume);
	if ( sprobe ) {
		reference.first  = sprobe->getAXS_1();
		reference.second = sprobe->getAXS_2();
		reference.third  = sprobe->getAXS_3();
	}
	else {
		tprobe = dynamic_cast<TiledVolume *>(volume);
		if ( tprobe ) {
			reference.first  = tprobe->getAXS_1();
			reference.second = tprobe->getAXS_2();
			reference.third  = tprobe->getAXS_3();
		}
	}
	for ( int c=0; c<channels; c++ ) {
		for(int res_i=0; res_i< resolutions_size; res_i++)
		{
			if(resolutions[res_i])
			{
				resolution_dir = chans_dir[c] + file_path[res_i].str();

				//---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
				//one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
				//is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
				//system.
				TiledVolume temp_vol(resolution_dir.c_str(),reference,
							volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));

	//          StackedVolume temp_vol(file_path[res_i].str().c_str(),ref_sys(axis(1),axis(2),axis(3)), volume->getVXL_V()*(res_i+1),
	//                      volume->getVXL_H()*(res_i+1),volume->getVXL_D()*(res_i+1));
			}
		}
	}


	// ubuffer allocated anyway
	delete ubuffer;

	// deallocate memory
    for(int res_i=0; res_i< resolutions_size; res_i++)
	{
		for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
		{
			for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
			{
				delete[] stacks_height[res_i][stack_row][stack_col];
				delete[] stacks_width [res_i][stack_row][stack_col];
				delete[] stacks_depth [res_i][stack_row][stack_col];
			}
			delete[] stacks_height[res_i][stack_row];
			delete[] stacks_width [res_i][stack_row];
			delete[] stacks_depth [res_i][stack_row];
		}
		delete[] stacks_height[res_i];
		delete[] stacks_width[res_i]; 
		delete[] stacks_depth[res_i]; 
	}
	delete[] chans_dir;
}

# else // channels directories in resolutions directories

void VolumeConverter::generateTilesVaa3DRawMC ( std::string output_path, bool* resolutions, 
				int block_height, int block_width, int block_depth, int method, 
				bool show_progress_bar, const char* saved_img_format, 
				int saved_img_depth )	throw (MyException)
{
    printf("in VolumeConverter::generateTilesVaa3DRawMC(path = \"%s\", resolutions = ", output_path.c_str());
    for(int i=0; i< S_MAX_MULTIRES; i++)
        printf("%d", resolutions[i]);
    printf(", block_height = %d, block_width = %d, block_depth = %d, method = %d, show_progress_bar = %s, saved_img_format = %s, saved_img_depth = %d)\n",
           block_height, block_width, block_depth, method, show_progress_bar ? "true" : "false", saved_img_format, saved_img_depth);

	if ( saved_img_depth == 0 ) // default is to generate an image with the same depth of the source
		saved_img_depth = volume->getBYTESxCHAN() * 8;
		
	if ( saved_img_depth != (volume->getBYTESxCHAN() * 8) ) {
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRaw: mismatch between bits per channel of source (%d) and destination (%d)",
			volume->getBYTESxCHAN() * 8, saved_img_depth);
		throw MyException(err_msg);
	}

	//LOCAL VARIABLES
    sint64 height, width, depth;	//height, width and depth of the whole volume that covers all stacks
	REAL_T* rbuffer;			//buffer where temporary image data are stored (REAL_INTERNAL_REP)
	uint8** ubuffer;			//array of buffers where temporary image data of channels are stored (UINT8_INTERNAL_REP)
	int bytes_chan = volume->getBYTESxCHAN();
	//uint8*  ubuffer_ch2;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
	//uint8*  ubuffer_ch3;	    //buffer temporary image data of channel 1 are stored (UINT8_INTERNAL_REP)
	int org_channels = 0;       //store the number of channels read the first time (for checking purposes)
    //REAL_T* stripe_up=NULL;		//will contain up-stripe and down-stripe computed by calling 'getStripe' method (unused)
	sint64 z_ratio, z_max_res;
    int n_stacks_V[S_MAX_MULTIRES];        //arrays of number of tiles along V, H and D directions respectively at i-th resolution
    int n_stacks_H[S_MAX_MULTIRES];
	int n_stacks_D[S_MAX_MULTIRES];  
    int ***stacks_height[S_MAX_MULTIRES];   //array of matrices of tiles dimensions at i-th resolution
	int ***stacks_width[S_MAX_MULTIRES];	
	int ***stacks_depth[S_MAX_MULTIRES];
	std::string *chans_dir;
	std::string resolution_dir;
    std::stringstream file_path[S_MAX_MULTIRES];                            //array of root directory name at i-th resolution
	int resolutions_size = 0;

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

	if ( volume == 0 ) {
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRawMC: undefined source volume");
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

	// code for testing
	//uint8 *temp = volume->loadSubvolume_to_UINT8(
	//	10,height-10,10,width-10,10,depth-10,
	//	&channels);

	//activating resolutions
    block_height = (block_height == -1 ? (int)height : block_height);
    block_width  = (block_width  == -1 ? (int)width  : block_width);
    block_depth  = (block_depth  == -1 ? (int)depth  : block_depth);
    if(block_height < S_MIN_SLICE_DIM || block_width < S_MIN_SLICE_DIM || block_depth < S_MIN_SLICE_DIM)
    { 
        char err_msg[IM_STATIC_STRINGS_SIZE];
        sprintf(err_msg,"The minimum dimension for block height, width, and depth is %d", S_MIN_SLICE_DIM);
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

	//computing tiles dimensions at each resolution
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
					stacks_width[res_i][stack_row][stack_col][stack_sli] = 
						((int)(width/POW_INT(2,res_i)))  / n_stacks_H[res_i] + (stack_col < ((int)(width/POW_INT(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
					stacks_depth[res_i][stack_row][stack_col][stack_sli] = 
						((int)(depth/POW_INT(2,res_i)))  / n_stacks_D[res_i] + (stack_sli < ((int)(depth/POW_INT(2,res_i)))  % n_stacks_D[res_i] ? 1:0);
				}
			}
		}
	}

	// computing channel directory names
	chans_dir = new std::string[channels];
	int n_digits = 1;
	int _channels = channels / 10;	
	while ( _channels ) {
		n_digits++;
		_channels /= 10;
	}
	for ( int c=0; c<channels; c++ ) {
		std::stringstream dir_name;
		dir_name.width(n_digits);
		dir_name.fill('0');
		dir_name << c;
		chans_dir[c] = "/" + (IM_CHANNEL_PREFIX + dir_name.str());
	}

	// computing resolutions directory names
	for(int res_i=0; res_i< resolutions_size; res_i++) {
		//creating volume directory iff current resolution is selected and test mode is disabled
		if(resolutions[res_i] == true) {
			file_path[res_i] << output_path << "/RES("<<height/POW_INT(2,res_i) 
							 << "x" << width/POW_INT(2,res_i) 
							 << "x" << depth/POW_INT(2,res_i) << ")";
			//if(make_dir(file_path[res_i].str().c_str())!=0) {
            if(!check_and_make_dir(file_path[res_i].str().c_str())) { // HP 130914
				char err_msg[S_MAX_MULTIRES];
				sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
				throw MyException(err_msg);
			}

			for ( int c=0; c<channels; c++ ) {
				//creating directory that will contain image data at current resolution
				resolution_dir = file_path[res_i].str() + chans_dir[c];
				//if(make_dir(resolution_dir.c_str())!=0)
                if(!check_and_make_dir(resolution_dir.c_str())) // HP 130914
				{
					char err_msg[S_MAX_MULTIRES];
					sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create DIR = \"%s\"\n", chans_dir[c].c_str());
					throw MyException(err_msg);
				}
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

	//slice_start and slice_end of current block depend on the resolution
	for(int res_i=0; res_i< resolutions_size; res_i++) {
		stack_block[res_i] = 0;
		slice_start[res_i] = this->D0; 
		slice_end[res_i] = slice_start[res_i] + stacks_depth[res_i][0][0][0] - 1;
	}

	for(sint64 z = this->D0, z_parts = 1; z < this->D1; z += z_max_res, z_parts++)
	{
		// fill one slice block
		if ( internal_rep == REAL_INTERNAL_REP )
			rbuffer = volume->loadSubvolume_to_REAL_T(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1);
		else { // internal_rep == UINT8_INTERNAL_REP
			ubuffer[0] = volume->loadSubvolume_to_UINT8(V0,V1,H0,H1,(int)(z-D0),(z-D0+z_max_res <= D1) ? (int)(z-D0+z_max_res) : D1,&channels,IM_NATIVE_RTYPE);
			if ( org_channels != channels ) {
				char err_msg[IM_STATIC_STRINGS_SIZE];
				sprintf(err_msg,"The volume contains images with a different number of channels (%d,%d)", org_channels, channels);
				throw MyException(err_msg);
			}
		
			for (int i=1; i<channels; i++ ) { // WARNING: assume 1-byte pixels
				// offsets are to be computed taking into account that buffer size along D may be different
				// WARNING: the offset must be of tipe sint64 
				ubuffer[i] = ubuffer[i-1] + (height * width * ((z_parts<=z_ratio) ? z_max_res : (depth%z_max_res)) * bytes_chan);
			}
		}
		// WARNING: should check that buffer has been actually allocated

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
			abs_pos_z << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
								(POW_INT(2,i)*slice_start[i]) * volume->getVXL_D());

			//compute the number of slice of previous groups at resolution i
			//note that z_parts in the number and not an index (starts from 1)
			n_slices_pred  = (z_parts - 1) * z_max_res / POW_INT(2,i);

			//buffer size along D is different when the remainder of the subdivision by z_max_res is considered
			sint64 z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

			//halvesampling resolution if current resolution is not the deepest one
			if(i!=0)	
				if ( internal_rep == REAL_INTERNAL_REP )
					VirtualVolume::halveSample(rbuffer,(int)height/(POW_INT(2,i-1)),(int)width/(POW_INT(2,i-1)),(int)z_size/(POW_INT(2,i-1)),method);
				else // internal_rep == UINT8_INTERNAL_REP
					VirtualVolume::halveSample_UINT8(ubuffer,(int)height/(POW_INT(2,i-1)),(int)width/(POW_INT(2,i-1)),(int)z_size/(POW_INT(2,i-1)),channels,method,bytes_chan);

			//saving at current resolution if it has been selected and iff buffer is at least 1 voxel (Z) deep
			if(resolutions[i] && (z_size/(POW_INT(2,i))) > 0)
			{
				if(show_progress_bar)
				{
					sprintf(progressBarMsg, "Saving to disc resolution %d",i+1);
                                        ProgressBar::getInstance()->updateInfo(progressBarMsg);
                                        ProgressBar::getInstance()->show();
				}

				for ( int c=0; c<channels; c++ ) {

					//storing in 'base_path' the absolute path of the directory that will contain all stacks
					std::stringstream base_path;
					base_path << file_path[i].str().c_str() << chans_dir[c].c_str() << "/";

					//looping on new stacks
					for(int stack_row = 0, start_height = 0, end_height = 0; stack_row < n_stacks_V[i]; stack_row++)
					{
						//incrementing end_height
						end_height = start_height + stacks_height[i][stack_row][0][0]-1; 
							
						//computing V_DIR_path and creating the directory the first time it is needed
						std::stringstream V_DIR_path;
						V_DIR_path << base_path.str() << this->getMultiresABS_V_string(i,start_height);
                        if(z==D0 && !check_and_make_dir(V_DIR_path.str().c_str()))
						{
							char err_msg[S_STATIC_STRINGS_SIZE];
							sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create V_DIR = \"%s\"\n", V_DIR_path.str().c_str());
							throw MyException(err_msg);
						}

						for(int stack_column = 0, start_width=0, end_width=0; stack_column < n_stacks_H[i]; stack_column++)
						{
							end_width  = start_width  + stacks_width [i][stack_row][stack_column][0]-1; 
								
							//computing H_DIR_path and creating the directory the first time it is needed
							std::stringstream H_DIR_path;
							H_DIR_path << V_DIR_path.str() << "/" << this->getMultiresABS_V_string(i,start_height) << "_" << this->getMultiresABS_H_string(i,start_width);
							if ( z==D0 ) {
                                if (!check_and_make_dir(H_DIR_path.str().c_str()))
								{
									char err_msg[S_STATIC_STRINGS_SIZE];
									sprintf(err_msg, "in generateTilesVaa3DRawMC(...): unable to create H_DIR = \"%s\"\n", H_DIR_path.str().c_str());
									throw MyException(err_msg);
								}
								else { // the directory has been created for the first time
									   // initialize block files
									V3DLONG *sz = new V3DLONG[4];
									int datatype;
									char *err_rawfmt;

									sz[0] = stacks_width[i][stack_row][stack_column][0];
									sz[1] = stacks_height[i][stack_row][stack_column][0];
									sz[3] = 1; // single channel files

									if ( internal_rep == REAL_INTERNAL_REP )
										datatype = 4;
									else if ( internal_rep == UINT8_INTERNAL_REP ) {
										if ( saved_img_depth == 16 )
											datatype = 2;
										else if ( saved_img_depth == 8 ) 
											datatype = 1;
										else {
											char err_msg[S_STATIC_STRINGS_SIZE];
											sprintf(err_msg, "in generateTilesVaa3DRaw(...): unknown image depth (%d)", saved_img_depth);
											throw MyException(err_msg);
										}
									}
									else {
										char err_msg[S_STATIC_STRINGS_SIZE];
										sprintf(err_msg, "in generateTilesVaa3DRaw(...): unknown internal representation (%d)", internal_rep);
										throw MyException(err_msg);
									}

									int slice_start_temp = 0;
									for ( int j=0; j < n_stacks_D[i]; j++ ) {
										sz[2] = stacks_depth[i][stack_row][stack_column][j];

										std::stringstream abs_pos_z_temp;
										abs_pos_z_temp.width(6);
										abs_pos_z_temp.fill('0');
										abs_pos_z_temp << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
											(POW_INT(2,i)*(slice_start_temp)) * volume->getVXL_D());

										std::stringstream img_path_temp;
										img_path_temp << H_DIR_path.str() << "/" 
													  << this->getMultiresABS_V_string(i,start_height) << "_" 
													  << this->getMultiresABS_H_string(i,start_width) << "_"
													  << abs_pos_z_temp.str();

										if ( (err_rawfmt = initRawFile((char *)img_path_temp.str().c_str(),sz,datatype)) != 0 ) {
											char err_msg[IM_STATIC_STRINGS_SIZE];
											sprintf(err_msg,"VolumeConverter::generateTilesVaa3DRawMC: error in initializing block file - %s", err_rawfmt);
											throw MyException(err_msg);
										};

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
								img_path << H_DIR_path.str() << "/" 
											<< this->getMultiresABS_V_string(i,start_height) << "_" 
											<< this->getMultiresABS_H_string(i,start_width) << "_";
								if ( (z/POW_INT(2,i)+buffer_z) > slice_end[i] ) { // start a new block along z
									abs_pos_z_next.width(6);
									abs_pos_z_next.fill('0');
									abs_pos_z_next << (int)(this->getMultiresABS_D(i) + // all stacks start at the same D position
											(POW_INT(2,i)*(slice_end[i]+1)) * volume->getVXL_D());
									img_path << abs_pos_z_next.str();
									slice_ind = (int)(n_slices_pred - (slice_end[i]+1)) + buffer_z;
								}
								else {
									img_path << abs_pos_z.str(); 
									slice_ind = (int)(n_slices_pred - slice_start[i]) + buffer_z;
								}

								if ( internal_rep == REAL_INTERNAL_REP )
									VirtualVolume::saveImage_to_Vaa3DRaw(
										slice_ind,
										img_path.str(), 
										rbuffer + buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i)), // adds the stride
										(int)height/(POW_INT(2,i)),(int)width/(POW_INT(2,i)),
										start_height,end_height,start_width,end_width, 
										saved_img_format, saved_img_depth
									);
								else // internal_rep == UINT8_INTERNAL_REP
									VirtualVolume::saveImage_from_UINT8_to_Vaa3DRaw(
										slice_ind,
										img_path.str(), 
										ubuffer + c,
										1,
										buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i))*bytes_chan,  // stride to be added for slice buffer_z
										(int)height/(POW_INT(2,i)),(int)width/(POW_INT(2,i)),
										start_height,end_height,start_width,end_width, 
										saved_img_format, saved_img_depth);
							}
							start_width  += stacks_width [i][stack_row][stack_column][0]; // WARNING TO BE CHECKED FOR CORRECTNESS
						}
						start_height += stacks_height[i][stack_row][0][0]; // WARNING TO BE CHECKED FOR CORRECTNESS
					}
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
	ref_sys reference(axis(1),axis(2),axis(3));
	TiledVolume   *tprobe;
	StackedVolume *sprobe;
	sprobe = dynamic_cast<StackedVolume *>(volume);
	if ( sprobe ) {
		reference.first  = sprobe->getAXS_1();
		reference.second = sprobe->getAXS_2();
		reference.third  = sprobe->getAXS_3();
	}
	else {
		tprobe = dynamic_cast<TiledVolume *>(volume);
		if ( tprobe ) {
			reference.first  = tprobe->getAXS_1();
			reference.second = tprobe->getAXS_2();
			reference.third  = tprobe->getAXS_3();
		}
	}
	for(int res_i=0; res_i< resolutions_size; res_i++) {

		if(resolutions[res_i]) {

			for ( int c=0; c<channels; c++ ) {
				resolution_dir = file_path[res_i].str() + chans_dir[c];

				//---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
				//one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
				//is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
				//system.
				TiledVolume temp_vol(resolution_dir.c_str(),reference,
							volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));

	//          StackedVolume temp_vol(file_path[res_i].str().c_str(),ref_sys(axis(1),axis(2),axis(3)), volume->getVXL_V()*(res_i+1),
	//                      volume->getVXL_H()*(res_i+1),volume->getVXL_D()*(res_i+1));
			}

			TiledMCVolume temp_mc_vol(file_path[res_i].str().c_str(),reference,
					volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));

		}
	}


	// ubuffer allocated anyway
	delete ubuffer;

	// deallocate memory
    for(int res_i=0; res_i< resolutions_size; res_i++)
	{
		for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
		{
			for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
			{
				delete[] stacks_height[res_i][stack_row][stack_col];
				delete[] stacks_width [res_i][stack_row][stack_col];
				delete[] stacks_depth [res_i][stack_row][stack_col];
			}
			delete[] stacks_height[res_i][stack_row];
			delete[] stacks_width [res_i][stack_row];
			delete[] stacks_depth [res_i][stack_row];
		}
		delete[] stacks_height[res_i];
		delete[] stacks_width[res_i]; 
		delete[] stacks_depth[res_i]; 
	}

	delete[] chans_dir;
}

# endif

