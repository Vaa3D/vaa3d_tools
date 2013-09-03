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

#include "SimpleVolume.h"
#include "Stack.h"
#ifdef _WIN32
#include "dirent_win.h"
#else
#include <dirent.h>
#endif
#include <list>
#include <cxcore.h>
#include <cv.h>
#include <highgui.h>

using namespace std;


SimpleVolume::SimpleVolume(const char* _root_dir)  throw (MyException)
: VirtualVolume(_root_dir,(float)1.0,(float)1.0,(float)1.0) 
{
	init();
	initChannels();
}


SimpleVolume::~SimpleVolume(void)
{
	#if IM_VERBOSE > 3
	printf("\t\t\t\tin SimpleVolume::~SimpleVolume(void)\n");
	#endif

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


void SimpleVolume::init()
{
	#if IM_VERBOSE > 3
        printf("\t\t\t\tin SimpleVolume::init()\n");
	#endif

	/************************* 1) LOADING STRUCTURE *************************    
	*************************************************************************/

	//LOCAL VARIABLES
	DIR *cur_dir_lev1;				//pointer to DIR, the data structure that represents a DIRECTORY (level 1 of hierarchical structure)
	int i=0,j=0;					//for counting of N_ROWS, N_COLS
    list<Stack*> stacks_list;                       //each stack found in the hierarchy is pushed into this list
	char stack_i_j_path[IM_STATIC_STRINGS_SIZE];

	//obtaining DIR pointer to root_dir (=NULL if directory doesn't exist)
	if (!(cur_dir_lev1=opendir(root_dir)))
	{
		char msg[IM_STATIC_STRINGS_SIZE];
		sprintf(msg,"in SimpleVolume::init(...): Unable to open directory \"%s\"", root_dir);
		throw MyException(msg);
	}

	// Simple format has only one stack
	N_ROWS = 1;
	N_COLS = 1;

	//allocating the only stack
	sprintf(stack_i_j_path,"%s",""); 
	Stack *new_stk = new Stack(this,i,j,stack_i_j_path);
	stacks_list.push_back(new_stk);

	//converting stacks_list (STL list of Stack*) into STACKS (2-D array of Stack*)
	STACKS = new Stack**[N_ROWS];
	for(int row=0; row < N_ROWS; row++)
		STACKS[row] = new Stack*[N_COLS];
	for(list<Stack*>::iterator i = stacks_list.begin(); i != stacks_list.end(); i++)
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

void SimpleVolume::initChannels ( ) throw (MyException) {
	char slice_fullpath[IM_STATIC_STRINGS_SIZE];

	sprintf(slice_fullpath, "%s/%s/%s", root_dir, STACKS[0][0]->getDIR_NAME(), STACKS[0][0]->getFILENAMES()[0]);
	IplImage* slice = cvLoadImage(slice_fullpath, CV_LOAD_IMAGE_ANYCOLOR);  //without CV_LOAD_IMAGE_ANYDEPTH, image is converted to 8-bits if needed
	if(!slice)
		throw MyException(std::string("Unable to load slice at \"").append(slice_fullpath).append("\"").c_str());
    CHANS = slice->nChannels;

	cvReleaseImage(&slice);
}


REAL_T *SimpleVolume::loadSubvolume_to_REAL_T(int V0,int V1, int H0, int H1, int D0, int D1)  throw (MyException) {

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
	REAL_T *subvol = new REAL_T[sbv_height * sbv_width * sbv_depth];

	//scanning of stacks matrix for data loading and storing into subvol
	Rect_t subvol_area;
	subvol_area.H0 = H0;
	subvol_area.V0 = V0;
	subvol_area.H1 = H1;
	subvol_area.V1 = V1;
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
	return subvol;
}


uint8 *SimpleVolume::loadSubvolume_to_UINT8(int V0,int V1, int H0, int H1, int D0, int D1, int *channels) throw (MyException) {

	//initializations
	V0 = (V0 == -1 ? 0	     : V0);
	V1 = (V1 == -1 ? DIM_V   : V1);
	H0 = (H0 == -1 ? 0	     : H0);
	H1 = (H1 == -1 ? DIM_H   : H1);
	D0 = (D0 == -1 ? 0		 : D0);
	D1 = (D1 == -1 ? DIM_D	 : D1);

	uint8 *subvol = 0;

	//allocation
	sint64 sbv_height = V1 - V0;
	sint64 sbv_width  = H1 - H0;
	sint64 sbv_depth  = D1 - D0;

    //initializing the number of channels with an undefined value (it will be detected from the first slice read)
    sint64 sbv_channels = -1;

	//scanning of stacks matrix for data loading and storing into subvol
	Rect_t subvol_area;
	subvol_area.H0 = H0;
	subvol_area.V0 = V0;
	subvol_area.H1 = H1;
	subvol_area.V1 = V1;
	bool first_time = true;

	for(int row=0; row<N_ROWS; row++) 
	{
		for(int col=0; col<N_COLS; col++)
		{
			Rect_t *intersect_area = STACKS[row][col]->Intersects(subvol_area);
			if(intersect_area)
			{
				// set input parameters
				int first_file = D0;
				int last_file  = D1 - 1;

				//LOCAL VARIABLES
				char slice_fullpath[IM_STATIC_STRINGS_SIZE];
				IplImage *slice;

				for(int k=0; k<sbv_depth; k++)
				{
					//building image path
					sprintf(slice_fullpath, "%s/%s/%s", root_dir, 
							STACKS[row][col]->getDIR_NAME(), 
							STACKS[row][col]->getFILENAMES()[D0+k]);
					//loading image
					slice = cvLoadImage(slice_fullpath, CV_LOAD_IMAGE_ANYCOLOR);  //without CV_LOAD_IMAGE_ANYDEPTH, image is converted to 8-bits if needed
					// old version: slice_img_i = cvLoadImage(slice_fullpath, CV_LOAD_IMAGE_GRAYSCALE | CV_LOAD_IMAGE_ANYDEPTH);
					if(!slice)
					{
						char msg[IM_STATIC_STRINGS_SIZE];
						sprintf(msg,"in Stack[%d,%d]::loadStack(%d,%d): unable to open image \"%s\". Wrong path or format.\nSupported formats are BMP, DIB, JPEG, JPG, JPE, PNG, PBM, PGM, PPM, SR, RAS, TIFF, TIF", 
							STACKS[row][col]->getROW_INDEX(), STACKS[row][col]->getCOL_INDEX(), first_file, last_file, slice_fullpath);
						throw MyException(msg);
					}		

					//if this is the first time a slice is loaded, detecting the number of channels and safely allocating memory for data
                    if(first_time)
                    {
                        first_time = false;
                        sbv_channels = slice->nChannels;
                        if(sbv_channels != 1 && sbv_channels != 3)
                            throw MyException(std::string("Unsupported number of channels at \"").append(slice_fullpath).append("\". Only 1 and 3-channels images are supported").c_str());

                        try
                        {
                            subvol = new uint8[sbv_height * sbv_width * sbv_depth * sbv_channels];
                        }
                        catch(...){throw MyException("in StackedVolume::loadSubvolume_to_UINT8: unable to allocate memory");}
                    }
                    //otherwise checking that all the other slices have the same bitdepth of the first one
                    else if(slice->nChannels != sbv_channels)
                        throw MyException(std::string("Image depth mismatch at slice at \"").append(slice_fullpath).append("\": all slices must have the same bitdepth").c_str());

					//computing offsets
                    int slice_step = slice->widthStep / sizeof(uint8);
                    int ABS_V_offset = V0 - STACKS[row][col]->getABS_V();
                    int ABS_H_offset = (H0 - STACKS[row][col]->getABS_H())*((int)sbv_channels);

					//different procedures for 1 and 3 channels images
                    int istart, iend, jstart, jend;
                    istart  = intersect_area->V0-V0;
                    iend    = intersect_area->V1-V0;
                    jstart  = intersect_area->H0-H0;
                    jend    = intersect_area->H1-H0;
                    if(sbv_channels == 1)
                    {
                        sint64 k_offset = k*sbv_height*sbv_width;
                        for(int i = istart; i < iend; i++)
                        {
                            uint8* slice_row = ((uint8*)slice->imageData) + (i+ABS_V_offset)*slice_step;
                            for(int j = jstart; j < jend; j++)
                                subvol[k_offset + i*sbv_width + j] = slice_row[j+ABS_H_offset];
                        }
                    }
                    else if(sbv_channels == 3)
                    {

                        sint64 offset1 =                                     k*sbv_height*sbv_width;
                        sint64 offset2 =   sbv_height*sbv_width*sbv_depth  + offset1;
                        sint64 offset3 = 2*sbv_height*sbv_width*sbv_depth  + k*sbv_height*sbv_width;
                        for(int i = istart; i < iend; i++)
                        {
                            uint8* slice_row = ((uint8*)slice->imageData) + (i+ABS_V_offset)*slice_step;
                            for(int j1 = jstart, j2 = jstart*3; j1 < jend; j1++, j2+=3)
                            {
                                subvol[offset1 + i*sbv_width + j1] = slice_row[j2 + ABS_H_offset + 2];
                                subvol[offset2 + i*sbv_width + j1] = slice_row[j2 + ABS_H_offset + 1];
                                subvol[offset3 + i*sbv_width + j1] = slice_row[j2 + ABS_H_offset];
                            }
                        }
                    }
                    else
                        throw MyException(std::string("Unsupported number of channels at \"").append(slice_fullpath).append("\". Only 1 and 3-channels images are supported").c_str());

					//releasing image
					cvReleaseImage(&slice);
				}
			}
		}
	}

    if(channels)
        *channels = (int)sbv_channels;
	return subvol;
}
