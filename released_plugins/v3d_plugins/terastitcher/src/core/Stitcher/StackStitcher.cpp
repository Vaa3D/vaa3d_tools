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

#include <iostream>
#include <string>
#include <sstream>
#include "StackStitcher.h"
#include <limits>
#include <list>
#include <ctime>

#include "ProgressBar.h"
#include "S_config.h"
#include "PDAlgo.h"
#include "TPAlgo.h"
#include "StackRestorer.h"
#include "VM_config.h"
#include "DisplacementMIPNCC.h"

#include "../ImageManager/IM_config.h"
#include "../ImageManager/VirtualVolume.h"
#include "../ImageManager/StackedVolume.h"

using namespace iomanager;
using namespace volumemanager;

//initialization of class members
double StackStitcher::time_displ_comp=0;
double StackStitcher::time_merging=0;
double StackStitcher::time_stack_desc=0;
double StackStitcher::time_stack_restore=0;
double StackStitcher::time_multiresolution=0;
double StackStitcher::stack_marging_old_val=std::numeric_limits<double>::max();
bool StackStitcher::blank_line_drawn = false;


struct coord_2D{int V,H;};
struct stripe_2Dcoords{coord_2D up_left, bottom_right;};
struct stripe_corner{int h,H; bool up;};
struct stripe_2Dcorners{std::list<stripe_corner> ups, bottoms, merged;};
bool compareCorners (stripe_corner first, stripe_corner second)
{ return ( first.H < second.H ); }

StackStitcher::StackStitcher(volumemanager::VirtualVolume* _volume)
{
	#if S_VERBOSE > 2
	printf("\t\t\t\t......in StackStitcher::StackStitcher(VirtualVolume* _volume)\n");
	#endif
	volume = _volume;
	V0 = V1 = H0 = H1 = D0 = D1 = ROW_START = ROW_END = COL_START = COL_END = -1;
}


/*************************************************************************************************************
* Method to be called for displacement computation. <> parameters are mandatory, while [] are optional.
* <algorithm_type>		: ID of the pairwise displacement algorithm to be used.
* [start/end_...]		: rows/columns intervals that possible identify the portion of volume to be processed.
*						  If not given, all stacks will be processed.
* [overlap_...]			: expected overlaps between the given stacks along V and H directions.These values can
*   					  be used to determine the region of interest where the overlapping occurs. 
*						  If not given, default values are assigned by computing the  expected  overlaps using 
*						  the <MEC_...> members of the <VirtualVolume> object.
* [displ_max_...]		: maximum displacements along VHD between two  adjacent stacks  taking the given over-
*						  lap as reference. These parameters, together with <overlap_...>,can be used to iden-
*						  tify the region of interest where the correspondence between the given stacks has to
*						  be found. When used, these parameters have to be tuned with respect to the precision 
*						  of the motorized stages. 
*						  If not given, value S_DISPL_MAX_VHD is assigned.
* [subvol_DIM_D]		: desired subvolumes dimension  along D axis.  Each pair  of stacks is split into sub-
*						  volumes along D axis in order to use memory efficiently.   Hence, multiple displace-
*						  ments for each pair of adjacent stacks are computed. 
*						  If not given, value S_SUBVOL_DIM_D_DEFAULT is assigned;
* [restoreSPIM]			: enables SPIM artifacts removal (zebrated patterns) along the given direction.
* [restore_direction]	: direction of SPIM zebrated patterns to be removed.
* [show_progress_bar]	: enables/disables progress bar with estimated time remaining.
**************************************************************************************************************/
void StackStitcher::computeDisplacements(int algorithm_type, int start_row, int start_col, int end_row, 
										 int end_col, int overlap_V, int overlap_H, int displ_max_V, 
										 int displ_max_H, int displ_max_D, int subvol_DIM_D, bool restoreSPIM, 
										 int restore_direction, bool show_progress_bar)		throw (MyException)
{
	#if S_VERBOSE>2
	printf("\t\t\t....in StackStitcher::computeDisplacements(..., overlap_V = %d, overlap_H = %d, displ_max_V = %d, displ_max_H = %d, displ_max_D = %d, start_row = %d, start_col = %d, end_row = %d, end_col = %d, subvol_DIM_D = %d, restoreSPIM=%s, restore_dir=%d)\n",
		overlap_V, overlap_H, displ_max_V, displ_max_H, displ_max_D, start_row, start_col, end_row, end_col, subvol_DIM_D, (restoreSPIM ? "ENABLED" : "disabled"), restore_direction);
	#endif

	//LOCAL VARIABLES
	char buffer[S_STATIC_STRINGS_SIZE];
	int n_subvols;								//number of subvolumes along D axis
	int subvol_DIM_D_actual;					//actual subvolumes dimension along D axis. Maybe slightly different from <subvol_DIM_D>
	int subvol_DIM_D_k;
	bool row_wise;								//used to decide if processing stacks row-wise or column-wise
	int displ_computations;						//stores the number of displacement computations (used for progress bar)
	int displ_computations_idx;					//counter for displacements computations
	int i,j,k;									//loop variables
	PDAlgo *algorithm;							//stores the reference to the algorithm to be used for pairwise displacement computation
	VirtualStack *stk_A, *stk_B;						//store references of each pair of adjacent stacks where <stk_B> follows <stk_A> along V or H

	//checks of parameters
	overlap_V	 = overlap_V	== -1 ? volume->getOVERLAP_V() : overlap_V;
	overlap_H	 = overlap_H	== -1 ? volume->getOVERLAP_H() : overlap_H;
	start_row	 = start_row	== -1 ? 0 : start_row;
	start_col	 = start_col	== -1 ? 0 : start_col;
	end_row		 = end_row		== -1 ? volume->getN_ROWS() -1 : end_row;
	end_col		 = end_col		== -1 ? volume->getN_COLS() -1 : end_col;
	if(start_row < 0 || start_row > end_row || end_row >= volume->getN_ROWS() ||start_col < 0 || start_col > end_col || end_col >= volume->getN_COLS())
	{
		sprintf(buffer, "in StackStitcher::computeDisplacements(...): selected portion of volume stacks ROWS[%d,%d] COLS[%d,%d] must be in ROWS[0,%d] COLS[0,%d]",
				start_row, end_row, start_col, end_col, volume->getN_ROWS() -1, volume->getN_COLS() -1);
		throw MyException(buffer);
	}
	if((overlap_V < S_OVERLAP_MIN || overlap_V > volume->getStacksHeight()) && volume->getN_ROWS() > 1)
	{
		sprintf(buffer, "in StackStitcher::computeDisplacements(...): overlap_V(=%d) must be in [%d,%d]", overlap_V, S_OVERLAP_MIN, volume->getStacksHeight());
		throw MyException(buffer);
	}
	if((overlap_H < S_OVERLAP_MIN || overlap_H > volume->getStacksWidth()) && volume->getN_COLS() > 1)
	{
		sprintf(buffer, "in StackStitcher::computeDisplacements(...): overlap_H(=%d) must be in [%d,%d]", overlap_H,S_OVERLAP_MIN, volume->getStacksWidth());
		throw MyException(buffer);
	}
    /*if(subvol_DIM_D < S_SUBVOL_DIM_D_MIN || subvol_DIM_D > volume->getN_SLICES())
	{
		sprintf(buffer, "in StackStitcher::computeDisplacements(...): subvol_DIM_D(=%d) must be in [%d,%d]", subvol_DIM_D, S_SUBVOL_DIM_D_MIN, volume->getN_SLICES());
		throw MyException(buffer);
    }*/

	//Pairwise Displacement Algorithm initialization
	algorithm = PDAlgo::instanceAlgorithm(algorithm_type);

	//computing subvolumes partitioning parameters
	n_subvols = (int) ceil( ( (float) volume->getN_SLICES() )/ ( (float) subvol_DIM_D));
	subvol_DIM_D_actual = (int) floor( ( (float) volume->getN_SLICES() ) / ( (float) n_subvols));
	int z_start=0;											//necessary due to to 'z_start' parameter of 'alignStacks' method

	//initializing Restorer module
	StackRestorer *stk_rst = NULL;
	if(restoreSPIM)
		stk_rst = new StackRestorer(volume, n_subvols);

	//initializing the progress bar
        ProgressBar::instance();
	if(show_progress_bar)
	{
                ProgressBar::getInstance()->start("Pairwise displacement computation");
                ProgressBar::getInstance()->update(0,"Initializing...");
                ProgressBar::getInstance()->show();
		displ_computations = n_subvols*(2*(end_row-start_row+1)*(end_col-start_col+1)-((end_row-start_row+1)+(end_col-start_col+1)));
		displ_computations_idx = 1;
	}

	//processing first N\95mod(n_subvols) LAYERS that are 'subvol_DIM_D_actual +1' long
	row_wise = end_row-start_row>=end_col-start_col;
	for(k = 1; k <= n_subvols; k++)
	{
		subvol_DIM_D_k = k <= volume->getN_SLICES()%n_subvols ? subvol_DIM_D_actual + 1 : subvol_DIM_D_actual;

		//loading [start_row][_j_start]VirtualStack in memory
		volume->getSTACKS()[start_row][start_col]->loadImageStack(z_start,z_start+subvol_DIM_D_k-1);

		//scanning LAYER through columns OR through rows depending on row_wise value 
		for(i=(row_wise ? start_row : start_col); i<=(row_wise ? end_row : end_col); i++)
		{
			for(j=(row_wise ? start_col : start_row); j<=(row_wise ? end_col : end_row); j++)
			{
				stk_A = volume->getSTACKS()[(row_wise? i : j  )][(row_wise ? j:   i)];

				//if #rows>=#columns, checking if eastern VirtualStack exists, otherwise checking if southern VirtualStack exists
				if(j!=(row_wise ? end_col : end_row))
				{
					stk_B = volume->getSTACKS()[(row_wise? i : j+1)][(row_wise ? j+1: i)];

					//if #rows>=#columns, checking if we are at first row. If so, allocating eastern VirtualStack
					//if #rows<#columns, checking if we are at first column. If so, allocating southern VirtualStack
					if(i== (row_wise ? start_row : start_col ))
						stk_B->loadImageStack(z_start, z_start+subvol_DIM_D_k-1);

					if(show_progress_bar)
					{
						sprintf(buffer, "Displacement computation %d of %d", displ_computations_idx, displ_computations);
                                                ProgressBar::getInstance()->update((100.0f/displ_computations)*displ_computations_idx, buffer);
                                                ProgressBar::getInstance()->show();
					}

					#ifdef S_TIME_CALC
					double proc_time = -TIME(0);
					#endif
					volume->insertDisplacement(stk_A, stk_B, 
											  algorithm->execute(stk_A->getSTACKED_IMAGE(), stk_A->getHEIGHT(), stk_A->getWIDTH(), subvol_DIM_D_k,
											  stk_B->getSTACKED_IMAGE(), stk_B->getHEIGHT(), stk_B->getWIDTH(), subvol_DIM_D_k, displ_max_V, 
											  displ_max_H, displ_max_D, row_wise ? dir_horizontal : dir_vertical, row_wise ? overlap_H : overlap_V ));
					displ_computations_idx++;
					#ifdef S_TIME_CALC
					proc_time += TIME(0);
					StackStitcher::time_displ_comp+=proc_time;
					proc_time = -TIME(0);
					#endif
				}
				//if #rows>=#columns, checking if southern VirtualStack exists, otherwise checking if eastern VirtualStack exists
				if(i!=(row_wise ? end_row : end_col))
				{
					stk_B = volume->getSTACKS()[(row_wise? i+1 : j)][(row_wise ? j: i+1)];

					//allocating southern/eastern VirtualStack
					stk_B->loadImageStack(z_start, z_start+subvol_DIM_D_k-1);
						
					if(show_progress_bar)
					{
						sprintf(buffer, "Displacement computation %d of %d", displ_computations_idx, displ_computations);
                                                ProgressBar::getInstance()->update((100.0f/displ_computations)*displ_computations_idx, buffer);
                                                ProgressBar::getInstance()->show();
					}

					#ifdef S_TIME_CALC
					double proc_time = -TIME(0);
					#endif
					volume->insertDisplacement(stk_A, stk_B, 
											  algorithm->execute(stk_A->getSTACKED_IMAGE(), stk_A->getHEIGHT(), stk_A->getWIDTH(), subvol_DIM_D_k,
											  stk_B->getSTACKED_IMAGE(), stk_B->getHEIGHT(), stk_B->getWIDTH(), subvol_DIM_D_k, displ_max_V, 
											  displ_max_H, displ_max_D, row_wise ? dir_vertical : dir_horizontal, row_wise ? overlap_V : overlap_H ));
                                        displ_computations_idx++;
                                        #ifdef S_TIME_CALC
					proc_time += TIME(0);
						StackStitcher::time_displ_comp+=proc_time;
					proc_time = -TIME(0);
					#endif
				}

				if(restoreSPIM)
				{
					#ifdef S_TIME_CALC
					double proc_time = -TIME(0);
					#endif
					stk_rst->computeSubvolDescriptors(stk_A->getSTACKED_IMAGE(), stk_A, k-1, subvol_DIM_D_k);
					#ifdef S_TIME_CALC
					proc_time += TIME(0);
					StackStitcher::time_stack_desc+=proc_time;
					#endif
				}

				//deallocating current VirtualStack
				stk_A->releaseImageStack();
			}
		}

		//increasing z_start and decreasing remainder due to properly layer scanning
		z_start = z_start + subvol_DIM_D_k;
	}

	if(restoreSPIM)
	{
		#ifdef S_TIME_CALC
		double proc_time = -TIME(0);
		#endif
		stk_rst->finalizeAllDescriptors();
		char desc_file_path[5000];
		sprintf(desc_file_path, "%s/%s", volume->getSTACKS_DIR(), S_DESC_FILE_NAME);
		stk_rst->save(desc_file_path);
		delete stk_rst;
		#ifdef S_TIME_CALC
		proc_time += TIME(0);
		StackStitcher::time_stack_desc+=proc_time;
		#endif
	}

	if(show_progress_bar)
	{
                ProgressBar::getInstance()->update(100, "Ended!");
                ProgressBar::getInstance()->show();
	}
}

/*************************************************************************************************************
* Computes final stitched volume dimensions assuming that current <VirtualVolume> object contains  the correct 
* stack coordinates. The given parameters identify the possible VOI (Volume Of Interest). If these are not us-
* ed, the whole volume is  considered and the parameter <exclude_nonstitchable_stacks> is used to discard rows
* or columns with no stitchable stacks 
**************************************************************************************************************/
void StackStitcher::computeVolumeDims(bool exclude_nonstitchable_stacks, int _ROW_START, int _ROW_END, 
									  int _COL_START, int _COL_END, int _D0, int _D1) throw (MyException)
{
        #if S_VERBOSE >2
	printf("\t\t\t....in StackStitcher::computeVolumeDims(exclude_nonstitchable_stacks = %s, _ROW_START=%d, _ROW_END=%d, _COL_START=%d, _COL_END=%d, _D0=%d, _D1=%d)\n",
										   exclude_nonstitchable_stacks ? "true" : "false", _ROW_START,    _ROW_END,	_COL_START,	   _COL_END,	_D0,	_D1);
	#endif

	char errMsg[2000];
	bool stitchable_sequence;

	//*** First DETERMINING the range of stacks to be stitched ***
        if(_ROW_START == -1 || exclude_nonstitchable_stacks)
            for(ROW_START=0, stitchable_sequence=false; ROW_START < volume->getN_ROWS() && !stitchable_sequence && exclude_nonstitchable_stacks; ROW_START++)
                for(int column_index=0; column_index < volume->getN_COLS() && !stitchable_sequence; column_index++)
                    stitchable_sequence = volume->getSTACKS()[ROW_START][column_index]->isStitchable();
	else if(_ROW_START >= 0 && _ROW_START < volume->getN_ROWS())
            ROW_START = _ROW_START;
	else
	{
            sprintf(errMsg, "in StackStitcher::computeVolumeDims(...): _ROW_START (=%d) out of range [%d,%d]", _ROW_START, 0, volume->getN_ROWS()-1);
            throw MyException(errMsg);
	}
        if(_COL_START == -1 || exclude_nonstitchable_stacks)
            for(COL_START=0, stitchable_sequence = false; COL_START < volume->getN_COLS() && !stitchable_sequence && exclude_nonstitchable_stacks; COL_START++)
                for(int row_index=0; row_index < volume->getN_ROWS() && !stitchable_sequence; row_index++)
                    stitchable_sequence = volume->getSTACKS()[row_index][COL_START]->isStitchable();
	else if(_COL_START >= 0 && _COL_START < volume->getN_COLS())
            COL_START = _COL_START;
	else
	{
            sprintf(errMsg, "in StackStitcher::computeVolumeDims(...): _COL_START (=%d) out of range [%d,%d]", _COL_START, 0, volume->getN_COLS()-1);
            throw MyException(errMsg);
	}
        if(_ROW_END == -1 || exclude_nonstitchable_stacks)
            for(ROW_END=volume->getN_ROWS()-1, stitchable_sequence = false; ROW_END>=0 && !stitchable_sequence && exclude_nonstitchable_stacks; ROW_END--)
                for(int column_index=0; column_index < volume->getN_COLS() && !stitchable_sequence; column_index++)
                    stitchable_sequence = volume->getSTACKS()[ROW_END][column_index]->isStitchable();
	else if(_ROW_END >= ROW_START && _ROW_END < volume->getN_ROWS())
            ROW_END = _ROW_END;
	else
	{
            sprintf(errMsg, "in StackStitcher::computeVolumeDims(...): no stitchable stacks found in the selected interval or _ROW_END (=%d) out of range [%d,%d]", _ROW_END, ROW_START, volume->getN_ROWS()-1);
            throw MyException(errMsg);
	}
        if(_COL_END == -1 || exclude_nonstitchable_stacks)
            for(COL_END=volume->getN_COLS()-1, stitchable_sequence = false; COL_END>=0 && !stitchable_sequence && exclude_nonstitchable_stacks; COL_END--)
                for(int row_index=0; row_index < volume->getN_ROWS() && !stitchable_sequence; row_index++)
                        stitchable_sequence = volume->getSTACKS()[row_index][COL_END]->isStitchable();
	else if(_COL_END >= COL_START && _COL_END < volume->getN_COLS())
            COL_END = _COL_END;
	else
	{
            sprintf(errMsg, "in StackStitcher::computeVolumeDims(...): no stitchable stacks found in the selected interval  or _COL_END (=%d) out of range [%d,%d]", _COL_END, COL_START, volume->getN_COLS()-1);
            throw MyException(errMsg);
	}

	//*** COMPUTING volume dimensions using the computed range of stacks ***
	V0=std::numeric_limits<int>::max();
	V1=std::numeric_limits<int>::min();
	H0=std::numeric_limits<int>::max();
	H1=std::numeric_limits<int>::min();
        D0=std::numeric_limits<int>::min();
        D1=std::numeric_limits<int>::max();
	for(int j=COL_START; j<=COL_END; j++)
	{
		if(volume->getSTACKS()[ROW_START][j]->getABS_V()<V0)
			V0 = volume->getSTACKS()[ROW_START][j]->getABS_V();

		if(volume->getSTACKS()[ROW_END][j]->getABS_V()>V1)
			V1 = volume->getSTACKS()[ROW_END][j]->getABS_V();
	}
	for(int i=ROW_START; i<=ROW_END; i++)
	{
		if(volume->getSTACKS()[i][COL_START]->getABS_H()<H0)
			H0 = volume->getSTACKS()[i][COL_START]->getABS_H();

		if(volume->getSTACKS()[i][COL_END]->getABS_H()>H1)
			H1 = volume->getSTACKS()[i][COL_END]->getABS_H();
	}
	for(int i=ROW_START; i<=ROW_END; i++)
	{
		for(int j=COL_START; j<=COL_END; j++)
		{
                        if(volume->getSTACKS()[i][j]->getABS_D()>D0)
				D0 = volume->getSTACKS()[i][j]->getABS_D();

                        if(volume->getSTACKS()[i][j]->getABS_D()<D1)
				D1 = volume->getSTACKS()[i][j]->getABS_D();
		}
	}
	H1+=volume->getStacksWidth();
	V1+=volume->getStacksHeight();
        D1+=volume->getN_SLICES();

	//*** SELECTING a subvolume along D axis, if optional parameters _D0 and _D1 have been used ***
        if(_D0 != -1 && _D0 >= D0)
            D0 = _D0;
        if(_D1 != -1 && _D1 <= D1)
            D1 = _D1;

    #if S_VERBOSE >2
    printf("\t\t\t....in StackStitcher::computeVolumeDims(): volume range is V[%d,%d], H[%d,%d], D[%d,%d]\n", V0, V1, H0, H1, D0, D1);
    #endif

	//*** FINAL CHECK ***
	if(V0 > V1 || H0 > H1 || D0 > D1)
	{
		sprintf(errMsg, "in StackStitcher::computeVolumeDims(...): invalid volume ranges V[%d,%d], H[%d,%d], D[%d,%d]", V0, V1, H0, H1, D0, D1);
		throw MyException(errMsg);
	}
}

/*************************************************************************************************************
* Returns the (up = true -> TOP, up = false -> BOTTOM) V coordinate of the virtual stripe at <row_index> row. 
**************************************************************************************************************/
int StackStitcher::getStripeABS_V(int row_index, bool up)
{
	#if S_VERBOSE >4
	printf("\t\t\t\t\t...in StackStitcher::getStripeABS_V(int row_index=%d, bool up=%s)\n", row_index, up ? "true" : "false");
	#endif

	//computing absolute V coordinates of stripe by simple 'min' or 'max' over absolute V-coordinates of stacks
	if(up)
	{
		int top_x_stripe=volume->getSTACKS()[row_index][COL_START]->getABS_V();

		for(int j=COL_START+1; j<=COL_END; j++)
			if(volume->getSTACKS()[row_index][j]->getABS_V()<top_x_stripe)
				top_x_stripe = volume->getSTACKS()[row_index][j]->getABS_V();

		return top_x_stripe;
	}
	else
	{
		int bottom_x_stripe=volume->getSTACKS()[row_index][COL_START]->getABS_V();

		for(int j=COL_START+1; j<=COL_END; j++)
			if(volume->getSTACKS()[row_index][j]->getABS_V()>bottom_x_stripe)
				bottom_x_stripe = volume->getSTACKS()[row_index][j]->getABS_V();
		bottom_x_stripe += volume->getStacksHeight();

		return bottom_x_stripe;
	}	
}


/*************************************************************************************************************
* Merges all slices of the given row at the given depth index, so obtaining the stripe that is returned.
* Uses [...]_blending() functions to blend pixels in  overlapping zones.  The appropriate blending function is
* selected by the [blending_algo] parameter. If a  <StackRestorer>  object has been passed,  each slice is re-
* stored before it is combined into the final stripe.
**************************************************************************************************************/
real_t* StackStitcher::getStripe(short row_index, short d_index, int restore_direction, StackRestorer* stk_rst,
								 int blending_algo)									        throw (MyException)
{
        #if S_VERBOSE >2
	printf("........in StackStitcher::getStripe(short row_index=%d, short d_index=%d, restore_direction=%d, blending_algo=%d)\n",
		row_index, d_index, restore_direction, blending_algo);
	#endif

	//LOCAL VARIABLES
	real_t* stripe = NULL;							//stripe, the result of merging all VirtualStack's of a row
	int width=0;									//width of stripe
	int height=0;									//height of stripe
	int stripe_V_top;								//top    V(ertical)   coordinate of current stripe
	int stripe_V_bottom;							//bottom V(ertical)   coordinate of current stripe
	int stripe_H_right;								//right  H(orizontal) coordinate of current stripe
	int stripe_H_left;								//left   H(orizontal) coordinate of current stripe
	int r_stk_top_displ , l_stk_top_displ;			//displacements of right and left stack from <stripe_V_top> respectively
	int rr_stk_left_displ;							//displacement of right-right stack from <stripe_H_left>
	int l_stk_right_displ;							//displacement of left stack from <stripe_H_right>
	int r_stk_left_displ, l_stk_left_displ;			//displacements of right and left stack from <stripe_H_left> respectively
	int stack_width  = volume->getStacksWidth();	//stacks H dimension
	int stack_height = volume->getStacksHeight();	//stacks V dimension
	VirtualStack  *l_stk    = NULL, *r_stk, *rr_stk;		//pointers to left stack, right stack and right-right stack respectively
	real_t *slice_left = NULL, *slice_right;		//"iterating" images, because current method merges images 2-by-2
	double angle=0;									//angle between 0 and PI
	double delta_angle;								//angle step used to sample the overlapping zone in [0,PI]
	char errMsg[5000];								//buffer where to store error messages
	real_t *stripe_ptr;								//buffer where to store the resulting stripe
	real_t *rslice_ptr, *lslice_ptr;				//buffers where to store each loaded pair of right and left slices
	sint64 i,j;										//pixel indexes
	real_t (*blending)(double& angle, real_t& pixel1, real_t& pixel2); //pointer to blending function

	//retrieving blending function
	if(blending_algo == S_SINUSOIDAL_BLENDING)
		blending = sinusoidal_blending;
	else if(blending_algo == S_NO_BLENDING)
		blending = no_blending;
	else if(blending_algo == S_SHOW_STACK_MARGIN)
		blending = stack_margin;
	else
		throw MyException("in StackStitcher::getStripe(...): unrecognized blending function");

	//checking that <row_index> is not out of bounds
	if(row_index>=volume->getN_ROWS() || row_index < 0)
	{
		sprintf(errMsg, "in StackStitcher::getStripe(...): row %d to be merged is out of bounds [%d,%d]", row_index, 0, volume->getN_ROWS()-1);
		throw MyException(errMsg);
	}

	//checking that <d_index> is not out of bounds
	if(!(d_index>=D0 && d_index<D1))
	{
		sprintf(errMsg, "in StackStitcher::getStripe(...): d_index (= %d) is out of bounds [%d,%d]", d_index, D0, D1-1);
		throw MyException(errMsg);
	}

	//computing current stripe VH coordinates and size
	stripe_V_top  = volume->getSTACKS()[row_index][COL_START]->getABS_V();
	stripe_V_bottom = stripe_V_top;
	stripe_H_left = volume->getSTACKS()[row_index][COL_START]->getABS_H();
	stripe_H_right = volume->getSTACKS()[row_index][COL_END]->getABS_H() +  volume->getStacksWidth();
	for(int j=COL_START+1; j<=COL_END; j++)
	{
		if(volume->getSTACKS()[row_index][j]->getABS_V() < stripe_V_top)
			stripe_V_top = volume->getSTACKS()[row_index][j]->getABS_V();

		if(volume->getSTACKS()[row_index][j]->getABS_V() > stripe_V_bottom)
			stripe_V_bottom = volume->getSTACKS()[row_index][j]->getABS_V();
	}
	stripe_V_bottom += volume->getStacksHeight();
	height=stripe_V_bottom-stripe_V_top;
	width=stripe_H_right-stripe_H_left;

	//ALLOCATING once for all the MEMORY SPACE for current stripe
	stripe = new real_t[height*width];

	//looping on all slices with row='row_index'
	stripe_ptr = stripe;
	for(int column_index=COL_START; column_index<=COL_END; column_index++, angle=0)
	{
		l_stk  =  column_index !=COL_START ? volume->getSTACKS()[row_index][column_index-1] : NULL;
		r_stk  =  volume->getSTACKS()[row_index][column_index];
		rr_stk =  column_index !=COL_END   ? volume->getSTACKS()[row_index][column_index+1] : NULL;

		r_stk_top_displ               = r_stk->getABS_V()  - stripe_V_top;
		if(l_stk)   l_stk_top_displ   = l_stk->getABS_V()  - stripe_V_top;
		r_stk_left_displ              = r_stk->getABS_H()  - stripe_H_left;
		if(l_stk)   l_stk_left_displ  = l_stk->getABS_H()  - stripe_H_left;
		if(l_stk)   l_stk_right_displ = l_stk->getABS_H()  - stripe_H_left + stack_width;
		if(rr_stk)  rr_stk_left_displ = rr_stk->getABS_H() - stripe_H_left;

		//loading right slice (slice_right) into memory
		slice_right = r_stk->loadImageStack(d_index-r_stk->getABS_D(), d_index-r_stk->getABS_D());

		#ifdef S_TIME_CALC
		double proc_time = -TIME(0);
		#endif

		//restoring right slice if restoring is enabled
		if(stk_rst)
			stk_rst->repairSlice(slice_right,d_index-r_stk->getABS_D(), r_stk,restore_direction);
		#ifdef S_TIME_CALC
		proc_time += TIME(0);
		StackStitcher::time_stack_restore+=proc_time;
		proc_time = -TIME(0);
		#endif

		//setting delta_angle
		if(l_stk) delta_angle = PI/((l_stk->getABS_H()+stack_width-r_stk->getABS_H())-1);
                angle = 0;

		//for every pair of adjacent slices, writing 2 different zones
		for(j=(l_stk ? r_stk_left_displ : 0); j<(rr_stk? rr_stk_left_displ : width); j++)
		{
			//FIRST ZONE: overlapping zone (iff l_stk exists)
			if(l_stk && j < l_stk_right_displ)
			{	
				stripe_ptr = &stripe[j];
				lslice_ptr = &slice_left [-l_stk_top_displ*stack_width+j-l_stk_left_displ];
				rslice_ptr = &slice_right[-r_stk_top_displ*stack_width+j-r_stk_left_displ];
				for(i=0; i<height; i++, stripe_ptr+=width, lslice_ptr+=stack_width, rslice_ptr+=stack_width)
					if(i - r_stk_top_displ >= 0 && i - r_stk_top_displ < stack_height && i - l_stk_top_displ >= 0 && i - l_stk_top_displ < stack_height)
                                                *stripe_ptr = blending(angle,*lslice_ptr,*rslice_ptr);
					else if (i - r_stk_top_displ >= 0 && i - r_stk_top_displ < stack_height)
						*stripe_ptr=*rslice_ptr;
					else if (i - l_stk_top_displ >= 0 && i - l_stk_top_displ < stack_height)
						*stripe_ptr= *lslice_ptr;

				angle=angle+delta_angle;
			}

			//SECOND ZONE: slice_right remainder by excluding overlapping zone between previous slice and overlapping zone between next slice
			else
			{
				rslice_ptr = &slice_right[-r_stk_top_displ*stack_width+j-r_stk_left_displ];
				for(i=0, stripe_ptr = &stripe[j]; i<height; i++, stripe_ptr+=width, rslice_ptr+=stack_width)
					if(i - r_stk_top_displ >= 0 && i - r_stk_top_displ < stack_height)
						*stripe_ptr=*rslice_ptr;
			}
		}

		#ifdef S_TIME_CALC
		proc_time += TIME(0);
		StackStitcher::time_merging+=proc_time;
		#endif

		//releasing memory allocated for last left VirtualStack
		slice_left = NULL;
		if(l_stk)
			l_stk->releaseImageStack();

		//moving to right slice_left
		slice_left=slice_right;
	}

	//releasing memory allocated for last right VirtualStack
	slice_right = NULL;
	volume->getSTACKS()[row_index][COL_END]->releaseImageStack();

	return stripe;
}

/*************************************************************************************************************
* Method to be called for tile merging. <> parameters are mandatory, while [] are optional.
* <output_path>			: absolute directory path where merged tiles have to be stored.
* [slice_height/width]	: desired dimensions of tiles  slices after merging.  It is actually an upper-bound of
*						  the actual slice dimensions, which will be computed in such a way that all tiles di-
*						  mensions can differ by 1 pixel only along both directions. If not given, the maximum
*						  allowed dimensions will be set, which will result in a volume composed by  one large 
*						  tile only.
* [resolutions]			: pointer to an array of S_MAX_MULTIRES  size which boolean entries identify the acti-
*						  vaction/deactivation of the i-th resolution.  If not given, all resolutions will  be
*						  activated.
* [exclude_nonstitc...] 
* [_...START/END]		
* [_D0/_D1]				: identify the possible VOI (Volume Of Interest). If these are not used, the whole vo-
*						  lume is  considered and the parameter <exclude_nonstitchable_stacks> is used to dis-
*						  card rows or columns with no stitchable stacks.
* [restoreSPIM]			: enables SPIM artifacts removal (zebrated patterns) along the given direction.
* [restore_direction]	: direction of SPIM zebrated patterns to be removed.
* [blending_algo]		: ID of the blending algorithm to be used in the overlapping regions.
* [test_mode]			: if enabled, the middle slice of the whole volume will be stitched and and  saved lo-
*						  cally. Stage coordinates will be used, s o this can be used to test  their precision
*						  as well as the selected reference system.
* [show_progress_bar]	: enables/disables progress bar with estimated time remaining.
* [saved_img_format]	: determines saved images format ("png","tif","jpeg", etc.).
* [saved_img_depth]		: determines saved images bitdepth (16 or 8).
**************************************************************************************************************/
void StackStitcher::mergeTiles(std::string output_path, int slice_height, int slice_width, bool* resolutions, 
							   bool exclude_nonstitchable_stacks, int _ROW_START, int _ROW_END, int _COL_START,
							   int _COL_END, int _D0, int _D1, bool restoreSPIM, int restore_direction,
							   int blending_algo, bool test_mode, bool show_progress_bar, 
							   const char* saved_img_format, int saved_img_depth)			throw (MyException)
{
        #if S_VERBOSE > 2
        printf("......in StackStitcher::mergeTiles(output_path=\"%s\", slice_height=%d, slice_width=%d, exclude_nonstitchable_stacks = %s, "
               "_ROW_START=%d, _ROW_END=%d, _COL_START=%d, _COL_END=%d, _D0=%d, _D1=%d, restoreSPIM = %s, restore_direction = %d, test_mode = %s, resolutions = { ",
                output_path.c_str(), slice_height, slice_width, (exclude_nonstitchable_stacks ? "true" : "false"), _ROW_START, _ROW_END,
                _COL_START, _COL_END, _D0, _D1, (restoreSPIM ? "ENABLED" : "disabled"), restore_direction, (test_mode ? "ENABLED" : "disabled"));
		for(int i=0; i<S_MAX_MULTIRES && resolutions; i++)
            printf("%d ", resolutions[i]);
        printf("}\n");
        #endif

	//LOCAL VARIABLES
        sint64 height, width, depth;                                            //height, width and depth of the whole volume that covers all stacks
	real_t* buffer;								//buffer temporary image data are stored
        real_t* stripe_up=NULL, *stripe_down;                                   //will contain up-stripe and down-stripe computed by calling 'getStripe' method
	double angle;								//angle between 0 and PI used to sample overlapping zone in [0,PI]
	double delta_angle;							//angle step
	int z_ratio, z_max_res;
        int n_stacks_V[S_MAX_MULTIRES], n_stacks_H[S_MAX_MULTIRES];             //array of number of tiles along V and H directions respectively at i-th resolution
        int **stacks_height[S_MAX_MULTIRES], **stacks_width[S_MAX_MULTIRES];	//array of matrices of tiles dimensions at i-th resolution
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
	real_t *buffer_ptr, *ustripe_ptr, *dstripe_ptr;	
	real_t (*blending)(double& angle, real_t& pixel1, real_t& pixel2);
	std::stringstream file_path[S_MAX_MULTIRES];

	//retrieving blending function
	if(blending_algo == S_SINUSOIDAL_BLENDING)
            blending = sinusoidal_blending;
	else if(blending_algo == S_NO_BLENDING)
            blending = no_blending;
	else if(blending_algo == S_SHOW_STACK_MARGIN)
            blending = stack_margin;
	else
            throw MyException("in StackStitcher::getStripe(...): unrecognized blending function");

	//initializing the progress bar
	char progressBarMsg[200];
        ProgressBar::instance();
	if(show_progress_bar)
	{
            ProgressBar::getInstance()->start("Multiresolution tile merging");
            ProgressBar::getInstance()->update(0,"Initializing...");
            ProgressBar::getInstance()->show();
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
    slice_height = (slice_height == -1 ? height : slice_height);
    slice_width  = (slice_width  == -1 ? width  : slice_width);
    if(slice_height < S_MIN_SLICE_DIM || slice_width < S_MIN_SLICE_DIM)
    {
        char err_msg[5000];
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
        n_stacks_V[res_i] = ceil ( (height/POW_INT(2,res_i)) / (float) slice_height );
        n_stacks_H[res_i] = ceil ( (width/POW_INT(2,res_i))  / (float) slice_width  );
        stacks_height[res_i] = new int *[n_stacks_V[res_i]];
        stacks_width[res_i]  = new int *[n_stacks_V[res_i]];
        for(int stack_row = 0; stack_row < n_stacks_V[res_i]; stack_row++)
        {
            stacks_height[res_i][stack_row] = new int[n_stacks_H[res_i]];
            stacks_width [res_i][stack_row] = new int[n_stacks_H[res_i]];
            for(int stack_col = 0; stack_col < n_stacks_H[res_i]; stack_col++)
            {
                stacks_height[res_i][stack_row][stack_col] = (height/POW_INT(2,res_i)) / n_stacks_V[res_i] + (stack_row < ((int)(height/POW_INT(2,res_i))) % n_stacks_V[res_i] ? 1:0);
                stacks_width [res_i][stack_row][stack_col] = (width/POW_INT(2,res_i))  / n_stacks_H[res_i] + (stack_col < ((int)(width/POW_INT(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
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
                sprintf(err_msg, "in mergeTiles(...): unable to create DIR = \"%s\"\n", file_path[res_i].str().c_str());
                throw MyException(err_msg);
            }

			//Alessandro - 23/03/2013: saving original volume XML descriptor into each folder
			char xmlPath[S_STATIC_STRINGS_SIZE];
			sprintf(xmlPath, "%s/original_volume_desc.xml", file_path[res_i].str().c_str());
			volume->saveXML(0, xmlPath);
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

	//ALLOCATING  the MEMORY SPACE for image buffer
	z_max_res = POW_INT(2,resolutions_size-1);
	z_ratio=depth/z_max_res;
	buffer = new real_t[height*width*z_max_res];

	#ifdef S_TIME_CALC
	double proc_time;
	#endif

	for(sint64 z = this->D0, z_parts = 1; z < this->D1; z += z_max_res, z_parts++)
	{
		for(sint64 k = 0; k < ( z_parts <= z_ratio ? z_max_res : depth%z_max_res ); k++)
		{
			//updating the progress bar
			if(show_progress_bar)
			{	
				sprintf(progressBarMsg, "Merging slice %d of %d",((uint32)(z-D0+k+1)),(uint32)depth);
                                ProgressBar::getInstance()->update(((float)(z-D0+k+1)*100/(float)depth), progressBarMsg);
                                ProgressBar::getInstance()->show();
			}

			//looping on all stripes
			for(int row_index=ROW_START; row_index<=ROW_END; row_index++)
			{
				//loading down stripe
				if(row_index==ROW_START) stripe_up = NULL;
				stripe_down = this->getStripe(row_index,z+k, restore_direction, stk_rst, blending_algo);

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
                                ProgressBar::getInstance()->updateInfo(progressBarMsg);
                                ProgressBar::getInstance()->show();
			}

			//buffer size along D is different when the remainder of the subdivision by z_max_res is considered
			int z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

			//halvesampling resolution if current resolution is not the deepest one
			if(i!=0)	
				StackStitcher::halveSample(buffer,height/(POW_INT(2,i-1)),width/(POW_INT(2,i-1)),z_size/(POW_INT(2,i-1)));

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
					if(!test_mode && z==D0 && !make_dir(V_DIR_path.str().c_str()))
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
						if(!test_mode && z==D0 && !make_dir(H_DIR_path.str().c_str()))
						{
							char err_msg[S_STATIC_STRINGS_SIZE];
							sprintf(err_msg, "in mergeTiles(...): unable to create H_DIR = \"%s\"\n", H_DIR_path.str().c_str());
							throw MyException(err_msg);
						}

						//saving HERE
						for(int buffer_z=0; buffer_z<z_size/(POW_INT(2,i)); buffer_z++)
						{
							std::stringstream img_path;
							int rel_pos_z = POW_INT(2,i)*buffer_z+z-D0;		// Alessandro, 23/03/2013 - see below. This is the relative Z pixel coordinate in the 
																			// highest resolution image space. '-D0' is necessary to make it relative, since
																			// getMultiresABS_D_string(...) accepts relative coordinates only.

							/*std::stringstream abs_pos_z;
							abs_pos_z.width(6);
							abs_pos_z.fill('0');
							abs_pos_z << (int)(POW_INT(2,i)*buffer_z+z);*/	// Alessandro, 23/03/2013 - bug found: we are saving the image space coordinate (in pixels) 
																			// instead of the volume space coordinate (in tenths of microns)
							img_path << H_DIR_path.str() << "/" 
										<< this->getMultiresABS_V_string(i,start_height) << "_" 
										<< this->getMultiresABS_H_string(i,start_width)  << "_"
										// << abs_pos_z.str();							// Alessandro, 23/03/2013 - bug found: see above
										<< this->getMultiresABS_D_string(0, rel_pos_z);	// Alessandro, 23/03/2013 - we pass '0' because rel_pos_z is the relative Z
																						// pixel coordinate in the HIGHEST (i=0) resolution image space (see above).
							if(test_mode)
							{
								img_path.str("");
								img_path << volume->getSTACKS_DIR() << "/test_middle_slice";
							}
							IOManager::saveImage(img_path.str(), buffer + buffer_z*(height/POW_INT(2,i))*(width/POW_INT(2,i)),height/(POW_INT(2,i)),width/(POW_INT(2,i)),start_height,end_height,start_width,end_width, saved_img_format, saved_img_depth);
						}
						start_width  += stacks_width [i][stack_row][stack_column];
					}
					start_height += stacks_height[i][stack_row][0];
				}
			}
		}
	}

	if ( !test_mode ) {
		// reloads created volumes to generate .bin file descriptors at all resolutions
		ref_sys temp = volume->getREF_SYS();  // required by clang compiler
		iim::ref_sys reference = *((iim::ref_sys *) &temp); // the cast is needed because there are two ref_sys in different name spaces
		for(int res_i=0; res_i< resolutions_size; res_i++) {
			if(resolutions[res_i])
        	{
            	//---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
            	//one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
            	//is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
            	//system.
				//---- Giulio 2013-08-23 fixed
				iim::StackedVolume temp_vol(file_path[res_i].str().c_str(),reference,
							volume->getVXL_V()*pow(2.0f,res_i), volume->getVXL_H()*pow(2.0f,res_i),volume->getVXL_D()*pow(2.0f,res_i));
        	}
        }
	}


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

	//releasing allocated memory
	delete buffer;
	if(stk_rst)
		delete stk_rst;
}

/*************************************************************************************************************
* Performs downsampling at a halved frequency on the given 3D image.  The given image is overwritten in order
* to store its halvesampled version without allocating any additional resources.
**************************************************************************************************************/
void StackStitcher::halveSample(real_t* img, int height, int width, int depth)
{
	#ifdef S_TIME_CALC
	double proc_time = -TIME(0);
	#endif

	float A,B,C,D,E,F,G,H;
	for(int z=0; z<depth/2; z++)
	{
		for(int i=0; i<height/2; i++)
		{
			for(int j=0; j<width/2; j++)
			{
				//computing 8-neighbours
				A = img[2*z*width*height +2*i*width + 2*j];
				B = img[2*z*width*height +2*i*width + (2*j+1)];
				C = img[2*z*width*height +(2*i+1)*width + 2*j];
				D = img[2*z*width*height +(2*i+1)*width + (2*j+1)];
				E = img[(2*z+1)*width*height +2*i*width + 2*j];
				F = img[(2*z+1)*width*height +2*i*width + (2*j+1)];
				G = img[(2*z+1)*width*height +(2*i+1)*width + 2*j];
				H = img[(2*z+1)*width*height +(2*i+1)*width + (2*j+1)];

				//computing mean
				img[z*(width/2)*(height/2)+i*(width/2)+j] = (A+B+C+D+E+F+G+H)/(float)8;
			}
		}
	}

	#ifdef S_TIME_CALC
	proc_time += TIME(0);
	StackStitcher::time_multiresolution+=proc_time;
	#endif
	
}

/*************************************************************************************************************
* For each stack, the vector of redundant displacements along D is projected into the displacement which embe-
* ds the most reliable parameters. After this operation, such vector will contain only the projected displace-
* ment. Where for a pair of adjacent stacks no displacement is available,  a displacement  is generated using
* nominal stage coordinates.
**************************************************************************************************************/
void StackStitcher::projectDisplacements()												   throw (MyException)
{
    #if S_VERBOSE > 3
    printf("......in StackStitcher::projectDisplacements()\n");
    #endif

    VirtualStack *stk;
    for(int i=0; i<volume->getN_ROWS(); i++)
    {
        for(int j=0; j<volume->getN_COLS(); j++)
        {
            stk = volume->getSTACKS()[i][j];
            if(i!= 0)
            {
                if(stk->getNORTH().size() != 0)
                    Displacement::projectDisplacements(stk->getNORTH());
                else
                {
                    DisplacementMIPNCC* nominal = new DisplacementMIPNCC(-volume->getDEFAULT_DISPLACEMENT_V(), 0, volume->getDEFAULT_DISPLACEMENT_D());
                    stk->getNORTH().push_back((Displacement*)nominal);
                }
            }
            if(j!= volume->getN_COLS() -1)
            {
                if(stk->getEAST().size() != 0)
                    Displacement::projectDisplacements(stk->getEAST());
                else
                {
                    DisplacementMIPNCC* nominal = new DisplacementMIPNCC(0, volume->getDEFAULT_DISPLACEMENT_H(), volume->getDEFAULT_DISPLACEMENT_D());
                    stk->getEAST().push_back((Displacement*)nominal);
                }
            }
            if(i!= volume->getN_ROWS() -1)
            {
                if(stk->getSOUTH().size() != 0)
                    Displacement::projectDisplacements(stk->getSOUTH());
                else
                {
                    DisplacementMIPNCC* nominal = new DisplacementMIPNCC(volume->getDEFAULT_DISPLACEMENT_V(), 0, volume->getDEFAULT_DISPLACEMENT_D());
                    stk->getSOUTH().push_back((Displacement*)nominal);
                }
            }
            if(j!= 0)
            {
                if(stk->getWEST().size() != 0)
                    Displacement::projectDisplacements(stk->getWEST());
                else
                {
                    DisplacementMIPNCC* nominal = new DisplacementMIPNCC(0, -volume->getDEFAULT_DISPLACEMENT_H(), volume->getDEFAULT_DISPLACEMENT_D());
                    stk->getWEST().push_back((Displacement*)nominal);
                }
            }
        }
    }
}

/*************************************************************************************************************
* Assuming that for each pair of adjacent stacks  exists one  and only one displacement,  this displacement is 
* thresholded according to the given <reliability_threshold>. When a displacement is not reliable enough,  its
* parameters are set to default values (i.e. nominal motorized stage coordinates).
* Moreover, stacks which do not have any reliable single-direction displacements with all 4 neighbors are mar-
* ked as NON STITCHABLE.
**************************************************************************************************************/
void StackStitcher::thresholdDisplacements(float reliability_threshold)					   throw (MyException)
{
	#if S_VERBOSE > 3
	printf("......in StackStitcher::thresholdDisplacements(reliability_threshold = %.4f)\n", reliability_threshold);
	#endif

	VirtualStack *stk;
	char errMsg[2000];

        //checking precondition: one and only one displacement must exist for each pair of adjacent stacks
        for(int i=0; i<volume->getN_ROWS(); i++)
            for(int j=0; j<volume->getN_COLS(); j++)
            {
                stk = volume->getSTACKS()[i][j];

                if(i != 0 && stk->getNORTH().size() != 1)
                    throw MyException("in StackStitcher::thresholdDisplacements(...): one and only displacement must exist for each pair of adjacent stacks.");

                if(j != 0 && stk->getWEST().size() != 1)
                    throw MyException("in StackStitcher::thresholdDisplacements(...): one and only displacement must exist for each pair of adjacent stacks.");

                if(i != (volume->getN_ROWS()-1)  && stk->getSOUTH().size() != 1)
                    throw MyException("in StackStitcher::thresholdDisplacements(...): one and only displacement must exist for each pair of adjacent stacks.");

                if(j != (volume->getN_COLS()-1) && stk->getEAST().size() != 1)
                    throw MyException("in StackStitcher::thresholdDisplacements(...): one and only displacement must exist for each pair of adjacent stacks.");
            }

	//thresholding displacements
	for(int i=0; i<volume->getN_ROWS(); i++)
		for(int j=0; j<volume->getN_COLS(); j++)
		{
			stk = volume->getSTACKS()[i][j];
			if(i!= 0) {
				if(stk->getNORTH().size() == 1)
					stk->getNORTH()[0]->threshold(reliability_threshold);
				else
				{
					sprintf(errMsg, "in StackStitcher::thresholdDisplacements(...): stack [%d,%d] must have exactly one displacement at NORTH", i, j);
					throw MyException(errMsg);
				}
			}
			if(j!= volume->getN_COLS() -1) {
				if(stk->getEAST().size() == 1)
					stk->getEAST()[0]->threshold(reliability_threshold);
				else
				{
					sprintf(errMsg, "in StackStitcher::thresholdDisplacements(...): stack [%d,%d] must have exactly one displacement at EAST", i, j);
					throw MyException(errMsg);
				}
			}
			if(i!= volume->getN_ROWS() -1) {
				if(stk->getSOUTH().size() == 1)
					stk->getSOUTH()[0]->threshold(reliability_threshold);
				else
				{
					sprintf(errMsg, "in StackStitcher::thresholdDisplacements(...): stack [%d,%d] must have exactly one displacement at SOUTH", i, j);
					throw MyException(errMsg);
				}
			}
			if(j!= 0) {
				if(stk->getWEST().size() == 1)
					stk->getWEST()[0]->threshold(reliability_threshold);
				else
				{
					sprintf(errMsg, "in StackStitcher::thresholdDisplacements(...): stack [%d,%d] must have exactly one displacement at WEST", i, j);
					throw MyException(errMsg);
				}
			}
		}

	//find stitchable stacks, i.e. stacks that have at least one reliable single-direction displacement
	bool stitchable;
	for(int i=0; i<volume->getN_ROWS(); i++)
		for(int j=0; j<volume->getN_COLS(); j++)
		{
			stitchable = false;
			stk = volume->getSTACKS()[i][j];
			if(i!= 0)
				for(int k=0; k<3; k++)
					stitchable = stitchable || (stk->getNORTH()[0]->getReliability(direction(k)) >= reliability_threshold);
			if(j!= volume->getN_COLS() -1)
				for(int k=0; k<3; k++)
					stitchable = stitchable || (stk->getEAST()[0]->getReliability(direction(k)) >= reliability_threshold);
			if(i!= volume->getN_ROWS() -1)
				for(int k=0; k<3; k++)
					stitchable = stitchable || (stk->getSOUTH()[0]->getReliability(direction(k)) >= reliability_threshold);
			if(j!= 0)
				for(int k=0; k<3; k++)
					stitchable = stitchable || (stk->getWEST()[0]->getReliability(direction(k)) >= reliability_threshold);
			volume->getSTACKS()[i][j]->setStitchable(stitchable);
		}
}

/*************************************************************************************************************
* Executes the compute tiles placement algorithm associated to the given ID <algorithm_type>
**************************************************************************************************************/
void StackStitcher::computeTilesPlacement(int algorithm_type)								throw (MyException)
{
	#if S_VERBOSE > 3
	printf("......in StackStitcher::computeTilesPlacement(algorithm_type = %d)\n", algorithm_type);
	#endif

	TPAlgo *algorithm = TPAlgo::instanceAlgorithm(algorithm_type, volume);
	algorithm->execute();
}

/*************************************************************************************************************
* Functions used to obtain absolute coordinates at different resolutions from relative coordinates
**************************************************************************************************************/
int StackStitcher::getMultiresABS_V(int res, int REL_V)
{
	if(volume->getVXL_V() > 0)
		return ROUND(volume->getABS_V( V0 + REL_V*POW_INT(2,res) )*10);
	else
		return ROUND(volume->getABS_V( V0 - 1 + REL_V*POW_INT(2,res))*10 + volume->getVXL_V()*POW_INT(2,res)*10);
}
std::string StackStitcher::getMultiresABS_V_string(int res, int REL_V)	
{
	std::stringstream multires_merging_x_pos;
	multires_merging_x_pos.width(6);
	multires_merging_x_pos.fill('0');
	multires_merging_x_pos << this->getMultiresABS_V(res, REL_V);
	return multires_merging_x_pos.str();
}
int StackStitcher::getMultiresABS_H(int res, int REL_H)
{
	if(volume->getVXL_H() > 0)
		return ROUND(volume->getABS_H( H0 + REL_H*POW_INT(2,res) )*10);
	else
		return ROUND(volume->getABS_H( H0 - 1 + REL_H*POW_INT(2,res))*10  + volume->getVXL_H()*POW_INT(2,res)*10);
}
std::string StackStitcher::getMultiresABS_H_string(int res, int REL_H)	
{
	std::stringstream multires_merging_y_pos;
	multires_merging_y_pos.width(6);
	multires_merging_y_pos.fill('0');
	multires_merging_y_pos << this->getMultiresABS_H(res, REL_H);
	return multires_merging_y_pos.str();
}
int StackStitcher::getMultiresABS_D(int res, int REL_D)
{
	if(volume->getVXL_D() > 0)
		return ROUND(volume->getABS_D( D0 + REL_D*POW_INT(2,res) )*10);
	else
		return ROUND(volume->getABS_D( D0 - 1 + REL_D*POW_INT(2,res) )*10 + volume->getVXL_D()*POW_INT(2,res)*10);
}
std::string StackStitcher::getMultiresABS_D_string(int res, int REL_D)	
{
	std::stringstream multires_merging_z_pos;
	multires_merging_z_pos.width(6);
	multires_merging_z_pos.fill('0');
	multires_merging_z_pos << this->getMultiresABS_D(res, REL_D);
	return multires_merging_z_pos.str();
}

/*************************************************************************************************************
* Functions used to save single phase time performances
**************************************************************************************************************/
void StackStitcher::saveComputationTimes(const char *filename, volumemanager::VirtualVolume &stk_org, double total_time)
{
	//building filename with local time
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	char filepath[5000];
	sprintf(filepath, "%s/%s_(%04d_%02d_%02d___%02d%02d).txt", stk_org.getSTACKS_DIR(), filename, timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min);
	
	FILE *file = fopen(filepath, "w");
	if(file)
	{
		double acc = 0;
		if(StackStitcher::time_displ_comp > 0)
		{
			fprintf(file, "time_displ_comp\t\t\t%.0f minutes,\t%.1f seconds\n", (StackStitcher::time_displ_comp/60), StackStitcher::time_displ_comp);
			acc+=StackStitcher::time_displ_comp;
		}
		if(StackStitcher::time_stack_desc > 0)
		{
			fprintf(file, "time_stack_desc\t\t\t%.0f minutes,\t%.1f seconds\n", (StackStitcher::time_stack_desc/60), StackStitcher::time_stack_desc);
			acc+=StackStitcher::time_stack_desc;
		}
		if(StackStitcher::time_merging > 0)
		{
			fprintf(file, "time_merging\t\t\t%.0f minutes,\t%.1f seconds\n", (StackStitcher::time_merging/60), StackStitcher::time_merging);
			acc+=StackStitcher::time_merging;
		}
		if(StackStitcher::time_stack_restore > 0)
		{
			fprintf(file, "time_stack_restore\t\t%.0f minutes,\t%.1f seconds\n", (StackStitcher::time_stack_restore/60), StackStitcher::time_stack_restore);
			acc+=StackStitcher::time_stack_restore;
		}
		if(StackStitcher::time_multiresolution > 0)
		{
			fprintf(file, "time_multiresolution\t%.0f minutes,\t%.1f seconds\n", (StackStitcher::time_multiresolution/60), StackStitcher::time_multiresolution);
			acc+=StackStitcher::time_multiresolution;
		}
		if(acc > 0)
		{
			fprintf(file, "SUM\t\t\t\t\t\t%.0f minutes,\t%.1f seconds\n", (acc/60), acc);
			acc+=StackStitcher::time_multiresolution;
		}

		if(IOManager::time_IO > 0)
			fprintf(file, "\nTIME I/O\t\t\t\t%.0f minutes,\t%.1f seconds\n", (IOManager::time_IO/60), IOManager::time_IO);
		if(IOManager::time_IO_conversions > 0)
			fprintf(file, "TIME I/O conversions\t%.0f minutes,\t%.1f seconds\n", (IOManager::time_IO_conversions/60), IOManager::time_IO_conversions);

		if(total_time > 0)
		{
			fprintf(file, "Non-measured TIME\t\t%.0f minutes,\t%.1f seconds\n", ((total_time-IOManager::time_IO-IOManager::time_IO_conversions-acc)/60), (total_time-IOManager::time_IO-IOManager::time_IO_conversions-acc));
			fprintf(file, "TOTAL TIME\t\t\t\t%.0f minutes,\t%.1f seconds\n", (total_time/60), total_time);
		}

		fclose(file);
	}

}


void StackStitcher::resetComputationTimes()
{
	StackStitcher::time_displ_comp=0;
	StackStitcher::time_merging=0;
	StackStitcher::time_stack_desc=0;
	StackStitcher::time_stack_restore=0;
	StackStitcher::time_multiresolution=0;
	IOManager::time_IO = 0;
	IOManager::time_IO_conversions = 0;
}
