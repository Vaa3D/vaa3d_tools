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

#ifndef _TILE_PLACEMENT_ALGORITHM_MINIMUM_SPANNING_TREE_H
#define _TILE_PLACEMENT_ALGORITHM_MINIMUM_SPANNING_TREE_H

#include "TPAlgo.h"

class TPAlgoMST : public TPAlgo
{
	private:

		TPAlgoMST(void){};			//default constructor is inhibited

	protected:

		//int TYPE;					//INHERITED from TPAlgo
		//StackedVolume* volume;	//INHERITED from TPAlgo

	public:

		TPAlgoMST(volumemanager::VirtualVolume * _volume);
		~TPAlgoMST(void){};

		/*************************************************************************************************************
		* Finds the optimal tile placement on the <volume> object member via Minimum Spanning Tree.
		* Stacks matrix is considered as a graph  where  displacements are edges a nd stacks are nodes. The inverse of
		* displacements reliability factors are  edge weights,  so that a totally unreliable displacement has a weight
		* 1/0.0 = +inf and a totally reliable displacement has a weight 1/1.0 = 1. Thus, weights will be in [1, +inf].
		* After computing the MST, the absolute tile positions are obtained from a stitchable source stack by means of
		* navigating the MST and using the selected displacements.
		* PROs: very general method; it ignores bad displacements since they have +inf weight.
		* CONs: the best path between two adjacent stacks can pass through many stacks even if the rejected displacem-
		*       ent is quite reliable, with a very little reliability gain.  This implies possible bad absolute  posi-
		*       tions estimations when the path is too long.
		**************************************************************************************************************/
		void execute() throw (MyException);
};

#endif /* _TILE_PLACEMENT_ALGORITHM_MINIMUM_SPANNING_TREE_H */
