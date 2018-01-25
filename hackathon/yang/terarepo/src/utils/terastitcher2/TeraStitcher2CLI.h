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
* 2017-02-10.  Giulio.     @ADDED options to specify the blending algorithm to be used for layers 
* 2016-09-04.  Giulio.     @ADDED the options for setting the configuration of the LibTIFF library 
*/


#ifndef _TERASTITCHER_COMMAND_LINE_INTERFACE_H
#define _TERASTITCHER_COMMAND_LINE_INTERFACE_H

#include <string>
//#include "MyException.h"
#include "S_config.h"
#include "IM_config.h"
//#include "IOManager_defs.h"

using namespace std;

class TeraStitcher2CLI
{
	public:

		//STITCHING pipeline steps switches
		//bool test;								//test mode: the middle-volume slice is produced and saved locally
		bool stitch;							//entire stitching pipeline is enabled
		bool import;							//volume import step
		bool computedisplacements;				//pairwise displacements computation step
		bool projdisplacements;					//displacements projection step
		bool thresholddisplacements;			//displacements thresholding step
		bool placetiles;						//globally optimal tiles placement step
		bool placelayers;						//globally optimal layers placement step
		bool mergetiles;						//tiles merging step
		//bool dumpMData;							//dump mdata.bin
		bool ignoreUnequalStacksDepth;			//ignore unequal stacks depth (i.e., number of slices).

		//STITCHING pipeline parameters
		string volume_load_path;				//directory path where the volume is stored used during the volume import step
		string volume_save_path;				//directory path where to save the stitched volume
		string projfile_load_path;				//file path of the project XML file to be loaded
		string projfile_save_path;				//file path of the project XML file to be saved
		//iim::ref_sys reference_system;				//reference system used by the acquired volume  
		//float VXL_1, VXL_2, VXL_3;				//voxel dimensions (in microns) along first, second and third axes
		int pd_algo;							//pairwise displacements computation algorithm identifier
		//int start_stack_row;					//first row of stacks matrix to be processed
		//int end_stack_row;						//last row of stacks matrix be processed
		//int start_stack_col;					//first column of stacks matrix to be processed
		//int end_stack_col;						//last column of stacks matrix to be processed
		int start_layer;					    //first layer of thevolume to be processed
		int end_layer;						    //last layer of the volume to be processed
		//int overlap_V, overlap_H;				//expected overlaps between adjacent stacks along V and H directions
		int search_radius_V;					//identifies the region of interest along V used to search for pairwise displacements
		int search_radius_H;					//identifies the region of interest along H used to search for pairwise displacements
		int search_radius_D;					//identifies the region of interest along D used to search for pairwise displacements
		int substk_dim_V;						//dimension V per substack partition used in the pairwise displacements computation step
		int substk_dim_H;						//dimension H per substack partition used in the pairwise displacements computation step
		float reliability_threshold;			//threshold used to select most reliable displacements and discard the others
		int tp_algo;							//tiles placement algorithm identifier
		int slice_height, slice_width;			//desired dimensions of tiles slices after merging
		bool resolutions[S_MAX_MULTIRES+1];		//resolutions flags
		//bool exclude_nonstitchables;			//if enabled, non-stitchable stacks will be excluded from the final stitched volume
		int D0, D1;								//portion of volume along D axis to be processed
		int tm_blending;						//tiles merging blending type
		int lm_blending;						//layers merging blending type
		//bool enable_restore;					//enable tiles restoring
		//int restoring_direction;				//restoring direction
		bool save_execution_times;				//enable execution times saving
		string execution_times_filename;		//name of file where to store execution times
		bool show_progress_bar;					//enables/disables progress bar with estimated time remaining
		string img_format;						//saved image format
		int img_depth;							//saved image depth

		//string image_format;				    //Format of input images (2D/Tiff3D)
		int slice_depth;						//desired dimension of blocks after merging
		float norm_fact_D;						//factor to rescale D coordinates; default is 1.0 which means that coordinate D step is 1.0 um
		float cut_depth;						//depth in um to be used to decrease the effect of the cut in alignment and blending between layers; default is 10 um

		// parameters to configure LibTIFF
		bool libtiff_uncompressed;
		int libtiff_rowsPerStrip;

		//constructor - deconstructor
		TeraStitcher2CLI(void);					//set default params
		~TeraStitcher2CLI(void){};

		//reads options and parameters from command line
		void readParams(int argc, char** argv) throw (iim::IOException);

		//checks parameters correctness
		void checkParams() throw (iim::IOException);

		//returns help text
		string getHelpText();

		//print all arguments
		void print();
};

#endif /* _TERASTITCHER_COMMAND_LINE_INTERFACE_H */


