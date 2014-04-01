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

#ifndef PAIRWISE_DISPLACEMENT_ALGORITHM_H
#define PAIRWISE_DISPLACEMENT_ALGORITHM_H

#include "Displacement.h"
#include "IOManager_defs.h"

class PDAlgo
{
	protected:

		int TYPE;				//type of algorithm

	public:

		PDAlgo(void){};
		~PDAlgo(void){};

		/*************************************************************************************************************
		* Abstract method that all derived classes must implement.
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
		virtual Displacement* execute(real_t *stk_A, uint32 A_dim_V, uint32 A_dim_H, uint32 A_dim_D,
									  real_t *stk_B, uint32 B_dim_V, uint32 B_dim_H, uint32 B_dim_D,
									  uint32 displ_max_V, uint32 displ_max_H, uint32 displ_max_D,
									  direction overlap_direction, uint32 overlap) throw (MyException) = 0;

		//static method which is responsible to instance and return the algorithm of the given type
		static PDAlgo* instanceAlgorithm(int _type);
};

#endif /* PAIRWISE_DISPLACEMENT_ALGORITHM_H */

