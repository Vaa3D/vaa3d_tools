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
* 2016-06-17. Giulio.    @ADDED ability to read a downsampled image 
* 2016-03-24. Giulio.    @ADDED 16 bit support.
* 2015-04-15 Alessandro. @ADDED definition for default constructor.
* 2014-11-22 Giulio.     @CHANGED code using OpenCV has been commente. It can be found searching comments containing 'Giulio_CV'
*/

#include "SimpleVolumeRaw.h"
#include "StackRaw.h"
#include "RawFmtMngr.h"

#ifdef _WIN32
#include "dirent_win.h"
#else
#include <dirent.h>
#endif
#include <list>
// Giulio_CV #include <cxcore.h>
// Giulio_CV #include <cv.h>
// Giulio_CV #include <highgui.h>

using namespace std;
using namespace iim;

// 2015-04-15. Alessandro. @ADDED definition for default constructor.
SimpleVolumeRaw::SimpleVolumeRaw(void) : VirtualVolume()
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);
    
    N_ROWS = N_COLS = 0;
    STACKS = 0;
}

SimpleVolumeRaw::SimpleVolumeRaw(const char* _root_dir)  throw (IOException)
: VirtualVolume(_root_dir, 1.0f, 1.0f, 1.0f)
{
    /**/iim::debug(iim::LEV3, strprintf("_root_dir = \"%s\"", _root_dir).c_str(), __iim__current__function__);

	init();
	initChannels();

	downsamplingFactor = 1;
	trueVXL_V = VXL_V;
	trueVXL_H = VXL_H;
	trueVXL_D = VXL_D;
	trueDIM_V = DIM_V;
	trueDIM_H = DIM_H;
	trueDIM_D = DIM_D;
}


SimpleVolumeRaw::~SimpleVolumeRaw(void) throw (iim::IOException)
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

	// iannello if(root_dir)
	// iannello 	delete[] root_dir;

	if(STACKS)
	{
		for(int row=0; row<N_ROWS; row++)
		{
			for(int col=0; col<N_COLS; col++)
				delete STACKS[row][col];
			delete[] STACKS[row];
		}
		delete[] STACKS;
	}
}


void SimpleVolumeRaw::init() throw (IOException)
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

	/************************* 1) LOADING STRUCTURE *************************    
	*************************************************************************/

	//LOCAL VARIABLES
	DIR *cur_dir_lev1;				//pointer to DIR, the data structure that represents a DIRECTORY (level 1 of hierarchical structure)
	int i=0,j=0;					//for counting of N_ROWS, N_COLS
    list<StackRaw*> stacks_list;                       //each stack found in the hierarchy is pushed into this list
	char stack_i_j_path[STATIC_STRINGS_SIZE];

	//obtaining DIR pointer to root_dir (=NULL if directory doesn't exist)
	if (!(cur_dir_lev1=opendir(root_dir)))
	{
		char msg[STATIC_STRINGS_SIZE];
		sprintf(msg,"in SimpleVolumeRaw::init(...): Unable to open directory \"%s\"", root_dir);
        throw IOException(msg);
	}
    closedir(cur_dir_lev1);

	// Simple format has only one stack
	N_ROWS = 1;
	N_COLS = 1;

	//allocating the only stack
	sprintf(stack_i_j_path,"%s",""); 
	StackRaw *new_stk = new StackRaw(this,i,j,stack_i_j_path);
	stacks_list.push_back(new_stk);

	//converting stacks_list (STL list of StackRaw*) into STACKS (2-D array of StackRaw*)
	STACKS = new StackRaw**[N_ROWS];
	for(int row=0; row < N_ROWS; row++)
		STACKS[row] = new StackRaw*[N_COLS];
	for(list<StackRaw*>::iterator i = stacks_list.begin(); i != stacks_list.end(); i++)
		STACKS[(*i)->getROW_INDEX()][(*i)->getCOL_INDEX()] = (*i);

	/******************** 3) COMPUTING VOLUME DIMENSIONS ********************  
	*************************************************************************/
	for(int row=0; row < N_ROWS; row++)
		for(int col=0; col < N_COLS; col++)
		{
			if(row==0)
				DIM_H+=STACKS[row][col]->getWIDTH();
			if(col==0)
				DIM_V+=STACKS[row][col]->getHEIGHT();
			DIM_D = STACKS[row][col]->getDEPTH() > DIM_D ? STACKS[row][col]->getDEPTH() : DIM_D;
		}

	/**************** 4) COMPUTING STACKS ABSOLUTE POSITIONS ****************  
	*************************************************************************/
	for(int row=0; row < N_ROWS; row++)
		for(int col=0; col < N_COLS; col++)
		{
			if(row)
				STACKS[row][col]->setABS_V(STACKS[row-1][col]->getABS_V()+STACKS[row-1][col]->getHEIGHT());
			else
				STACKS[row][col]->setABS_V(0);

			if(col)
				STACKS[row][col]->setABS_H(STACKS[row][col-1]->getABS_H()+STACKS[row][col-1]->getWIDTH());
			else
				STACKS[row][col]->setABS_H(0);
		}
}

void SimpleVolumeRaw::initChannels ( ) throw (IOException)
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

	char slice_fullpath[STATIC_STRINGS_SIZE];

	sprintf(slice_fullpath, "%s/%s/%s", root_dir, STACKS[0][0]->getDIR_NAME(), STACKS[0][0]->getFILENAMES()[0]);

	// get file attributes
	char *err_rawfmt;
	void *fhandle;
	V3DLONG *sz = 0;
	int datatype;
	int b_swap;
	int header_len;
	
	if ( (err_rawfmt = loadRaw2Metadata(slice_fullpath,sz,datatype,b_swap,fhandle,header_len)) != 0 ) {
		if ( sz ) delete[] sz;
		char msg[STATIC_STRINGS_SIZE];
		sprintf(msg,"in SimpleVolumeRaw::initChannels: unable to open image \"%s\". Wrong path or format (%s)", 
			slice_fullpath,err_rawfmt);
        throw IOException(msg);
	}
	closeRawFile((FILE *)fhandle);

	DIM_C = (int)sz[3];
	BYTESxCHAN = datatype;

    n_active = DIM_C;
    active = new uint32[n_active];
    for ( int c=0; c<DIM_C; c++ )
        active[c] = c; // all channels are assumed active
	
	delete[] sz;
}

void SimpleVolumeRaw::setDOWNSAMPLINGFACTOR( int f ) {
	downsamplingFactor = f;
	VXL_V = trueVXL_V * f;
	VXL_H = trueVXL_H * f;
	VXL_D = trueVXL_D * f;
	DIM_V = (int)ceil((double)trueDIM_V / f);
	DIM_H = (int)ceil((double)trueDIM_H / f);
	DIM_D = (int)ceil((double)trueDIM_D / f);
}


void SimpleVolumeRaw::resetDOWNSAMPLINGFACTOR() {
	downsamplingFactor = 1;
	VXL_V = trueVXL_V;
	VXL_H = trueVXL_H;
	VXL_D = trueVXL_D;
	DIM_V = trueDIM_V;
	DIM_H = trueDIM_H;
	DIM_D = trueDIM_D;
}


real32 *SimpleVolumeRaw::loadSubvolume_to_real32(int V0,int V1, int H0, int H1, int D0, int D1)  throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d", V0, V1, H0, H1, D0, D1).c_str(), __iim__current__function__);

	throw IOException("in SimpleVolumeRaw::loadSubvolume_to_real32(...): disabled to remove dependence from openCV"); // Giulio_CV

	char msg[STATIC_STRINGS_SIZE];
    sprintf(msg,"in SimpleVolumeRaw::loadSubvolume_to_real32: not implemented yet");
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

	/* Giulio_CV

	for(int row=0; row<N_ROWS; row++)
		for(int col=0; col<N_COLS; col++)
		{
			Rect_t *intersect_area = STACKS[row][col]->Intersects(subvol_area);
			if(intersect_area)
			{
				STACKS[row][col]->loadStack(D0, D1-1);

				for(int k=0; k<sbv_depth; k++)
				{
					CvMat *slice = STACKS[row][col]->getSTACKED_IMAGE()[D0+k];
					int   step  = slice->step/sizeof(float);
					float *data = slice->data.fl;
					int ABS_V_stk = STACKS[row][col]->getABS_V();
					int ABS_H_stk = STACKS[row][col]->getABS_H();

					for(int i=intersect_area->V0-V0; i<intersect_area->V1-V0; i++)
						for(int j=intersect_area->H0-H0; j<intersect_area->H1-H0; j++)
							subvol[k*sbv_height*sbv_width + i*sbv_width + j] = (data+(i-ABS_V_stk+V0)*step)[j-ABS_H_stk+H0];
				}

				// allocated space has to always released
				STACKS[row][col]->releaseStack();
			}
		}

	*/

	return subvol;
}


uint8 *SimpleVolumeRaw::loadSubvolume_to_UINT8(int V0,int V1, int H0, int H1, int D0, int D1, int *channels, int ret_type) throw (IOException, iom::exception)
{
    /**/iim::debug(iim::LEV3, strprintf("V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d, *channels=%d, ret_type=%d", V0, V1, H0, H1, D0, D1, channels ? *channels : -1, ret_type).c_str(), __iim__current__function__);

    //checking for non implemented features
	if( this->BYTESxCHAN > 2 ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"in SimpleVolumeRaw::loadSubvolume_to_UINT8: invalid number of bytes per channel (%d)",this->BYTESxCHAN); 
        throw IOException(err_msg);
	}

	// if ( (ret_type == iim::DEF_IMG_DEPTH) && ((8 * this->BYTESxCHAN) != iim::DEF_IMG_DEPTH)  ) {
	//	// return type is 8 bits, but native depth is not 8 bits
	//	char err_msg[STATIC_STRINGS_SIZE];
	//	sprintf(err_msg,"SimpleVolumeRaw::loadSubvolume_to_UINT8: non supported return type (%d bits) - native type is %d bits",ret_type, 8*this->BYTESxCHAN); 
	//       throw IOException(err_msg);
	//}

    if ( (ret_type != iim::NATIVE_RTYPE) && (ret_type != iim::DEF_IMG_DEPTH) ) {
		// return type should be converted, but not to 8 bits per channel
        char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"in SimpleVolumeRaw::loadSubvolume_to_UINT8: non supported return type (%d bits) - native type is %d bits",ret_type, 8*this->BYTESxCHAN); 
        throw IOException(err_msg);
	}

	// reduction factor to be applied to the loaded buffer
    int red_factor = (ret_type == iim::NATIVE_RTYPE) ? 1 : ((8 * this->BYTESxCHAN) / ret_type);

	char *err_rawfmt;

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

	uint8 *subvol;

	bool whole_slices = V0 == 0 && V1 == DIM_V && H0 == 0 && H1 == DIM_H;

	//if ( whole_slices )
	if ( true )
		subvol = new uint8[sbv_height * sbv_width * sbv_depth * DIM_C * BYTESxCHAN];
	else 
		subvol = 0;

    //initializing the number of channels with an undefined value (it will be detected from the first slice read)
    sint64 sbv_channels = -1;
	sint64 sbv_bytes_chan = -1;

	//scanning of stacks matrix for data loading and storing into subvol
	Rect_t subvol_area;
	subvol_area.H0 = H0;
	subvol_area.V0 = V0;
	subvol_area.H1 = H1;
	subvol_area.V1 = V1;
	//bool first_time = true;

	for(int row=0; row<N_ROWS; row++) 
	{
		for(int col=0; col<N_COLS; col++)
		{
			Rect_t *intersect_area = STACKS[row][col]->Intersects(subvol_area);
			if(intersect_area)
			{
				//LOCAL VARIABLES
				char slice_fullpath[STATIC_STRINGS_SIZE];
				//IplImage *slice;
				unsigned char *slice = 0;

				for(int k=0; k<sbv_depth; k++)
				{
					//building image path
					int trueSliceIndex = downsamplingFactor * (D0+k);
					sprintf(slice_fullpath, "%s/%s/%s", root_dir, 
							STACKS[row][col]->getDIR_NAME(), 
							STACKS[row][col]->getFILENAMES()[trueSliceIndex]);
							//STACKS[row][col]->getFILENAMES()[D0+k]);

					//loading image
					char *err_rawfmt;
					V3DLONG *sz = 0;
					int datatype;
					int b_swap;
					void *fhandle;
					int header_len;
					
					if ( (err_rawfmt = loadRaw2Metadata(slice_fullpath,sz,datatype,b_swap,fhandle,header_len )) != 0 ) {
						if ( sz ) delete[] sz;
						char msg[STATIC_STRINGS_SIZE];
						sprintf(msg,"in SimpleVolumeRaw::loadSubvolume_to_UINT8: unable to open image \"%s\". Wrong path or format (%s)", 
							slice_fullpath,err_rawfmt);
                        throw IOException(msg);
					}

					// resize cols and rows
					int cols = (int)ceil((double)sz[0]/downsamplingFactor);
					int rows = (int)ceil((double)sz[1]/downsamplingFactor);

					/*
					// if whole_slices is true, then the offset on subvol must be of k slices relative to one chanel and the channel stride is the number of slices, 
					// otherwise allocate a buffer to contain all the channels and the channel stride is 1
					*/
					// if whole_slices is true use image dimensions to compute the offset, otherwise use subvolume dimensions
					unsigned char *slice = whole_slices ? (subvol + k * (rows * cols * datatype)) : (subvol + k * (sbv_height * sbv_width * datatype))/*new uint8[rows * cols * sz[3] * datatype]*/;
					if ( whole_slices ? 
							(err_rawfmt = loadRawSlices2SubStack(fhandle,slice,(whole_slices ? (int)sbv_depth : 1),sz,datatype,0,0,b_swap,header_len,downsamplingFactor)) != 0 :
							(err_rawfmt = loadRaw2SubStack(fhandle,slice,sz,H0,V0,0,H1,V1,1,datatype,b_swap,header_len,(int)sbv_depth)) != 0 ) {
						if ( sz ) delete[] sz;
						char msg[STATIC_STRINGS_SIZE];
						sprintf(msg,"in SimpleVolumeRaw::loadSubvolume_to_UINT8: unable to read image \"%s\". Wrong path or format (%s)", 
							slice_fullpath,err_rawfmt);
                        throw IOException(msg);
					}

                    closeRawFile(fhandle);
					
					//slice = cvLoadImage(slice_fullpath, CV_LOAD_IMAGE_ANYCOLOR);  //without CV_LOAD_IMAGE_ANYDEPTH, image is converted to 8-bits if needed
					// old version: slice_img_i = cvLoadImage(slice_fullpath, CV_LOAD_IMAGE_GRAYSCALE | CV_LOAD_IMAGE_ANYDEPTH);

					if(sz[2] != 1)
					{
						if ( sz ) delete[] sz;
						char msg[STATIC_STRINGS_SIZE];
						sprintf(msg,"in SimpleVolumeRaw::loadSubvolume_to_UINT8: raw image contains more than one slice (%s)", 
							slice_fullpath);
                        throw IOException(msg);
					}		

					//if ( whole_slices ) {
					if ( true ) {
                        sbv_channels = sz[3];
						delete []sz; // release memory because when e new 2D file is opened at the next cycle sz is reallocated
						continue;
					}

					if(!slice)
					{
						if ( sz ) delete[] sz;
						char msg[STATIC_STRINGS_SIZE];
						sprintf(msg,"in SimpleVolumeRaw::loadSubvolume_to_UINT8: unable to read image \"%s\". Wrong path or format", 
							slice_fullpath);
                        throw IOException(msg);
					}		

					//if this is the first time a slice is loaded, detecting the number of channels and safely allocating memory for data
                    if(!subvol)
                    {
                        //first_time = false;
                        sbv_channels = sz[3];
                        //if(sbv_channels != 1 && sbv_channels != 3)
                        //	throw iim::IOException(std::string("Unsupported number of channels at \"").append(slice_fullpath).append("\". Only 1 and 3-channels images are supported").c_str());

                        try
                        {
                            subvol = new uint8[sbv_height * sbv_width * sbv_depth * sbv_channels * datatype];
                        }
                        catch(...){
							if ( sz ) delete[] sz;
                            throw IOException("in SimpleVolumeRaw::loadSubvolume_to_UINT8: unable to allocate memory");
						}
                    }
                    //otherwise checking that all the other slices have the same bitdepth of the first one
					else if(sz[3] != sbv_channels) {
						if ( sz ) delete[] sz;
                        throw IOException(std::string("in SimpleVolumeRaw::loadSubvolume_to_UINT8: Image depth mismatch at slice at \"").append(slice_fullpath).append("\": all slices must have the same bitdepth").c_str());
					}

					//computing offsets
                    int slice_step = (int)sz[0]; // WARNING, not sure: to be checked
                    sint64 ABS_V_offset = V0 - STACKS[row][col]->getABS_V();
                    sint64 ABS_H_offset = (H0 - STACKS[row][col]->getABS_H());

                    sint64 istart, iend, jstart, jend;
                    istart  = intersect_area->V0-V0;
                    iend    = intersect_area->V1-V0;
                    jstart  = intersect_area->H0-H0;
                    jend    = intersect_area->H1-H0;
 					for ( int c=0; c<sbv_channels; c++ )
					{
                        sint64 c_offset = c*sbv_height*sbv_width*sbv_depth;
                        sint64 k_offset = k*sbv_height*sbv_width;
                        for(sint64 i = istart; i < iend; i++)
                        {
                            uint8* slice_row = ((uint8*)slice) + ((c*sbv_height*sbv_width + (i+ABS_V_offset)*slice_step) * datatype);
							int cnt = 0;
                            for(sint64 j = jstart * datatype; cnt < ((jend-jstart) * datatype); j++, cnt++)
                                subvol[((c_offset + k_offset + i*sbv_width) * datatype) + j] = slice_row[(ABS_H_offset * datatype) + j];
                        }
                    }

					//releasing image
					//cvReleaseImage(&slice);
					if ( !whole_slices && slice ) // release only if olny prtial slices have been read (and then copied)
						delete slice;
					if ( sz )
						delete[] sz;
					slice = 0;
					sz = 0;
				}
			}
		}
	}

    if(channels)
        *channels = (int)sbv_channels;

	if ( red_factor > 1 ) { // the buffer has to be reduced

		if ( (err_rawfmt = convert2depth8bits(red_factor,(sbv_height*sbv_width*sbv_depth),sbv_channels,subvol)) != 0  ) {
            char err_msg[STATIC_STRINGS_SIZE];
			sprintf(err_msg,"in SimpleVolumeRaw::loadSubvolume_to_UINT8: %s", err_rawfmt);
            throw IOException(err_msg);
		}
	}

	return subvol;
}
