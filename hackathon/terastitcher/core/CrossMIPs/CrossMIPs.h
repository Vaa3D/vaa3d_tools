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

/*
 * Alignements.h
 *
 *  Created on: September 2010
 *      Author: iannello
 *
 *  Last revision: May, 31 2011
 */


# ifndef ALIGNMENTS_H
# define ALIGNMENTS_H

# define NK            0

// valid side values
# define NORTH_SOUTH   0
# define WEST_EAST     1

# include "my_defs.h"

/***************************************** RESULT STRUCTURE ********************************************/
typedef struct {
	int coord[3];       // alignement as offset of the second stack with respect to the first one
	real_t NCC_maxs[3]; // reliability of alignment (-1=unreliable, 1=highly reliable)
	int NCC_widths[3];  // estimate of potential error in pixels
} NCC_descr_t;

/********************************* CONTROLLING PARAMETERS STRUCTURE ************************************/
typedef struct {
	bool enhance;     // boolean flag that enabled an enhancement step before performing NCC
	int maxIter;
	real_t maxThr;    // threshold for NCC maximum (below this threshold the NCC is considered unreliable)
	real_t widthThr;  // fraction of maximum used to evaluate the maximum width (belongs to [0,1])
	int wRangeThr;    // range used to evaluate maximum width (when maximum width is greater or equal to 
	                  // this value, width is set to INF_W
	real_t UNR_NCC;   // unreliable NCC peak value
	int INF_W;        // infinite NCC peak width
	int INV_COORD;    // invalid alignment
	int n_transforms; // used only if enhance=true; number of scaled linear transformations used to enhance the MIPs 
	real_t *percents; // used only if enhance=true; list of fractions of pixels; percents[i] is the fraction of pixels 
	                  // to be transformed with i-th transformation; percents[n_transforms-1] must be 1.00
	real_t *c;        // used only if enhance=true; list of values; the i-th transformations map pixels from value 
					  // c[i-1] to value c[i] 
} NCC_parms_t;

/***************************************** MAIN FUNCTION ***********************************************/
NCC_descr_t *norm_cross_corr_mips ( real_t *A, real_t *B, 
						    int dimk, int dimi, int dimj, int nk, int ni, int nj, 
							int delayk, int delayi, int delayj, int side, NCC_parms_t *NCC_params = 0 );
/*
 * returns an alignment between volume A and B; the two volumes are assumed to have the same dimensions
 * INPUT PARAMETERS:
 *   A: first 3D stack to be aligned
 *   B: second 3D stack to bealigned
 *   dimk: number of slices of the two stacks
 *   dimi: number of rows of each slice
 *   dimj: number of columns of each slice
 *   nk: initial offset between slices of second stack with respect to the first one (WARNINIG: assumed 0 by the current implementation)
 *   ni: initial offset between rows of second stack with respect to the first one
 *   nj: initial offset between columns of second stack with respect to the first one
 *   delayk: half range of NCC search around initial offset along third dimension
 *   delayi: half range of NCC search around initial offset  along first dimension
 *   delayj: half range of NCC search around initial offset  along second dimension
 *   side: side with respect to which alignment is computed (if it is NORTH_SOUTH A is over B, if it is WEST_EAST A is at left with respect to B)
 *   NCC_params: optional parameter; contains parameter values controlling the alignment strategy; default values are assumed if omitted; 
 *         
 * WARNING: 3D stacks A and B are supposed to be stored slice by slice; each slice is supposed to be stored row-wise
 * WARNING: a subtle relation is assumed to hold among dimi, dimj, dimk, ni, nj, nk, delayi, delayj, delayk, and controlling
 *          parameter wRangeThr; in practice the dimensions of the MIPS (depending on dimi, dimj, dimk, ni, nj, nk) have
 *          to be large enough with respect to delayi, delayj, delayk; moreover controlling parameter wRangeThr is supposed
 *          to be not greater than MAX(delayi,delayj,delayk)
 */

# endif
