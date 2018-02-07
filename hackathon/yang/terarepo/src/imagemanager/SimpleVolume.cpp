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
* 2016-06-16. Giulio.     @ADDED ability to read a downsampled image 
* 2016-03-24. Giulio.     @ADDED 16 bit support.
* 2015-04-15. Alessandro. @ADDED definition for default constructor.
* 2015-02-27. Alessandro. @ADDED automated selection of IO plugin if not provided.
* 2014-11-27 Giulio.      @FIXED   eliminated part of the dipendences from OpenCV and restored the corresponding code
* 2014-11-22 Giulio.      @CHANGED code using OpenCV has been commente. It can be found searching comments containing 'Giulio_CV'
*/

#include <iostream>
#include "SimpleVolume.h"
#include "Stack.h"
#ifdef _WIN32
#include "dirent_win.h"
#else
#include <dirent.h>
#endif
#include <list>
// Giulio_CV #include <cxcore.h>
// Giulio_CV #include <cv.h>
// Giulio_CV #include <highgui.h>
#include "Tiff3DMngr.h"
#include "RawFmtMngr.h"

#include "IOPluginAPI.h" // 2014-11-26. Giulio.

using namespace std;
using namespace iim;

// 2015-04-15. Alessandro. @ADDED definition for default constructor.
SimpleVolume::SimpleVolume(void) : VirtualVolume()
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);
    
    N_ROWS = N_COLS = 0;
    STACKS = 0;
}

SimpleVolume::SimpleVolume(const char* _root_dir)  throw (IOException)
: VirtualVolume(_root_dir, 1.0f, 1.0f, 1.0f)
{
    /**/iim::debug(iim::LEV3, strprintf("_root_dir = \"%s\"", root_dir).c_str(), __iim__current__function__);

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


SimpleVolume::~SimpleVolume(void) throw (iim::IOException)
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

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


void SimpleVolume::init() throw (iim::IOException)
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

	// 2015-02-27. Alessandro. @ADDED automated selection of IO plugin if not provided.
	// Currently, this class does not use the IO plugins, but directly calls functions in Tiff3DMngr.cpp.
	// This should be changed if a different policy will be used in the future.
	if(iom::IMIN_PLUGIN.compare("empty") == 0)
		iom::IMIN_PLUGIN = "tiff2D";

	/************************* 1) LOADING STRUCTURE *************************    
	*************************************************************************/

	//LOCAL VARIABLES
	DIR *cur_dir_lev1;				//pointer to DIR, the data structure that represents a DIRECTORY (level 1 of hierarchical structure)
	int i=0,j=0;					//for counting of N_ROWS, N_COLS
    list<iim::Stack*> stacks_list;                       //each stack found in the hierarchy is pushed into this list
	char stack_i_j_path[STATIC_STRINGS_SIZE];

	//obtaining DIR pointer to root_dir (=NULL if directory doesn't exist)
	if (!(cur_dir_lev1=opendir(root_dir)))
	{
		char msg[STATIC_STRINGS_SIZE];
		sprintf(msg,"in SimpleVolume::init(...): Unable to open directory \"%s\"", root_dir);
        throw IOException(msg);
	}
    closedir(cur_dir_lev1);

	// Simple format has only one stack
	N_ROWS = 1;
	N_COLS = 1;

	//allocating the only stack
	sprintf(stack_i_j_path,"%s",""); 
	iim::Stack *new_stk = new iim::Stack(this,i,j,stack_i_j_path);
	stacks_list.push_back(new_stk);

	//converting stacks_list (STL list of Stack*) into STACKS (2-D array of Stack*)
	STACKS = new iim::Stack**[N_ROWS];
	for(int row=0; row < N_ROWS; row++)
		STACKS[row] = new iim::Stack*[N_COLS];
	for(list<iim::Stack*>::iterator i = stacks_list.begin(); i != stacks_list.end(); i++)
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

void SimpleVolume::initChannels ( ) throw (IOException)
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

	char slice_fullpath[STATIC_STRINGS_SIZE];
	int img_width;
	int img_height;
	std::string params;


	sprintf(slice_fullpath, "%s/%s/%s", root_dir, STACKS[0][0]->getDIR_NAME(), STACKS[0][0]->getFILENAMES()[0]);

    try
    {
        iomanager::IOPluginFactory::getPlugin2D("tiff2D")->readMetadata(slice_fullpath, img_width, img_height,	BYTESxCHAN, DIM_C, params);
    }
    catch (iom::exception & ex)
    {
        throw iim::IOException(ex.what());
    }

	/* Giulio_CV 
	
	IplImage* slice = cvLoadImage(slice_fullpath, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);  //without CV_LOAD_IMAGE_ANYDEPTH, image is converted to 8-bits if needed
	if(!slice)
        throw IOException(std::string("Unable to load slice at \"").append(slice_fullpath).append("\"").c_str());
    DIM_C = slice->nChannels;
	if ( slice->depth == IPL_DEPTH_8U )
		BYTESxCHAN = 1; 
	else if ( slice->depth == IPL_DEPTH_16U )
		BYTESxCHAN = 2; 
	else if ( slice->depth == IPL_DEPTH_32F )
		BYTESxCHAN = 4;
	else {
		char msg[STATIC_STRINGS_SIZE];
		sprintf(msg,"in SimpleVolume::initChannels: unknown color depth");
        throw IOException(msg);
	}

	*/

	n_active = DIM_C;
    active = new uint32[n_active];
    for ( int c=0; c<DIM_C; c++ )
        active[c] = c; // all channels are assumed active

	// Giulio_CV cvReleaseImage(&slice);
}

void SimpleVolume::setDOWNSAMPLINGFACTOR( int f ) {
	downsamplingFactor = f;
	VXL_V = trueVXL_V * f;
	VXL_H = trueVXL_H * f;
	VXL_D = trueVXL_D * f;
	DIM_V = (int)ceil((double)trueDIM_V / f);
	DIM_H = (int)ceil((double)trueDIM_H / f);
	DIM_D = (int)ceil((double)trueDIM_D / f);
}


void SimpleVolume::resetDOWNSAMPLINGFACTOR() {
	downsamplingFactor = 1;
	VXL_V = trueVXL_V;
	VXL_H = trueVXL_H;
	VXL_D = trueVXL_D;
	DIM_V = trueDIM_V;
	DIM_H = trueDIM_H;
	DIM_D = trueDIM_D;
}


real32 *SimpleVolume::loadSubvolume_to_real32(int V0,int V1, int H0, int H1, int D0, int D1)  throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("V0 = %d, V1 = %d, H0 = %d, H1 = %d, D0 = %d, D1 = %d", V0, V1, H0, H1, D0, D1).c_str(), __iim__current__function__);

	throw IOException("in SimpleVolume::loadSubvolume_to_real32(...): disabled to remove dependence from openCV"); // Giulio_CV

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


uint8 *SimpleVolume::loadSubvolume_to_UINT8(int V0,int V1, int H0, int H1, int D0, int D1, int *channels, int ret_type) throw (IOException, iom::exception) {

    /**/iim::debug(iim::LEV3, strprintf("V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d, *channels=%d, ret_type=%d", V0, V1, H0, H1, D0, D1, channels ? *channels : -1, ret_type).c_str(), __iim__current__function__);

    //cout<<"SimpleVolume::loadSubvolume_to_UINT8 "<<V0<<" "<<V1<<" "<<H0<<" "<<H1<<" "<<D0<<" "<<D1<<" "<<(channels ? *channels : -1)<<endl;

	//throw IOException("in SimpleVolume::loadSubvolume_to_UINT8(...): disabled to remove dependence from openCV"); // Giulio_CV

    //checking for non implemented features
	if( this->BYTESxCHAN > 2 ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"SimpleVolume::loadSubvolume_to_UINT8: invalid number of bytes per channel (%d)",this->BYTESxCHAN); 
        throw IOException(err_msg);
	}

	// if ( (ret_type == iim::DEF_IMG_DEPTH) && ((8 * this->BYTESxCHAN) != iim::DEF_IMG_DEPTH)  ) {
	//	// return type is 8 bits, but native depth is not 8 bits
	//	char err_msg[STATIC_STRINGS_SIZE];
	//	sprintf(err_msg,"SimpleVolume::loadSubvolume_to_UINT8: non supported return type (%d bits) - native type is %d bits",ret_type, 8*this->BYTESxCHAN); 
	//		throw IOException(err_msg);
	//}

    if ( (ret_type != iim::NATIVE_RTYPE) && (ret_type != iim::DEF_IMG_DEPTH) ) {
		// return type should be converted, but not to 8 bits per channel
        char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"SimpleVolume::loadSubvolume_to_UINT8: non supported return type (%d bits) - native type is %d bits",ret_type, 8*this->BYTESxCHAN); 
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

	uint8 *subvol = 0;

	bool whole_slices = V0 == 0 && V1 == DIM_V && H0 == 0 && H1 == DIM_H;

	if ( DIM_C == 1 ) 
		subvol = new uint8[sbv_height * sbv_width * sbv_depth * BYTESxCHAN];
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
	bool first_time = true;

	for(int row=0; row<N_ROWS; row++) // probably can be eliminated: N_ROWS is always 1
	{
		for(int col=0; col<N_COLS; col++) // probably can be eliminated: N_ROWS is always 1
		{
			Rect_t *intersect_area = STACKS[row][col]->Intersects(subvol_area);
			if(intersect_area) // probably can be eliminated: intersect_area is always true
			{
				//LOCAL VARIABLES
				char slice_fullpath[STATIC_STRINGS_SIZE];
				// Giulio_CV IplImage *slice;

				for(int k=0; k<sbv_depth; k++)
				{
					//building image path
					int trueSliceIndex = downsamplingFactor * (D0+k);
					sprintf(slice_fullpath, "%s/%s/%s", root_dir, 
							STACKS[row][col]->getDIR_NAME(), 
							STACKS[row][col]->getFILENAMES()[trueSliceIndex]);
					//loading image

					char *err_Tiff3Dfmt;

					unsigned int rows;
					unsigned int cols;
					unsigned int n_slices;
					unsigned int channels;
					int bytes_x_chan;
					int swap;
					void *dummy;
					int dummy_len;

					if ( (err_Tiff3Dfmt = loadTiff3D2Metadata((char *)slice_fullpath,cols,rows,n_slices,channels,bytes_x_chan,swap,dummy,dummy_len)) != 0 ) {
						throw iom::exception(iom::strprintf("unable to read tiff file (%s)",err_Tiff3Dfmt), __iom__current__function__);
					}
					closeTiff3DFile(dummy);

					// resize cols and rows
					cols = (int)ceil((double)cols/downsamplingFactor);
					rows = (int)ceil((double)rows/downsamplingFactor);

					if ( whole_slices && (rows != sbv_height || cols != sbv_width) ) {
						throw iom::exception(iom::strprintf("whole slices to be read: mismatch in slice dimensions ([rows,cols]=%dx%d, [sbv_height,sbv_width]=%dx%d, downsampling factor=%d)",
																												rows, cols, sbv_height, sbv_width, downsamplingFactor), __iom__current__function__);
					}

					uint8 *slice = ( DIM_C == 1 ) ? subvol + (k*sbv_height*sbv_width*bytes_x_chan) : new uint8[sbv_height * sbv_width * channels * bytes_x_chan]; // sbv variable should be used here

					// cols and rows should be passed here because sbv variables have a different value when a subregion is to be loaded
					if ( (err_Tiff3Dfmt = readTiff3DFile2Buffer((char *)slice_fullpath,slice,cols,rows,0,0,downsamplingFactor,V0,V1-1,H0,H1-1)) != 0 ) {
						throw iom::exception(iom::strprintf("unable to read tiff file (%s)",err_Tiff3Dfmt), __iom__current__function__);
					}

					/* Giulio_CV 

					slice = cvLoadImage(slice_fullpath, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);  //without CV_LOAD_IMAGE_ANYDEPTH, image is converted to 8-bits if needed
					// old version: slice_img_i = cvLoadImage(slice_fullpath, CV_LOAD_IMAGE_GRAYSCALE | CV_LOAD_IMAGE_ANYDEPTH);
					if(!slice)
					{
						char msg[STATIC_STRINGS_SIZE];
						sprintf(msg,"in SimpleVolume::loadSubvolume_to_UINT: unable to open image \"%s\". Wrong path or format.\nSupported formats are BMP, DIB, JPEG, JPG, JPE, JP2, PNG, PBM, PGM, PPM, SR, RAS, TIFF, TIF", slice_fullpath);
                        throw IOException(msg);
					}

					*/

					// WARNING: does not manage correclty multi-channel data

					//if ( whole_slices ) {
					//	continue;
					//}

					//if this is the first time a slice is loaded, detecting the number of channels and safely allocating memory for data
                    if(first_time)
                    {
                        first_time = false;
                        sbv_channels = channels; // Giulio_Cv slice->nChannels;
						sbv_bytes_chan = bytes_x_chan; // Giulio_Cv slice->depth / 8;
                        if(sbv_channels != 1 && sbv_channels != 3)
                            throw IOException(std::string("in SimpleVolume::loadSubvolume_to_UINT8: Unsupported number of channels at \"").append(slice_fullpath).append("\". Only 1 and 3-channels images are supported").c_str());
                        if(sbv_bytes_chan != this->BYTESxCHAN)
                            throw IOException(std::string("in SimpleVolume::loadSubvolume_to_UINT8: Wrong number of bits per channel\"").c_str());

						if ( !subvol ) {
							try
							{
								subvol = new uint8[sbv_height * sbv_width * sbv_depth * sbv_channels * sbv_bytes_chan];
							}
							catch(...){throw IOException("in SimpleVolume::loadSubvolume_to_UINT8: unable to allocate memory");}
						}
                    }
                    //otherwise checking that all the other slices have the same bitdepth of the first one
                    else
                    {
                        if (channels != sbv_channels)
                            throw iim::IOException(iim::strprintf("in SimpleVolume::loadSubvolume_to_UINT8: TIFF 2D series channel mismatch: slice at \"%s\" has %d channels, but the first slice has %d channels", slice_fullpath, sbv_channels, channels));
                        if (bytes_x_chan != sbv_bytes_chan)
                            throw iim::IOException(iim::strprintf("in SimpleVolume::loadSubvolume_to_UINT8: TIFF 2D series bytes per channel mismatch: slice at \"%s\" has %d, but the first slice has %d", slice_fullpath, bytes_x_chan, sbv_bytes_chan));
					}

					//computing offsets
                    sint64 slice_step = sbv_width * bytes_x_chan * channels; // Giulio_CV slice->widthStep / sizeof(uint8); // widthStep takes already into account the number of bytes per channel
                    int ABS_V_offset = V0 - STACKS[row][col]->getABS_V();
                    int ABS_H_offset = (H0 - STACKS[row][col]->getABS_H())*((int)sbv_channels);

					if ( whole_slices && (ABS_V_offset != 0 || ABS_H_offset != 0) ) {
						throw iom::exception(iom::strprintf("ABS_V_offset=%d, ABS_H_offset=%d when extracting whole slices",ABS_V_offset,ABS_H_offset), __iom__current__function__);
					}

					//different procedures for 1 and 3 channels images
                    int istart, iend, jstart, jend;
                    istart  = intersect_area->V0-V0;
                    iend    = intersect_area->V1-V0;
                    jstart  = intersect_area->H0-H0;
                    jend    = intersect_area->H1-H0;

					if ( istart !=0 || iend != sbv_height || jstart != 0 || jend != sbv_width ) {
						throw iom::exception(iom::strprintf("istart=%d, iend=%d, jstart=%d, jend=%d",istart,iend,jstart,jend), __iom__current__function__);
					}
					// istart = jstart = 0

                    if(sbv_channels == 1)
                    {
 						// about subvol index: actually there is always just one stack, 
						// hence all the subvolume has to be filled
						// this is why for k=0 is k_offset=0
						//sint64 k_offset = k*sbv_height*sbv_width*sbv_bytes_chan;
						//memcpy(subvol + k_offset, slice, sbv_height*sbv_width*sbv_bytes_chan);
       //                 for(int i = istart; i < iend; i++)
       //                 {
       //                     uint8* slice_row = slice + (i/*+ABS_V_offset*/)*slice_step;
							//int c = 0; // controls the number of bytes to be copied
							//for(sint64 j = (jstart * sbv_bytes_chan); c <((jend-jstart) * sbv_bytes_chan); j++, c++) { 
							//	// all horizontal indices have to be multiplied by sbv_bytes_chan, including jstart
							//	// the number of bytes to be copied is [(jend-jstart) * sbv_bytes_chan]
       //                         subvol[k_offset + i*sbv_width*sbv_bytes_chan + j] = slice_row[/*ABS_H_offset*sbv_bytes_chan*/ + j];
							//}
       //                 }
                    }
                    else if(sbv_channels == 3) // channels in subvol are separated, whereas in slice_row are arranged in triplets
                    {
						// about subvol index: actually there is always just one stack, 
						// hence all the subvolume has to be filled
						// this is why for k=0 offset1=0
                        sint64 offset1 = k*sbv_height*sbv_width*sbv_bytes_chan;
                        sint64 offset2 = offset1 + sbv_height*sbv_width*sbv_bytes_chan*sbv_depth;
                        sint64 offset3 = offset2 + sbv_height*sbv_width*sbv_bytes_chan*sbv_depth;
                        for(int i = istart; i < iend; i++)
                        {
                            uint8* slice_row = slice + (i/*+ABS_V_offset*/)*slice_step; // slice height is equal to subvol height: there is no vertical offset
 							sint64 c = 0; // controls the number of triplets to be copied
							// all horizontal indices have to be multiplied by sbv_bytes_chan, including jstart
 							// the number of triplets to be copied is (jend-jstart)
							for(sint64 j1 = (jstart * sbv_bytes_chan), j2 = (3 * jstart * sbv_bytes_chan); c <(jend-jstart); j1+=sbv_bytes_chan, j2+=3*sbv_bytes_chan, c++)
                            {
								// each triplet consists of sbv_bytes_chan bytes
								for ( int b=0; b<sbv_bytes_chan; b++ ) {
									// slice width is equal to subvol width: there is no horizontal offset
									subvol[offset1 + i*sbv_width*sbv_bytes_chan + j1 + b] = slice_row[/*ABS_H_offset +*/ j2 + b];
									subvol[offset2 + i*sbv_width*sbv_bytes_chan + j1 + b] = slice_row[/*ABS_H_offset +*/ sbv_bytes_chan + j2 + b];
									subvol[offset3 + i*sbv_width*sbv_bytes_chan + j1 + b] = slice_row[/*ABS_H_offset +*/ 2*sbv_bytes_chan + j2 + b];
								}
                            }
                        }
                    }
                    else
                        throw IOException(std::string("Unsupported number of channels at \"").append(slice_fullpath).append("\". Only 1 and 3-channels images are supported in this format").c_str());

					//releasing image
					//if ( !whole_slices && slice ) // release only if olny partial slices have been read (and then copied)
					if ( DIM_C != 1 && slice ) 
						delete []slice; // Giulio_CV cvReleaseImage(&slice);
				}
			}
		}
	}

    if(channels)
        *channels = (int)sbv_channels;

	if ( red_factor > 1 ) { // the buffer has to be reduced

		if ( (err_rawfmt = convert2depth8bits(red_factor,(sbv_height*sbv_width*sbv_depth),sbv_channels,subvol)) != 0  ) {
            char err_msg[STATIC_STRINGS_SIZE];
			sprintf(err_msg,"in SimpleVolume::loadSubvolume_to_UINT8: %s", err_rawfmt);
            throw IOException(err_msg);
		}
	}

	return subvol;
}
