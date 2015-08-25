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

#ifndef S_CONFIG_H
#define S_CONFIG_H

#define S_VERBOSE 0
#define S_PI 3.14159265
#define S_STATIC_STRINGS_SIZE 3000

//*** DISPLACEMENTS COMPUTATION PHASE***
#define S_NCC_MODE 0
#define S_NCC_NAME "MIPNCC"
#define S_PHASE_CORRELATION_MODE 1
#define S_PC_NAME "PC"
#define S_DISPL_COMP_MODE S_NCC_MODE
#define S_MIP_NCC_DISPL_TYPE 1
#define S_PC_DISPL_TYPE 2
#define S_DISPL_SEARCH_RADIUS_DEF 25
#define S_SUBVOL_DIM_D_DEFAULT 200
#define S_OVERLAP_MIN 40
#define S_SUBVOL_DIM_D_MIN 50
# define HALVE_BY_MEAN 1
# define HALVE_BY_MAX  2

//*** FINDING ABSOLUTE TILE POSITIONS PHASE***
#define S_FATPM_SP_TREE 0				//spanning tree modality for finding absolute tile positions
#define S_FATPM_SP_TREE_NAME "MST"
#define S_FATPM_SCAN_V 1				//vertical scanning modality for finding absolute tile positions
#define S_FATPM_SCAN_V_NAME "SCANV"
#define S_FATPM_SCAN_H 2				//horizontal scanning modality for finding absolute tile positions
#define S_FATPM_SCAN_H_NAME "SCANH"
#define S_FATPM S_FATPM_SP_TREE			//selected modality for finding absolute tile positions (FATPM)
#define S_NCC_PEAK_MIN 0				//minimum value of NCC peak
#define S_NCC_PEAK_MAX 1				//maximum value of NCC peak
#define S_NCC_WIDTH_MIN 0				//minimum value of NCC width
#define S_NCC_WIDTH_MAX 30				//maximum value of NCC width
#define S_NCC_PEAK_WEIGHT 0.5			//weight of NCC peak feature in displacements reliability evaluation: must sum to 1 with S_NCC_WIDTH_WEIGHT
#define S_NCC_WIDTH_WEIGHT 0.5			//weight of NCC width feature in displacements reliability evaluation: must sum to 1 with S_NCC_PEAK_WEIGHT
#define S_UNRELIABLE_WEIGHT 1000		//value returned by displacements error evaluating function when error is above the given threshold

//*** MERGING PHASE***
#define S_NO_BLENDING			0		//id associated to a function which does not perform any blending
#define S_NO_BLENDING_NAME "NOBLEND"
#define S_SINUSOIDAL_BLENDING	1		//ID associated to sinusoidal blending function
#define S_SINUSOIDAL_BLENDING_NAME "SINBLEND"
#define S_SHOW_STACK_MARGIN 2
#define S_SHOW_STACK_MARGIN_NAME "STACKMARGIN"
#define S_ENHANCED_NO_BLENDING	3		//ID associated to inverse sinusoidal blending function enhancing margins
#define S_ENHANCED_NO_BLENDING_NAME "ENHNOBLEND"
#define S_SAVED_TIFF_BIT_DEPTH 8		//bit depth of saved images
#define S_MAX_MULTIRES 8				//in multiresolution mode, images will be downsampled up to 2^(S_MAX_MULTIRES)
#define S_MIN_SLICE_DIM 100

//*** RESTORING PHASE***
#define S_RESTORE_V_DIRECTION 1			//association of IDs to restoring directions
#define S_RESTORE_H_DIRECTION 2			// ''
#define S_RESTORE_Z_DIRECTION 3			// ''
#define S_DEFAULT_RESTORE_DIRECTION 1	//enabled restoring direction
//#define S_RESTORE_VZ					//if defined, restoring along Z will be performed contextually to vertical direction
#define S_MIN_PROF_VAL ((iom::real_t) 1e-5)	// minimum profile value (to be used when profile is null)
#define S_DESC_FILE_NAME "stk_descs.dsc"//file name of illumination maps to load/save from/to disc

#endif
