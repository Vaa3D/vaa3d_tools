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

#ifndef _STACK_RESTORER_H
#define _STACK_RESTORER_H

#include "StackedVolume.h"

class StackRestorer
{
	class vol_descr_t 
	{
		public:

			Stack* stk_p;								//pointer to Stack object
			real_t max_val;								//maximum value of current volume
			real_t mean_val;							//mean value of current volume
			int V_dim, H_dim, D_dim;					//VHD dimensions of current volume
			real_t *V_profile, *H_profile, *D_profile;	//lighting map for current volume
			real_t *V_MIP, *H_MIP, *D_MIP;				//maximum intensity projections of current volume along VHD axes
			bool is_subvol_descriptor;					//true if the current object contains descriptors of a subvolume
			bool is_finalized;							//true if descriptors have been successfully computed

			vol_descr_t();			
			~vol_descr_t();

			void init(Stack *new_stk_p, bool is_subvol_desc, int new_V_dim, int new_H_dim, int new_D_dim);
			void computeSubvolDescriptors(real_t *subvol)  throw (MyException);
			void computeStackDescriptors(vol_descr_t *subvol_desc, int D_subvols)  throw (MyException);
			bool isFinalized() {return is_finalized;}
			bool isSubvolDescriptor() {return is_subvol_descriptor;}
			void print();
	};

	private:
		
		StackedVolume *STK_ORG;						//pointer to <StackedVolume> object
		vol_descr_t ***SUBSTKS_DESCRIPTORS;				//3D array of <vol_descr_t> relative to substack descriptors
		int SD_DIM_i, SD_DIM_j, SD_DIM_k;				//dimensions of SUBSTKS_DESCRIPTORS
		vol_descr_t **STKS_DESCRIPTORS;					//2D array of <vol_descr_t> relative to entire Stack descriptors
		int N_ROWS, N_COLS;								//dimensions of STKS_DESCRIPTORS						

		StackRestorer(void){};

	public:

		StackRestorer(StackedVolume* stk_org);
		StackRestorer(StackedVolume* stk_org, int D_subvols);
		StackRestorer(StackedVolume* stk_org, char* file_path);
		~StackRestorer(void);

		void computeSubvolDescriptors(real_t* data, Stack* stk_p, int subvol_idx, int subvol_D_dim)  throw (MyException);
		void computeStackDescriptors(Stack* stk_p)  throw (MyException);
		void finalizeAllDescriptors();

		void repairSlice(real_t* data, int slice_idx, Stack* stk_p, int direction)  throw (MyException);
		void repairStack(real_t* data, Stack* stk_p, int direction)  throw (MyException);

		void printSubvolDescriptors(Stack* stk_p, int subvol_idx);
		void printVolDescriptors(Stack* stk_p);
		void save(char* file_path) throw (MyException);
		void load(char* file_path)  throw (MyException);
};

#endif
