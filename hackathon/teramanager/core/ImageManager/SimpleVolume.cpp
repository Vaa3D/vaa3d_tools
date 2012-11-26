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
*       Bria, A., Iannello, G., "A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Datasets", submitted on July 2012 to IEEE Transactions on Information Technology in Biomedicine.
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

using namespace std;


SimpleVolume::SimpleVolume(const char* _root_dir)  throw (MyException)
: VirtualVolume(_root_dir,(float)1.0,(float)1.0,(float)1.0) 
{
	init();
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
	sprintf(stack_i_j_path,"");
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


REAL_T *SimpleVolume::loadSubvolume_to_REAL_T(int V0,int V1, int H0, int H1, int D0, int D1)  throw (MyException) {
	#if IM_VERBOSE > 3
	printf("\t\t\t\tin SimpleVolume::loadSubvolume(V0=%d, V1=%d, H0=%d, H1=%d, D0=%d, D1=%d%s)\n", V0, V1, H0, H1, D0, D1, (involved_stacks? ", involved_stacks" : ""));
	#endif

	//initializations
	V0 = (V0 == -1 ? 0	     : V0);
	V1 = (V1 == -1 ? DIM_V   : V1);
	H0 = (H0 == -1 ? 0	     : H0);
	H1 = (H1 == -1 ? DIM_H   : H1);
	D0 = (D0 == -1 ? 0		 : D0);
	D1 = (D1 == -1 ? DIM_D	 : D1);

	//allocation
	int sbv_height = V1 - V0;
	int sbv_width  = H1 - H0;
	int sbv_depth  = D1 - D0;
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
				#if IM_VERBOSE > 4
				printf("\t\t\t\tin SimpleVolume::loadSubvolume(): using STACK[%d,%d] for area %d-%d(V) x %d-%d(H)\n", row, col, intersect_area->V0-V0, intersect_area->V1-V0, intersect_area->H0-H0, intersect_area->H1-H0);
				#endif

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
	return 0;
}
