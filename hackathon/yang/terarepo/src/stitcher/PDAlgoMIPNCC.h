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

#ifndef PD_ALGO_MIPNCC_H
#define PD_ALGO_MIPNCC_H

#include "S_config.h"
#include "PDAlgo.h"

class PDAlgoMIPNCC : PDAlgo
{
	protected:

		//int TYPE;				//type of algorithm (INHERITED)

	public:

		PDAlgoMIPNCC(void){TYPE = S_NCC_MODE;};
		~PDAlgoMIPNCC(void){};

		//implements abstract method declared in superclass
		Displacement* execute(iom::real_t *stk_A, iom::uint32 A_dim_V, iom::uint32 A_dim_H, iom::uint32 A_dim_D,
			iom::real_t *stk_B, iom::uint32 B_dim_V, iom::uint32 B_dim_H, iom::uint32 B_dim_D,
			iom::uint32 displ_max_V, iom::uint32 displ_max_H, iom::uint32 displ_max_D,
			direction overlap_direction, iom::uint32 overlap) throw (iom::exception);
};

#endif /* PD_ALGO_MIPNCC_H */

