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

#include "PDAlgoMIPNCC.h"
#include "CrossMIPs.h"
#include "DisplacementMIPNCC.h"

/*************************************************************************************************************
* Implements abstract method declared in superclass
* <stk_[]>				: image stacks stored in row-wise in a monodimensional array
* <[]_dim[]>			: VHD dimensions of <stk[]>
* <displ_max_[]>		: maximum displacements along VHD of <stk_B> respect to <stk_A> taking the given over-
*						  lap as reference. These parameters, together with <overlap>, can be used to identify  
*						  the region of interest where the correspondence between the given  stacks  has to be 
*						  found. When used, these parameters have to be tuned with respect to the precision of
*						  the motorized stages.
* <overlap_direction>	: direction of overlapping (see <direction> type definition for further details)
* <overlap>				: expected overlap between the given stacks along the given direction.  This value can
*						  be used to determine the region of interest where the overlapping occurs.
**************************************************************************************************************/
Displacement* PDAlgoMIPNCC::execute(real_t *stk_A, uint32 A_dim_V, uint32 A_dim_H, uint32 A_dim_D,
	real_t *stk_B, uint32 B_dim_V, uint32 B_dim_H, uint32 B_dim_D,
	uint32 displ_max_V, uint32 displ_max_H, uint32 displ_max_D,
	direction overlap_direction, uint32 overlap) throw (MyException)
{
	#if S_VERBOSE>3
	printf("\t\t\t\tin PDAlgoMIPNCC::execute(..., A_dim_V = %d, A_dim_H = %d, A_dim_D = %d, B_dim_V = %d, B_dim_H = %d, B_dim_D = %d, displ_max_V = %d, displ_max_H = %d, displ_max_D = %d, overlap_direction = %d, overlap = %d)\n",
		    A_dim_V, A_dim_H, A_dim_D, B_dim_V, B_dim_H, B_dim_D, displ_max_V, displ_max_H, displ_max_D, overlap_direction, overlap);
	#endif

	//some checks
	if(A_dim_V != B_dim_V || A_dim_H != B_dim_H || A_dim_D != B_dim_D)
		throw MyException("in PDAlgoMIPNCC::execute(...): stacks A and B don't have the same dimensions");
	if(overlap_direction != dir_horizontal && overlap_direction != dir_vertical)
		throw MyException("in PDAlgoMIPNCC::execute(...): unsupported overlapping direction");

	// Alessandro - 31/05/2013 - parameters MUST be passed (and controlled) by the caller
	NCC_parms_t params;
	params.enhance      = false;
	params.maxIter		= 2;
	params.maxThr       = 0.10f;
	params.UNR_NCC      = S_NCC_PEAK_MIN;
	params.wRangeThr    = MIN(std::min(std::min(displ_max_V, displ_max_H), displ_max_D), S_NCC_WIDTH_MAX);
	params.INF_W        = params.wRangeThr + 1;
	params.widthThr     = 0.75f;
	params.INV_COORD    = 0;

	NCC_descr_t* descr = norm_cross_corr_mips(stk_A, stk_B, A_dim_D, A_dim_V, A_dim_H, 0, overlap_direction == dir_vertical ? A_dim_V - overlap : 0, 
											  overlap_direction == dir_horizontal ? A_dim_H - overlap: 0, displ_max_D, displ_max_V, displ_max_H, overlap_direction, &params);

	// Alessandro - 31/05/2013 - storing parameters
	DisplacementMIPNCC *displ = new DisplacementMIPNCC(*descr);
	displ->delays[0]  = displ_max_V;
	displ->delays[1]  = displ_max_H;
	displ->delays[2]  = displ_max_D;
	displ->wRangeThr = params.wRangeThr;
	displ->invWidth  = params.INF_W;

	delete descr;

	return (Displacement*)displ;	
}
