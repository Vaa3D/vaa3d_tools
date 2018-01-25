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

/******************
*    CHANGELOG    *
*******************
* 2017-04-01.  Giulio.     @ADDED code for completing the management of multi-layer stitching
* 2017-02-10.  Giulio.     @ADDED in merge methods added a parameter to specify the blending algorithm to be used for layers 
*/


#include "StackStitcher2.h"

#include <iostream>
#include <string>
#include <sstream>
#include <limits>
#include <list>
#include <ctime>

#include "ProgressBar.h"
#include "PDAlgo.h"
#include "TPAlgo2.h"
#include "DisplacementMIPNCC.h"

#include "StackedVolume.h"
#include "UnstitchedVolume.h"
#include "IOPluginAPI.h"

using namespace iomanager;
using namespace iim;
using namespace std;

//initialization of class members
double StackStitcher2::time_displ_comp=0;
double StackStitcher2::time_merging=0;
double StackStitcher2::time_stack_desc=0;
double StackStitcher2::time_stack_restore=0;
double StackStitcher2::time_multiresolution=0;
double StackStitcher2::stack_marging_old_val=std::numeric_limits<double>::max();
bool StackStitcher2::blank_line_drawn = false;


//struct coord_2D{int V,H;};
//struct stripe_2Dcoords{coord_2D up_left, bottom_right;};
//struct stripe_corner{int h,H; bool up;};
//struct stripe_2Dcorners{std::list<stripe_corner> ups, bottoms, merged;};
//bool compareCorners (stripe_corner first, stripe_corner second)
//{ return ( first.H < second.H ); }

StackStitcher2::StackStitcher2(MultiLayersVolume* _volume)
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
* [start/end_...]		: layers interval that possible identify the portion of volume to be processed.
*						  If not given, all stacks will be processed.
* [displ_max_...]		: maximum displacements along VHD between two  adjacent stacks  taking the given over-
*						  lap as reference. These parameters, together with <overlap_...>,can be used to iden-
*						  tify the region of interest where the correspondence between the given stacks has to
*						  be found. When used, these parameters have to be tuned with respect to the precision 
*						  of the motorized stages. 
*						  If not given, value S_DISPL_MAX_VHD is assigned.
* [tile_idx_V ...]	    : indices of tiles to be aligned
* [show_progress_bar]	: enables/disables progress bar with estimated time remaining.
**************************************************************************************************************/
void StackStitcher2::computeTileDisplacements(int algorithm_type, int start_layer, int end_layer, 
											  int displ_max_V, int displ_max_H, int displ_max_D, bool show_progress_bar) throw (IOException) {

	#if S_VERBOSE>2
	printf("\t\t\t....in StackStitcher2::computeDisplacements(..., start_layer = %d, end_layer = %d, tile_idx_V = %d, tile_idx_H = %d, displ_max_V = %d, displ_max_H = %d, displ_max_D = %d)\n",
		start_layer, end_layer, tile_idx_V, tile_idx_H, displ_max_V, displ_max_H, displ_max_D);
	#endif

	//LOCAL VARIABLES
	char buffer[S_STATIC_STRINGS_SIZE];
	PDAlgo *algorithm;					//stores the reference to the algorithm to be used for pairwise displacement computation
	int dim_V, dim_H;			// lowest extension between two adjacent layers
	int n_substk_V, n_substk_H;
	int displ_computations;						//stores the number of displacement computations (used for progress bar)
	int displ_computations_idx;					//counter for displacements computations
	int i, j, k;
	iim::real32 *sstk_A, *sstk_B;				//substacks to be aligned 
	int D0_A, D1_A, D0_B, D1_B;

	start_layer	 = start_layer	== -1 ? 0 : start_layer;
	end_layer	 = end_layer	== -1 ? volume->getN_LAYERS() - 1 : end_layer;

	if(start_layer < 0 || start_layer > end_layer || end_layer >= volume->getN_LAYERS())
	{
		sprintf(buffer, "in StackStitcher2::computeDisplacements(...): selected portion of volume layers LAYERS[%d,%d] must be in LAYERS[0,%d]",
				start_layer, end_layer, volume->getN_LAYERS()-1);
		throw IOException(buffer);
	}

	//Pairwise Displacement Algorithm initialization
	algorithm = PDAlgo::instanceAlgorithm(algorithm_type);

	// computing the number of substacks 
	n_substk_V = ((UnstitchedVolume *) (volume->getLAYER(start_layer)))->volume->getN_ROWS();
	n_substk_H = ((UnstitchedVolume *) (volume->getLAYER(start_layer)))->volume->getN_COLS();

	// getting height and width of tiles
	dim_V = ((UnstitchedVolume *) (volume->getLAYER(start_layer)))->volume->getStacksHeight();
	dim_H = ((UnstitchedVolume *) (volume->getLAYER(start_layer)))->volume->getStacksWidth();

	// all layers are assumend to have the same tile matrix

	//initializing the progress bar
    ts::ProgressBar::instance();
	if(show_progress_bar)
	{
		ts::ProgressBar::instance()->start("Pairwise displacement computation");
		ts::ProgressBar::instance()->setProgressValue(0,"Initializing...");
		ts::ProgressBar::instance()->display();
		displ_computations = n_substk_V * n_substk_H * (end_layer - start_layer);
		displ_computations_idx = 1;
	}

	for ( k=0; k<(volume->getN_LAYERS()-1); k++ ) {
		D0_A = volume->getLAYER_DIM(k,2) - volume->getOVERLAP_D(k);
		D1_A = D0_A + volume->getOVERLAP_D(k);
		D0_B = 0;
		D1_B = D0_B + volume->getOVERLAP_D(k);

		volume->initDISPS(k,n_substk_V,n_substk_H);

		for ( j=0; j<n_substk_H; j++ ) {
			//H1 = H0 + (j<r_substk_H ? d_substk_H+1 : d_substk_H);

			for ( i=0; i<n_substk_V; i++ ) {
				//V1 = V0 + (i<r_substk_V ? d_substk_V+1 : d_substk_V);

				if(show_progress_bar)
				{
					sprintf(buffer, "Displacement computation %d of %d", displ_computations_idx, displ_computations);
                                            ts::ProgressBar::instance()->setProgressValue((100.0f/displ_computations)*displ_computations_idx, buffer);
                                            ts::ProgressBar::instance()->display();
				}
				#ifdef S_TIME_CALC
				double proc_time = -TIME(0);
				#endif

				if ( ((UnstitchedVolume *) (volume->getLAYER(k)))->volume->getSTACKS()[i][j]->isComplete(D0_A,D1_A-1) &&
						((UnstitchedVolume *) (volume->getLAYER(k+1)))->volume->getSTACKS()[i][j]->isComplete(D0_B,D1_B-1) ) { // substacks can be aligned

					sstk_A = ((UnstitchedVolume *) (volume->getLAYER(k)))->volume->getSTACKS()[i][j]->loadImageStack(D0_A,D1_A-1);
					sstk_B = ((UnstitchedVolume *) (volume->getLAYER(k+1)))->volume->getSTACKS()[i][j]->loadImageStack(D0_B,D1_B-1);

					// insert displacement
					volume->insertDisplacement(i, j, k,
						algorithm->execute(sstk_A, dim_V, dim_H, (D1_A - D0_A), sstk_B, dim_V, dim_H, (D1_B - D0_B), 
																displ_max_V, displ_max_H, displ_max_D, dir_horizontal, dim_H ));

					((UnstitchedVolume *) (volume->getLAYER(k)))->volume->getSTACKS()[i][j]->releaseImageStack();
					sstk_A = 0;
					((UnstitchedVolume *) (volume->getLAYER(k+1)))->volume->getSTACKS()[i][j]->releaseImageStack();
					sstk_B = 0;

				}

				displ_computations_idx++;
				#ifdef S_TIME_CALC
				proc_time += TIME(0);
				StackStitcher2::time_displ_comp+=proc_time;
				proc_time = -TIME(0);
				#endif
			}
		} 
	}

	if(show_progress_bar)
	{
                ts::ProgressBar::instance()->setProgressValue(100, "Ended!");
                ts::ProgressBar::instance()->display();
	}
}


/*************************************************************************************************************
* Computes final stitched volume dimensions assuming that current <VirtualVolume> object contains  the correct 
* stack coordinates. The given parameters identify the possible VOI (Volume Of Interest). If these are not us-
* ed, the whole volume is  considered and the parameter <exclude_nonstitchable_stacks> is used to discard rows
* or columns with no stitchable stacks 
*
* exclude_nonstitchable_stacks: not used
* _ROW_START: fist coordinate of the up/left/top corner
* _ROW_END:   fist coordinate of the down/right/bottm corner
* _COL_START: second coordinate of the up/left/top corner
* _COL_END:   second coordinate of the down/right/bottm corner
* _D0:        third coordinate of the up/left/top corner
* _D1:        third coordinate of the down/right/bottm corner
**************************************************************************************************************/
void StackStitcher2::computeVolumeDims(int _ROW_START, int _ROW_END, 
									  int _COL_START, int _COL_END, int _D0, int _D1) throw (IOException)
{
        #if S_VERBOSE >2
	printf("\t\t\t....in StackStitcher2::computeVolumeDims(exclude_nonstitchable_stacks = %s, _ROW_START=%d, _ROW_END=%d, _COL_START=%d, _COL_END=%d, _D0=%d, _D1=%d)\n",
										   exclude_nonstitchable_stacks ? "true" : "false", _ROW_START,    _ROW_END,	_COL_START,	   _COL_END,	_D0,	_D1);
	#endif

	char errMsg[2000];
	//bool stitchable_sequence;

	////*** First DETERMINING the range of stacks to be stitched ***
 //       if(_ROW_START == -1 || exclude_nonstitchable_stacks)
 //           for(ROW_START=0, stitchable_sequence=false; ROW_START < volume->getN_ROWS() && !stitchable_sequence && exclude_nonstitchable_stacks; ROW_START++)
 //               for(int column_index=0; column_index < volume->getN_COLS() && !stitchable_sequence; column_index++)
 //                   stitchable_sequence = volume->getSTACKS()[ROW_START][column_index]->isStitchable();
	//else if(_ROW_START >= 0 && _ROW_START < volume->getN_ROWS())
 //           ROW_START = _ROW_START;
	//else
	//{
 //           sprintf(errMsg, "in StackStitcher::computeVolumeDims(...): _ROW_START (=%d) out of range [%d,%d]", _ROW_START, 0, volume->getN_ROWS()-1);
 //           throw IOException(errMsg);
	//}
 //       if(_COL_START == -1 || exclude_nonstitchable_stacks)
 //           for(COL_START=0, stitchable_sequence = false; COL_START < volume->getN_COLS() && !stitchable_sequence && exclude_nonstitchable_stacks; COL_START++)
 //               for(int row_index=0; row_index < volume->getN_ROWS() && !stitchable_sequence; row_index++)
 //                   stitchable_sequence = volume->getSTACKS()[row_index][COL_START]->isStitchable();
	//else if(_COL_START >= 0 && _COL_START < volume->getN_COLS())
 //           COL_START = _COL_START;
	//else
	//{
 //           sprintf(errMsg, "in StackStitcher::computeVolumeDims(...): _COL_START (=%d) out of range [%d,%d]", _COL_START, 0, volume->getN_COLS()-1);
 //           throw IOException(errMsg);
	//}
 //       if(_ROW_END == -1 || exclude_nonstitchable_stacks)
 //           for(ROW_END=volume->getN_ROWS()-1, stitchable_sequence = false; ROW_END>=0 && !stitchable_sequence && exclude_nonstitchable_stacks; ROW_END--)
 //               for(int column_index=0; column_index < volume->getN_COLS() && !stitchable_sequence; column_index++)
 //                   stitchable_sequence = volume->getSTACKS()[ROW_END][column_index]->isStitchable();
	//else if(_ROW_END >= ROW_START && _ROW_END < volume->getN_ROWS())
 //           ROW_END = _ROW_END;
	//else
	//{
 //           sprintf(errMsg, "in StackStitcher::computeVolumeDims(...): no stitchable stacks found in the selected interval or _ROW_END (=%d) out of range [%d,%d]", _ROW_END, ROW_START, volume->getN_ROWS()-1);
 //           throw IOException(errMsg);
	//}
 //       if(_COL_END == -1 || exclude_nonstitchable_stacks)
 //           for(COL_END=volume->getN_COLS()-1, stitchable_sequence = false; COL_END>=0 && !stitchable_sequence && exclude_nonstitchable_stacks; COL_END--)
 //               for(int row_index=0; row_index < volume->getN_ROWS() && !stitchable_sequence; row_index++)
 //                       stitchable_sequence = volume->getSTACKS()[row_index][COL_END]->isStitchable();
	//else if(_COL_END >= COL_START && _COL_END < volume->getN_COLS())
 //           COL_END = _COL_END;
	//else
	//{
 //           sprintf(errMsg, "in StackStitcher::computeVolumeDims(...): no stitchable stacks found in the selected interval  or _COL_END (=%d) out of range [%d,%d]", _COL_END, COL_START, volume->getN_COLS()-1);
 //           throw IOException(errMsg);
	//}

	////*** COMPUTING volume dimensions using the computed range of stacks ***
	//V0=std::numeric_limits<int>::max();
	//V1=std::numeric_limits<int>::min();
	//H0=std::numeric_limits<int>::max();
	//H1=std::numeric_limits<int>::min();
 //   D0=std::numeric_limits<int>::min();
 //   D1=std::numeric_limits<int>::max();

	V0 = (_ROW_START == -1) ? 0                  : _ROW_START;
	V1 = (_ROW_END == -1)   ? volume->getDIM_V() : _ROW_END;
	H0 = (_COL_START == -1) ? 0                  : _COL_START;
	H1 = (_COL_END == -1)   ? volume->getDIM_H() : _COL_END;
	D0 = (_D0 == -1)        ? 0                  : _D0;
	D1 = (_D1 == -1)        ? volume->getDIM_D() : _D1;

	//for(int j=COL_START; j<=COL_END; j++)
	//{
	//	if(volume->getSTACKS()[ROW_START][j]->getABS_V()<V0)
	//		V0 = volume->getSTACKS()[ROW_START][j]->getABS_V();

	//	if(volume->getSTACKS()[ROW_END][j]->getABS_V()>V1)
	//		V1 = volume->getSTACKS()[ROW_END][j]->getABS_V();
	//}
	//for(int i=ROW_START; i<=ROW_END; i++)
	//{
	//	if(volume->getSTACKS()[i][COL_START]->getABS_H()<H0)
	//		H0 = volume->getSTACKS()[i][COL_START]->getABS_H();

	//	if(volume->getSTACKS()[i][COL_END]->getABS_H()>H1)
	//		H1 = volume->getSTACKS()[i][COL_END]->getABS_H();
	//}
	//for(int i=ROW_START; i<=ROW_END; i++)
	//{
	//	for(int j=COL_START; j<=COL_END; j++)
	//	{
 //                       if(volume->getSTACKS()[i][j]->getABS_D()>D0)
	//			D0 = volume->getSTACKS()[i][j]->getABS_D();

 //                       if(volume->getSTACKS()[i][j]->getABS_D()<D1)
	//			D1 = volume->getSTACKS()[i][j]->getABS_D();
	//	}
	//}
	//H1+=volume->getStacksWidth();
	//V1+=volume->getStacksHeight();
 //       D1+=volume->getN_SLICES();

	////*** SELECTING a subvolume along D axis, if optional parameters _D0 and _D1 have been used ***
 //       if(_D0 != -1 && _D0 >= D0)
 //           D0 = _D0;
 //       if(_D1 != -1 && _D1 <= D1)
 //           D1 = _D1;

    #if S_VERBOSE >2
    printf("\t\t\t....in StackStitcher2::computeVolumeDims(): volume range is V[%d,%d], H[%d,%d], D[%d,%d]\n", V0, V1, H0, H1, D0, D1);
    #endif

	//*** FINAL CHECK ***
	if(V0 > V1 || H0 > H1 || D0 > D1)
	{
		sprintf(errMsg, "in StackStitcher2::computeVolumeDims(...): invalid volume ranges V[%d,%d], H[%d,%d], D[%d,%d]", V0, V1, H0, H1, D0, D1);
		throw IOException(errMsg);
	}
}

/*************************************************************************************************************
* Returns the (up = true -> TOP, up = false -> BOTTOM) V coordinate of the virtual stripe at <row_index> row. 
**************************************************************************************************************/
int StackStitcher2::getStripeABS_V(int row_index, bool up)
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
iim::real32 *StackStitcher2::getStripe(short row_index, short d_index, int restore_direction, StackRestorer* stk_rst,
								 int blending_algo)									        throw (IOException)
{
        #if S_VERBOSE >2
	printf("........in StackStitcher::getStripe(short row_index=%d, short d_index=%d, restore_direction=%d, blending_algo=%d)\n",
		row_index, d_index, restore_direction, blending_algo);
	#endif

	//LOCAL VARIABLES
	iim::real32* stripe = NULL;							//stripe, the result of merging all VirtualStack's of a row
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
	vm::VirtualStack  *l_stk    = NULL, *r_stk, *rr_stk;		//pointers to left stack, right stack and right-right stack respectively
	iim::real32 *slice_left = NULL, *slice_right;		//"iterating" images, because current method merges images 2-by-2
	double angle=0;									//angle between 0 and PI
	double delta_angle;								//angle step used to sample the overlapping zone in [0,PI]
	char errMsg[5000];								//buffer where to store error messages
	iim::real32 *stripe_ptr;								//buffer where to store the resulting stripe
	iim::real32 *rslice_ptr, *lslice_ptr;				//buffers where to store each loaded pair of right and left slices
	iim::sint64 i,j;										//pixel indexes
	iim::real32 (*blending)(double& angle, iim::real32& pixel1, iim::real32& pixel2); //pointer to blending function

	//retrieving blending function
	if(blending_algo == S_SINUSOIDAL_BLENDING)
		blending = sinusoidal_blending;
	else if(blending_algo == S_NO_BLENDING)
		blending = no_blending;
	else if(blending_algo == S_SHOW_STACK_MARGIN)
		blending = stack_margin;
	else
		throw IOException("in StackStitcher::getStripe(...): unrecognized blending function");

	//checking that <row_index> is not out of bounds
	if(row_index>=volume->getN_ROWS() || row_index < 0)
	{
		sprintf(errMsg, "in StackStitcher::getStripe(...): row %d to be merged is out of bounds [%d,%d]", row_index, 0, volume->getN_ROWS()-1);
		throw IOException(errMsg);
	}

	//checking that <d_index> is not out of bounds
	if(!(d_index>=D0 && d_index<D1))
	{
		sprintf(errMsg, "in StackStitcher::getStripe(...): d_index (= %d) is out of bounds [%d,%d]", d_index, D0, D1-1);
		throw IOException(errMsg);
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
	stripe = new iim::real32[height*width];

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
		StackStitcher2::time_stack_restore+=proc_time;
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
		StackStitcher2::time_merging+=proc_time;
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
void StackStitcher2::mergeTiles(std::string output_path, int slice_height, int slice_width, bool* resolutions, 
							   int _ROW_START, int _ROW_END, int _COL_START,
							   int _COL_END, int _D0, int _D1, int blending_algo, int intralayer_blending_algo, bool test_mode, bool show_progress_bar, 
							   const char* saved_img_format, int saved_img_depth)			throw (IOException)
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
    iim::sint64 height, width, depth;                                            //height, width and depth of the whole volume that covers all stacks
	iim::real32* buffer;								//buffer temporary image data are stored
	iim::real32* stripe_up=NULL;                                   //will contain up-stripe and down-stripe computed by calling 'getStripe' method
	double angle;								//angle between 0 and PI used to sample overlapping zone in [0,PI]
	double delta_angle;							//angle step
	int z_ratio, z_max_res;
        int n_stacks_V[S_MAX_MULTIRES], n_stacks_H[S_MAX_MULTIRES];             //array of number of tiles along V and H directions respectively at i-th resolution
        int **stacks_height[S_MAX_MULTIRES], **stacks_width[S_MAX_MULTIRES];	//array of matrices of tiles dimensions at i-th resolution
	//iim::sint64 u_strp_bottom_displ;
	//iim::sint64 d_strp_top_displ;
	//iim::sint64 u_strp_top_displ;
	//iim::sint64 d_strp_left_displ;
	//iim::sint64 u_strp_left_displ;
	//iim::sint64 d_strp_width;
	//iim::sint64 u_strp_width;
	//iim::sint64 dd_strp_top_displ;
	//iim::sint64 u_strp_d_strp_overlap;
	//iim::sint64 h_up, h_down, h_overlap;
	//stripe_2Dcoords  *stripesCoords;
	//stripe_2Dcorners *stripesCorners;
	int resolutions_size = 0;
	//StackRestorer *stk_rst = NULL;
	//iim::real32 *buffer_ptr, *ustripe_ptr, *dstripe_ptr;	
	iim::real32 (*blending)(double& angle, iim::real32& pixel1, iim::real32& pixel2);
	std::stringstream file_path[S_MAX_MULTIRES];

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
            throw IOException("in StackStitcher::mergeTiles(...): unrecognized blending function");

	// 2017-02-10. Giulio. @ADDED setting of the blending function to be used for intralayers merging
	if ( dynamic_cast<UnstitchedVolume *>((volume->getLAYER(0))) ) { // volumes of multilayer volume are unstitched 
		for ( int i=0; i<volume->getN_LAYERS(); i++ )
			((UnstitchedVolume *) (volume->getLAYER(i)))->setBLENDING_ALGO(intralayer_blending_algo);
	}

	// 2015-05-14. Giulio. @ADDED selection of IO plugin if not provided.
	if(iom::IMOUT_PLUGIN.compare("empty") == 0)
	{
		iom::IMOUT_PLUGIN = "tiff2D";
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
    slice_height = (int) (slice_height == -1 ? height : slice_height);
    slice_width  = (int) (slice_width  == -1 ? width  : slice_width);
    if(slice_height < S_MIN_SLICE_DIM || slice_width < S_MIN_SLICE_DIM)
    {
        char err_msg[5000];
        sprintf(err_msg,"The minimum dimension for both slice width and height is %d", S_MIN_SLICE_DIM);
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
                stacks_height[res_i][stack_row][stack_col] = ((int)height/POW_INT(2,res_i)) / n_stacks_V[res_i] + (stack_row < ((int)(height/POW_INT(2,res_i))) % n_stacks_V[res_i] ? 1:0);
                stacks_width [res_i][stack_row][stack_col] = ((int)width/POW_INT(2,res_i))  / n_stacks_H[res_i] + (stack_col < ((int)(width/POW_INT(2,res_i)))  % n_stacks_H[res_i] ? 1:0);
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
	z_ratio=(int) depth/z_max_res;
	buffer = new iim::real32[height*width*z_max_res];

	//#ifdef S_TIME_CALC
	//double proc_time;
	//#endif

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

			//buffer size along D is different when the remainder of the subdivision by z_max_res is considered
			int z_size = (z_parts<=z_ratio) ? z_max_res : (depth%z_max_res);

			//halvesampling resolution if current resolution is not the deepest one
			if(i!=0)	
				StackStitcher2::halveSample(buffer,(int)height/(POW_INT(2,i-1)),(int)width/(POW_INT(2,i-1)),z_size/(POW_INT(2,i-1)));

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
                                base_path << output_path << "/RES(" << (int)(height/POW_INT(2,i)) << "x" << (int)(width/POW_INT(2,i)) << "x" << (int)(depth/POW_INT(2,i)) << ")/";

				//looping on new stacks
				for(int stack_row = 0, start_height = 0, end_height = 0; stack_row < n_stacks_V[i]; stack_row++)
				{
					//incrementing end_height
					end_height = start_height + stacks_height[i][stack_row][0]-1;
						
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
						end_width  = start_width  + stacks_width [i][stack_row][stack_column]-1;
							
						//computing H_DIR_path and creating the directory the first time it is needed
						std::stringstream H_DIR_path;
						H_DIR_path << V_DIR_path.str() << "/" << this->getMultiresABS_V_string(i,start_height) << "_" << this->getMultiresABS_H_string(i,start_width);
						if(!test_mode && z==D0 && !vm::make_dir(H_DIR_path.str().c_str()))
						{
							char err_msg[S_STATIC_STRINGS_SIZE];
							sprintf(err_msg, "in mergeTiles(...): unable to create H_DIR = \"%s\"\n", H_DIR_path.str().c_str());
							throw IOException(err_msg);
						}

						//saving HERE
						for(int buffer_z=0; buffer_z<z_size/(POW_INT(2,i)); buffer_z++)
						{
							std::stringstream img_path;
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
										<< this->getMultiresABS_H_string(i,start_width)  << "_"
										// << abs_pos_z.str();							// Alessandro, 23/03/2013 - bug found: see above
										<< this->getMultiresABS_D_string(0, rel_pos_z);	// Alessandro, 23/03/2013 - we pass '0' because rel_pos_z is the relative Z
																						// pixel coordinate in the HIGHEST (i=0) resolution image space (see above).
							if(test_mode)
							{
								img_path.str("");
								img_path << volume->getLAYERS_DIR() << "/test_middle_slice";
							}
							VirtualVolume::saveImage(img_path.str(), buffer + buffer_z*((int)height/POW_INT(2,i))*((int)width/POW_INT(2,i)),(int)height/(POW_INT(2,i)),(int)width/(POW_INT(2,i)),start_height,end_height,start_width,end_width, saved_img_format, saved_img_depth);
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
		//iim::ref_sys temp = volume->getREF_SYS();  // required by clang compiler
		//iim::ref_sys reference = *((iim::ref_sys *) &temp); // the cast is needed because there are two ref_sys in different name spaces
		for(int res_i=0; res_i< resolutions_size; res_i++) {
			if(resolutions[res_i])
        	{
            	//---- Alessandro 2013-04-22 partial fix: wrong voxel size computation. In addition, the predefined reference system {1,2,3} may not be the right
            	//one when dealing with CLSM data. The right reference system is stored in the <StackedVolume> object. A possible solution to implement
            	//is to check whether <volume> is a pointer to a <StackedVolume> object, then specialize it to <StackedVolume*> and get its reference
            	//system.
				//---- Giulio 2013-08-23 fixed
				iim::StackedVolume temp_vol(file_path[res_i].str().c_str(),volume->getREF_SYS(), //reference,
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
	//if(stk_rst)
	//	delete stk_rst;
}

/*************************************************************************************************************
* Performs downsampling at a halved frequency on the given 3D image.  The given image is overwritten in order
* to store its halvesampled version without allocating any additional resources.
**************************************************************************************************************/
void StackStitcher2::halveSample(iim::real32* img, int height, int width, int depth)
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
	StackStitcher2::time_multiresolution+=proc_time;
	#endif
	
}

/*************************************************************************************************************
* For each pair of layers projects the best interlayer displacement and leaves one displacements that  has  to 
* be applied to the layer as a whole. 
* WARNING: this mathod it has to be used if layers are already stitched blocks (i.e. their internal diplacements 
* are not affected by interlayer displacements.
**************************************************************************************************************/
void StackStitcher2::projectDisplacements() throw (IOException)
{
	for ( int k=0; k<(volume->getN_LAYERS() - 1); k++ ) { // for each pair of layers i, i+1
		vector<Displacement *> temp_V(volume->getDISPS(k)->size());
		for ( int i=0; i<volume->getDISPS(k)->size(); i++ ) {
			vector<Displacement *> temp_H(volume->getDISPS(k)->at(i).size());
			temp_H = volume->getDISPS(k)->at(i);        // copies one row of the matrix of displacements
			Displacement::projectDisplacements(temp_H); // projects the displacements of the row
			temp_V.at(i) = temp_H.at(0);                // set the projection
		}
		Displacement::projectDisplacements(temp_V);     // projects along V
		// substitutes the matrix with a singleton
		volume->setDISPS(k,new vector< std::vector<Displacement *> >(1,vector<Displacement *>(1,temp_V.at(0))));
	}
}

/*************************************************************************************************************
* Assuming that for each pair of adjacent stacks  exists one  and only one displacement,  this displacement is 
* thresholded according to the given <reliability_threshold>. When a displacement is not reliable enough,  its
* parameters are set to default values (i.e. nominal motorized stage coordinates).
* Moreover, stacks which do not have any reliable single-direction displacements with all 4 neighbors are mar-
* ked as NON STITCHABLE.
**************************************************************************************************************/
void StackStitcher2::thresholdDisplacements(float reliability_threshold)					   throw (IOException)
{
	#if S_VERBOSE > 3
	printf("......in StackStitcher2::thresholdDisplacements(reliability_threshold = %.4f)\n", reliability_threshold);
	#endif

	vector< vector<Displacement *> > *disp;

	// 2017-02-26. Giulio. This not true any more. With interlayer global optimization all displacement have to be thresholded
  //  //checking precondition: one and only one displacement must exist for each pair of layers
  //  for(int i=0; i<(volume->getN_LAYERS() - 1); i++) {
		//disp = volume->getDISPS(i);
		//if(disp->size() != 1 || disp->at(0).size() != 1)
  //          throw IOException("in StackStitcher2::thresholdDisplacements(...): one and only displacement must exist for each pair of adjacent stacks.");
  //  }

	//thresholding displacements
	for(int i=0; i<(volume->getN_LAYERS() - 1); i++) {
		disp = volume->getDISPS(i);
		for ( int v=0; v<disp->size(); v++) {
			for ( int h=0; h<disp->at(v).size(); h++) {
				disp->at(v).at(h)->threshold(reliability_threshold);
			}
		}
	}
}

/*************************************************************************************************************
* Compute the tiles placement with a global optimization algorithm taking into account the alignment of the 
* whole 3D matrix of tiles.
* Update the internal representation of each layer.
**************************************************************************************************************/
void StackStitcher2::computeTilesPlacement(int algorithm_type) throw (IOException)
{
	#if S_VERBOSE > 3
	printf("......in StackStitcher2::computeTilesPlacement(algorithm_type = %d)\n", algorithm_type);
	#endif

	TPAlgo2 *algorithm = TPAlgo2::instanceAlgorithm(algorithm_type, volume);
	int **newABS_VHD = algorithm->execute();

	// recomputes layers size and coords
	for ( int layer=0; layer<volume->getN_LAYERS(); layer++ ) {
		((UnstitchedVolume *) (volume->getLAYER(layer)))->updateTilesPositions();
	}

	// update layers nominal position
	volume->updateLayerCoords();

	// set displacements of tile (0,0) of each layer (starting from 1) to the correct value (after global optimization)
	for ( int layer=1; layer<(volume->getN_LAYERS()); layer++ ) {
		// build a temporary descriptor
		NCC_descr_t descr;
		descr.coord[0] = newABS_VHD[layer][0] - volume->getLAYER_COORDS(layer,0); // V displacement
		descr.coord[1] = newABS_VHD[layer][1] - volume->getLAYER_COORDS(layer,1); // H displacement
		descr.coord[2] = newABS_VHD[layer][2] - volume->getLAYER_COORDS(layer,2); // D displacement
		descr.NCC_maxs[0] = descr.NCC_maxs[1] = descr.NCC_maxs[2] = 1.0;
		descr.NCC_widths[0] = descr.NCC_widths[1] = descr.NCC_widths[2] = 1;
		DisplacementMIPNCC *temp_disp = new DisplacementMIPNCC(descr); // substitute the displacement descriptor
		// update layer displacements
		volume->insertDisplacement(0,0,layer-1,(Displacement *)temp_disp); // displacement of layer i is in interlayer descriptor i-1
		delete []newABS_VHD[layer];
	}

	delete []newABS_VHD;
}

/*************************************************************************************************************
* Executes the compute tiles placement algorithm associated to the given ID <algorithm_type>
*
* Convention: meaning of indices
* - 0 is V
* - 1 is H
* - 2 is D
**************************************************************************************************************/
void StackStitcher2::computeLayersPlacement()								throw (IOException)
{
	#if S_VERBOSE > 3
	printf("......in StackStitcher2::computeTilesPlacement(algorithm_type = %d)\n", algorithm_type);
	#endif

	int min_disp[2] = {0, 0};    // minimum V/H offsets with respect to the coordinates of the first layer (0,0)
	int disp[3]     = {0, 0, 0}; // cumulative (absolute) displacement of current layer with respect to nominal position 
	int max_disp[2];             // maximum V/H indices with respect to the coordinates of the first layer (0,0)
	// displacement of layer 0 is assumed (0,0,0) 
	max_disp[0] = volume->getLAYER_DIM(0,0); 
	max_disp[1] = volume->getLAYER_DIM(0,1);

	// layer coords have initially the nominal value
	for(int i=0; i<(volume->getN_LAYERS() - 1); i++) {
		// dimensions V and H only
		for ( int j=0; j<2; j++ ) {
			disp[j] += volume->getDISPS(i)->at(0).at(0)->getDisplacement((direction) j);
			// disp[j] is the cumulative displacement of layer i+1 in direction j
			if ( disp[j] < min_disp[j] ) 
				min_disp[j] = disp[j];
			else if ( (disp[j] + volume->getLAYER_DIM(i+1,j)) > max_disp[j] )
				// layer i+1 has larger indices than previous layers
				max_disp[j] = disp[j] + volume->getLAYER_DIM(i+1,j);
			volume->addVHD_COORDS(i+1,j,disp[j]);
		}
		// along D we do not need the min/max displacements
		disp[2] += volume->getDISPS(i)->at(0).at(0)->getDisplacement(dir_depth);
		// coords[2] is the displacement along D at layer i+1
		volume->addVHD_COORDS(i+1,2,disp[2]);
	}
	// min_disp is the most negative displacement with repsect to the first layer
	// max_disp is the greatest extension with respect to the nominal volume origin

	// resize the layered volume
	int _DIM_V = -min_disp[0] + max_disp[0];
	int _DIM_H = -min_disp[1] + max_disp[1];
	int _DIM_D = volume->getDIM_D() + disp[2];
	volume->setDIMS(_DIM_V,_DIM_H,_DIM_D);

	// correct V and H coordinates only with respect to the new origin 
	for(int i=0; i<volume->getN_LAYERS(); i++) {
		for ( int j=0; j<2; j++ ) {
			volume->addVHD_COORDS(i,j,-min_disp[j]); // all layers have to be placed
		}
	}

	// adjust absolute volume coordinates (V and H only)
	float new_coord;
	new_coord = volume->getORG_V();
	new_coord += min_disp[0] * (volume->getVXL_V() / 1000.0f);
	volume->setORG_V(new_coord);
	new_coord = volume->getORG_H();
	new_coord += min_disp[1] * (volume->getVXL_H() / 1000.0f);
	volume->setORG_H(new_coord);
}

/*************************************************************************************************************
* Functions used to obtain absolute coordinates at different resolutions from relative coordinates
**************************************************************************************************************/
int StackStitcher2::getMultiresABS_V(int res, int REL_V)
{
	if(volume->getVXL_V() > 0)
		return (int) ROUND(volume->getABS_V( V0 + REL_V*POW_INT(2,res) )*10);
	else
		return (int) ROUND(volume->getABS_V( V0 - 1 + REL_V*POW_INT(2,res))*10 + volume->getVXL_V()*POW_INT(2,res)*10);
}
std::string StackStitcher2::getMultiresABS_V_string(int res, int REL_V)	
{
	std::stringstream multires_merging_x_pos;
	multires_merging_x_pos.width(6);
	multires_merging_x_pos.fill('0');
	multires_merging_x_pos << this->getMultiresABS_V(res, REL_V);
	return multires_merging_x_pos.str();
}
int StackStitcher2::getMultiresABS_H(int res, int REL_H)
{
	if(volume->getVXL_H() > 0)
		return (int) ROUND(volume->getABS_H( H0 + REL_H*POW_INT(2,res) )*10);
	else
		return (int) ROUND(volume->getABS_H( H0 - 1 + REL_H*POW_INT(2,res))*10  + volume->getVXL_H()*POW_INT(2,res)*10);
}
std::string StackStitcher2::getMultiresABS_H_string(int res, int REL_H)	
{
	std::stringstream multires_merging_y_pos;
	multires_merging_y_pos.width(6);
	multires_merging_y_pos.fill('0');
	multires_merging_y_pos << this->getMultiresABS_H(res, REL_H);
	return multires_merging_y_pos.str();
}
int StackStitcher2::getMultiresABS_D(int res, int REL_D)
{
	if(volume->getVXL_D() > 0)
		return (int) ROUND(volume->getABS_D( D0 + REL_D*POW_INT(2,res) )*10);
	else
		return (int) ROUND(volume->getABS_D( D0 - 1 + REL_D*POW_INT(2,res) )*10 + volume->getVXL_D()*POW_INT(2,res)*10);
}
std::string StackStitcher2::getMultiresABS_D_string(int res, int REL_D)	
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
void StackStitcher2::saveComputationTimes(const char *filename, MultiLayersVolume &stk_org, double total_time)
{
	//building filename with local time
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	char filepath[5000];
	sprintf(filepath, "%s/%s_(%04d_%02d_%02d___%02d%02d).txt", stk_org.getLAYERS_DIR(), filename, timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min);
	
	FILE *file = fopen(filepath, "w");
	if(file)
	{
		double acc = 0;
		if(StackStitcher2::time_displ_comp > 0)
		{
			fprintf(file, "time_displ_comp\t\t\t%.0f minutes,\t%.1f seconds\n", (StackStitcher2::time_displ_comp/60), StackStitcher2::time_displ_comp);
			acc+=StackStitcher2::time_displ_comp;
		}
		if(StackStitcher2::time_stack_desc > 0)
		{
			fprintf(file, "time_stack_desc\t\t\t%.0f minutes,\t%.1f seconds\n", (StackStitcher2::time_stack_desc/60), StackStitcher2::time_stack_desc);
			acc+=StackStitcher2::time_stack_desc;
		}
		if(StackStitcher2::time_merging > 0)
		{
			fprintf(file, "time_merging\t\t\t%.0f minutes,\t%.1f seconds\n", (StackStitcher2::time_merging/60), StackStitcher2::time_merging);
			acc+=StackStitcher2::time_merging;
		}
		if(StackStitcher2::time_stack_restore > 0)
		{
			fprintf(file, "time_stack_restore\t\t%.0f minutes,\t%.1f seconds\n", (StackStitcher2::time_stack_restore/60), StackStitcher2::time_stack_restore);
			acc+=StackStitcher2::time_stack_restore;
		}
		if(StackStitcher2::time_multiresolution > 0)
		{
			fprintf(file, "time_multiresolution\t%.0f minutes,\t%.1f seconds\n", (StackStitcher2::time_multiresolution/60), StackStitcher2::time_multiresolution);
			acc+=StackStitcher2::time_multiresolution;
		}
		if(acc > 0)
		{
			fprintf(file, "SUM\t\t\t\t\t\t%.0f minutes,\t%.1f seconds\n", (acc/60), acc);
			acc+=StackStitcher2::time_multiresolution;
		}

		if(iomanager::IOPluginFactory::getTimeIO() > 0)
			fprintf(file, "\nTIME I/O\t\t\t\t%.0f minutes,\t%.1f seconds\n", (iomanager::IOPluginFactory::getTimeIO()/60), iomanager::IOPluginFactory::getTimeIO());
		if(iomanager::IOPluginFactory::getTimeConversions() > 0)
			fprintf(file, "TIME I/O conversions\t%.0f minutes,\t%.1f seconds\n", (iomanager::IOPluginFactory::getTimeConversions()/60), iomanager::IOPluginFactory::getTimeConversions());

		if(total_time > 0)
		{
			fprintf(file, "Non-measured TIME\t\t%.0f minutes,\t%.1f seconds\n", ((total_time-iomanager::IOPluginFactory::getTimeIO()-iomanager::IOPluginFactory::getTimeConversions()-acc)/60), (total_time-iomanager::IOPluginFactory::getTimeIO()-iomanager::IOPluginFactory::getTimeConversions()-acc));
			fprintf(file, "TOTAL TIME\t\t\t\t%.0f minutes,\t%.1f seconds\n", (total_time/60), total_time);
		}

		fclose(file);
	}

}


void StackStitcher2::resetComputationTimes()
{
	StackStitcher2::time_displ_comp=0;
	StackStitcher2::time_merging=0;
	StackStitcher2::time_stack_desc=0;
	StackStitcher2::time_stack_restore=0;
	StackStitcher2::time_multiresolution=0;
	//IOManager::time_IO = 0;
	//IOManager::time_IO_conversions = 0;
}


void StackStitcher2::copyBlock2SubBuf ( iim::real32 *src, iim::real32 *dst, int dimi, int dimj, int dimk, 
	 									iim::sint64 s_stridej, iim::sint64 s_strideij, iim::sint64 d_stridej, iim::sint64 d_strideij ) {
	 						   
	iim::real32 *s_slice = src;
	iim::real32 *d_slice = dst;
	iim::real32 *s_stripe;
	iim::real32 *d_stripe;
	int i, k;
	
	for ( k=0; k<dimk; k++, s_slice+=s_strideij, d_slice+=d_strideij ) {
		for ( i=0, s_stripe=s_slice, d_stripe=d_slice; i<dimi; i++, s_stripe+=s_stridej, d_stripe+=d_stridej ) {
			memcpy(d_stripe,s_stripe,(dimj*sizeof(iim::real32)));
		}
	}
}

void StackStitcher2::addBlock2SubBuf ( iim::real32 *src, iim::real32 *dst, int dimi, int dimj, int dimk,
	 						iim::sint64 s_stridej, iim::sint64 s_strideij, iim::sint64 d_stridej, iim::sint64 d_strideij, 
							iim::real32 (*blending)(double&, iim::real32&, iim::real32&), double alpha ) {

	iim::real32 *s_slice = src;
	iim::real32 *d_slice = dst;
	iim::real32 *s_stripe;
	iim::real32 *d_stripe;
	int i, j, k;
	
	for ( k=0; k<dimk; k++, s_slice+=s_strideij, d_slice+=d_strideij ) {
		for ( i=0, s_stripe=s_slice, d_stripe=d_slice; i<dimi; i++, s_stripe+=s_stridej, d_stripe+=d_stridej ) {
			if ( blending ) {
				for ( j=0; j<dimj; j++ ) {
					d_stripe[j] = blending(alpha,d_stripe[j],s_stripe[j]); // for small alpha first parameter (destination buffer) is weighted more
				}
			}
			else { // overwrite buffer
				memcpy(d_stripe,s_stripe,(dimj*sizeof(iim::real32)));
			}
		}
	}
}

