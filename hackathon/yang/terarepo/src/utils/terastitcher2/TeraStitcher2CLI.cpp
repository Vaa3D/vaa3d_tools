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
* 2017-02-10.  Giulio.     @CHANGED the name and the order of some options 
* 2017-02-10.  Giulio.     @ADDED options to specify the blending algorithm to be used for layers 
* 2016-09-04.  Giulio.     @ADDED the options for setting the configuration of the LibTIFF library 
*/


#include "TeraStitcher2CLI.h"
#include "volumemanager.config.h"
//#include "S_config.h"
#include "IOPluginAPI.h"
#include "GUI_config.h"
#include <CmdLine.h>
#include <sstream>
#include "config.h"


using namespace iim;

TeraStitcher2CLI::TeraStitcher2CLI(void)
{
	//initialization should be done for all parameters, but we use TCLAP default values for arguments (see readParams)
	this->pd_algo = S_NCC_MODE;
	this->tp_algo = S_FATPM_SP_TREE;
	this->tm_blending = S_SINUSOIDAL_BLENDING;
	this->lm_blending = S_SINUSOIDAL_BLENDING;
}

//reads options and parameters from command line
void TeraStitcher2CLI::readParams(int argc, char** argv) throw (IOException)
{
	//command line object definition
	TCLAP::CmdLine cmd(getHelpText(), '=', terastitcher::terastitcher2_version);

	//argument objects definitions
	TCLAP::SwitchArg p_stitch("S","stitch","Stitches a volume by executing the entire pipeline (steps 1-6).",false);
	TCLAP::SwitchArg p_import("1","import","Step 1: imports a volume to TeraStitcher XML project file.", false);
	//TCLAP::ValueArg<std::string> p_image_format("f","imfmt","Format of input images (2D/Tiff3D).",false,"2D","string");
	TCLAP::SwitchArg p_computedisplacements("2","displcompute","Step 2: computes pairwise stacks displacements.", false);
	TCLAP::SwitchArg p_projdisplacements("7","displproj","Step 3b: projects existing displacements along Z axis for each stack by selecting the most reliable one.", false);
	TCLAP::SwitchArg p_thresholdisplacements("3","displthres","Step 3: thresholds displacements using the given reliability threshold.", false);
	TCLAP::SwitchArg p_placetiles("4","placetiles","Step 4: places tiles in the image space using a globally optimal tiles placement algorithm.", false);
	TCLAP::SwitchArg p_placelayers("5","placelayers","Step 5: places layers in the image space and determines the final image size.", false);
	TCLAP::SwitchArg p_merge("6","merge","Step 6: merges tiles at different resolutions.", false);
	//TCLAP::SwitchArg p_test("t","test","Stitches the middle slice of the whole volume and saves it locally. Stage coordinates will be used, so this can be used to test their precision as well as the selected reference system.",false);
	//TCLAP::SwitchArg p_dump("d","dump","Print the entire content of metadata file mdata.bin",false);
	TCLAP::ValueArg<std::string> p_volume_load_path("","volin","Directory path where the volume is stored.",false,"null","string");
	TCLAP::ValueArg<std::string> p_volume_save_path("","volout","Directory path where to save the stitched volume.",false,"null","string");
	TCLAP::ValueArg<std::string> p_project_load_path("","projin","File path of the XML project file to be loaded.",false,"null","string");
	TCLAP::ValueArg<std::string> p_project_save_path("","projout","File path of the XML project file to be saved.",false,"","string");
	//TCLAP::ValueArg<int> p_refsys_1("","ref1","First axis of the used reference system.                              1  = Vertical,      2 = Horizontal,      3 = Depth,                              -1 = inv_Vertical, -2 = inv_Horizontal, -3 = inv_Depth.",false,CLI_DEF_REF1,"integer");
	//TCLAP::ValueArg<int> p_refsys_2("","ref2","Second axis of the used reference system.                              1  = Vertical,      2 = Horizontal,      3 = Depth,                              -1 = inv_Vertical, -2 = inv_Horizontal, -3 = inv_Depth.",false,CLI_DEF_REF2,"integer");
	//TCLAP::ValueArg<int> p_refsys_3("","ref3","Third axis of the used reference system.                              1  = Vertical,      2 = Horizontal,      3 = Depth,                              -1 = inv_Vertical, -2 = inv_Horizontal, -3 = inv_Depth.",false,CLI_DEF_REF3,"integer");
	//TCLAP::ValueArg<float> p_vxl_1("","vxl1","Voxel dimension along first axis (in microns).",false,CLI_DEF_VXL1,"real");
	//TCLAP::ValueArg<float> p_vxl_2("","vxl2","Voxel dimension along second axis (in microns).",false,CLI_DEF_VXL2,"real");
	//TCLAP::ValueArg<float> p_vxl_3("","vxl3","Voxel dimension along third axis (in microns).",false,CLI_DEF_VXL3,"real");
	TCLAP::MultiArg<std::string> p_algo("","algorithm","Forces the use of the given algorithm.",false,"string");
	//TCLAP::ValueArg<int> p_start_stack_row("","R0","First stacks row to be processed.",false,-1,"integer");
	//TCLAP::ValueArg<int> p_end_stack_row("","R1","Last stacks row to be processed.",false,-1,"integer");
	//TCLAP::ValueArg<int> p_start_stack_col("","C0","First stacks column to be processed.",false,-1,"integer");
	//TCLAP::ValueArg<int> p_end_stack_col("","C1","Last stacks column to be processed.",false,-1,"integer");
	TCLAP::ValueArg<int> p_start_layer("","L0","First layer to be processed.",false,-1,"integer");
	TCLAP::ValueArg<int> p_end_layer("","L1","Last layer to be processed.",false,-1,"integer");
	TCLAP::ValueArg<int> p_D0("","D0","First slice to be processed (index along D).",false,-1,"integer");
	TCLAP::ValueArg<int> p_D1("","D1","Last slice to be processed (index along D).",false,-1,"integer");
	//TCLAP::ValueArg<int> p_overlap_V("","oV","Overlap (in pixels) between two adjacent tiles along V.",false,-1,"integer");
	//TCLAP::ValueArg<int> p_overlap_H("","oH","Overlap (in pixels) between two adjacent tiles along H.",false,-1,"integer");
	TCLAP::ValueArg<int> p_search_radius_V("","sV","Displacements search radius along V (in pixels).",false,S_DISPL_SEARCH_RADIUS_DEF,"integer");
	TCLAP::ValueArg<int> p_search_radius_H("","sH","Displacements search radius along H (in pixels).",false,S_DISPL_SEARCH_RADIUS_DEF,"integer");
	TCLAP::ValueArg<int> p_search_radius_D("","sD","Displacements search radius along D (in pixels).",false,S_DISPL_SEARCH_RADIUS_DEF,"integer");
	TCLAP::ValueArg<int> p_substk_dim_V("","substkdimV","Dimension per subvolume partition used in the pairwise displacements computation step.",false,S_SUBVOL_DIM_D_DEFAULT,"integer");
	TCLAP::ValueArg<int> p_substk_dim_H("","substkdimH","Dimension per subvolume partition used in the pairwise displacements computation step.",false,S_SUBVOL_DIM_D_DEFAULT,"integer");
	TCLAP::ValueArg<float> p_reliability_threshold("","threshold","Reliability threshold. Values are in [0.0, 1.0] where 0 = unreliable, 1.0 = totally reliable.",false,CLI_DEF_REL_THRESHOLD,"real");
	TCLAP::ValueArg<int> p_slice_height("","sliceheight","Desired slice height of merged tiles (in pixels).",false,-1,"integer");
	TCLAP::ValueArg<int> p_slice_width("","slicewidth","Desired slice width of merged tiles (in pixels).",false,-1,"integer");
	TCLAP::ValueArg<int> p_slice_depth("","slicedepth","Desired block depth of merged tiles (in pixels).",false,-1,"integer");
	TCLAP::ValueArg<std::string> p_resolutions("","resolutions","Resolutions to be produced. Possible values are [[i]...]             where i = 0,..,5 and 2^i is the subsampling factor.",false,"0","string");
	//TCLAP::SwitchArg p_exclude_nonstitchables("","stitchablesonly","Excludes non-stitchables stacks from the stitched volume.", false);
	//TCLAP::SwitchArg p_enable_restore("","restoreSPIM","Enables SPIM artifacts removal (zebrated pattern).", false);
	//TCLAP::ValueArg<int> p_restoring_direction("","restoredir","SPIM zebrated pattern direction. 1 = Vertical, 2 = Horizontal, 3 = Depth.",false,-1,"integer");
	TCLAP::SwitchArg p_save_execution_times("","exectimes","Saves execution times into a file located in the volume directory.", false);
	TCLAP::ValueArg<std::string> p_execution_times_file("","exectimesfile","Name of file where to write execution times. ",false,"null","string");
	TCLAP::SwitchArg p_hide_progress_bar("","noprogressbar","Disables progress bar and estimated time remaining", false);
	TCLAP::ValueArg<std::string> p_saved_img_format("","imformat","Format of saved images (\"tif\" is default, other: Tiff3D / Vaa3DRaw). ",false,"tif","string");
	TCLAP::ValueArg<int> p_saved_img_depth("","imdepth","Bitdepth of saved images (\"8\" is default). ",false,8,"integer");
	//TCLAP::SwitchArg p_ignoreUnequalStacksDepth("","ignore_unequal_stacks_depth","If enabled, differences among stacks in terms of number of slices are ignored. The number of slices of the smallest stack will be assumed as the dimension along D for all the stacks",false);
	TCLAP::ValueArg<std::string> p_img_regex("","imregex","A regular expression to be used to match image filenames when the volume is imported (by default, all the images with supported file extension are found)",false,"","string");
	TCLAP::ValueArg<std::string> p_img_channel_select("","imchannel","The channel(s) to be selected when the images are loaded. Allowed values are {\"all\",\"R\",\"G\",\"B\"}. Default is \"all\" (color images will be converted to grayscale).",false,"all","string");
	TCLAP::ValueArg<float> p_norm_fact_D("","normD","Normalizing factor along D (in microns).",false,CLI_DEF_NORM3,"real");
	TCLAP::ValueArg<float> p_cut_depth("","depthcut","Depth of cut (in microns).",false,CLI_DEF_CUT_DEPTH,"real");

	TCLAP::SwitchArg p_libtiff_uncompressed("","libtiff_uncompress","Configure libtiff library to not compress output files (default: compression enabled).", false);
	TCLAP::ValueArg<int> p_libtiff_rowsperstrip("","libtiff_rowsperstrip","Configure libtiff library to pack n rows per strip when compression is enabled (default: 1 row per strip).",false,1,"integer");

	//argument objects must be inserted using LIFO policy (last inserted, first shown)
	cmd.add(p_libtiff_rowsperstrip);
	cmd.add(p_libtiff_uncompressed);

	cmd.add(p_cut_depth);
	cmd.add(p_norm_fact_D);
	cmd.add(p_img_channel_select);
	cmd.add(p_img_regex);
	//cmd.add(p_ignoreUnequalStacksDepth);
	cmd.add(p_hide_progress_bar);
	cmd.add(p_execution_times_file);
	cmd.add(p_save_execution_times);
	cmd.add(p_saved_img_depth);
	cmd.add(p_saved_img_format);
	//cmd.add(p_restoring_direction);
	//cmd.add(p_enable_restore);
	//cmd.add(p_exclude_nonstitchables);
	cmd.add(p_resolutions);
	cmd.add(p_slice_depth);
	cmd.add(p_slice_width);
	cmd.add(p_slice_height);
	cmd.add(p_reliability_threshold);
	cmd.add(p_substk_dim_V);
	cmd.add(p_substk_dim_H);
	cmd.add(p_search_radius_D);
	cmd.add(p_search_radius_H);
	cmd.add(p_search_radius_V);
	//cmd.add(p_overlap_H);
	//cmd.add(p_overlap_V);
	cmd.add(p_D1);
	cmd.add(p_D0);
	cmd.add(p_end_layer);
	cmd.add(p_start_layer);
	//cmd.add(p_end_stack_col);
	//cmd.add(p_start_stack_col);
	//cmd.add(p_end_stack_row);
	//cmd.add(p_start_stack_row);
	cmd.add(p_algo);
	//cmd.add(p_vxl_3);
	//cmd.add(p_vxl_2);
	//cmd.add(p_vxl_1);
	//cmd.add(p_refsys_3);
	//cmd.add(p_refsys_2);
	//cmd.add(p_refsys_1);
	cmd.add(p_project_save_path);
	cmd.add(p_project_load_path);
	cmd.add(p_volume_save_path);
	cmd.add(p_volume_load_path);
	//cmd.add(p_dump);
	cmd.add(p_stitch);
	cmd.add(p_merge);
	cmd.add(p_placelayers);
	cmd.add(p_placetiles);
	cmd.add(p_thresholdisplacements);
	cmd.add(p_projdisplacements);
	cmd.add(p_computedisplacements);
	//cmd.add(p_image_format);
	cmd.add(p_import);
	//cmd.add(p_test);

	// Parse the argv array and catch <TCLAP> exceptions, which are translated into <IOException> exceptions
	char errMsg[S_STATIC_STRINGS_SIZE];
	try{ cmd.parse( argc, argv ); } 
	catch (TCLAP::ArgException &e)
	{ 
		sprintf(errMsg, "%s for arg %s\n", e.error().c_str(), e.argId().c_str());
		throw IOException(errMsg);
	}

	/* Checking mandatory parameters for each step */

	//TEST
	//if(p_test.isSet() && !(
	//	p_volume_load_path.isSet() && 
	//	p_refsys_1.isSet() && p_refsys_2.isSet() && p_refsys_3.isSet() &&
	//	p_vxl_1.isSet() && p_vxl_2.isSet() && p_vxl_3.isSet() ) && !p_project_load_path.isSet())
	//{
	//	sprintf(errMsg, "One or more required arguments missing for --%s!\nUSAGE is:\n--%s\n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s>\n\nor\n\n--%s\n\t--%s%c<%s>", 
	//		p_test.getName().c_str(), p_test.getName().c_str(),
	//		p_volume_load_path.getName().c_str(), cmd.getDelimiter(), "string",
	//		p_refsys_1.getName().c_str(), cmd.getDelimiter(), "integer",
	//		p_refsys_2.getName().c_str(), cmd.getDelimiter(), "integer",
	//		p_refsys_3.getName().c_str(), cmd.getDelimiter(), "integer",
	//		p_vxl_1.getName().c_str(), cmd.getDelimiter(), "real",
	//		p_vxl_2.getName().c_str(), cmd.getDelimiter(), "real",
	//		p_vxl_3.getName().c_str(), cmd.getDelimiter(), "real",
	//		p_test.getName().c_str(),
	//		p_project_load_path.getName().c_str(), cmd.getDelimiter(), "string");
	//	throw IOException(errMsg);
	//}

	//STEP 1
	if(p_import.isSet() && !(
		p_volume_load_path.isSet() && p_project_save_path.isSet() //&& 
		//p_refsys_1.isSet() && p_refsys_2.isSet() && p_refsys_3.isSet() &&
		//p_vxl_1.isSet() && p_vxl_2.isSet() && p_vxl_3.isSet() 
		))
	{
		sprintf(errMsg, "One or more required arguments missing for --%s!\nUSAGE is:\n--%s\n\t--%s%c<%s> \n\t--%s%c<%s>", 
			p_import.getName().c_str(), p_import.getName().c_str(),
			p_volume_load_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_project_save_path.getName().c_str(), cmd.getDelimiter(), "string"//,
			//p_refsys_1.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_refsys_2.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_refsys_3.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_vxl_1.getName().c_str(), cmd.getDelimiter(), "real",
			//p_vxl_2.getName().c_str(), cmd.getDelimiter(), "real",
			//p_vxl_3.getName().c_str(), cmd.getDelimiter(), "real"
			);
		throw IOException(errMsg);
	}

	//STEP 2
	if(p_computedisplacements.isSet() &&!( 
		(p_volume_load_path.isSet() && p_project_save_path.isSet()) // && p_refsys_1.isSet() && p_refsys_2.isSet() && p_refsys_3.isSet() && p_vxl_1.isSet() && p_vxl_2.isSet() && p_vxl_3.isSet() 
		|| 	
		(p_project_load_path.isSet() && p_project_save_path.isSet())))
	{
		sprintf(errMsg, "One or more required arguments missing for --%s!\n\nUSAGE 1 is:\n--%s\n\t--%s%c<%s> \n\t--%s%c<%s> \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>]", 
			p_computedisplacements.getName().c_str(), p_computedisplacements.getName().c_str(),
			p_volume_load_path.getName().c_str(), cmd.getDelimiter(), "string",
			//p_refsys_1.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_refsys_2.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_refsys_3.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_vxl_1.getName().c_str(), cmd.getDelimiter(), "real",
			//p_vxl_2.getName().c_str(), cmd.getDelimiter(), "real",
			//p_vxl_3.getName().c_str(), cmd.getDelimiter(), "real",
			p_project_save_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_algo.getName().c_str(), cmd.getDelimiter(), "string",
			//p_start_stack_row.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_end_stack_row.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_start_stack_col.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_end_stack_col.getName().c_str(), cmd.getDelimiter(), "integer",
			p_start_layer.getName().c_str(), cmd.getDelimiter(), "integer",
			p_end_layer.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_overlap_V.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_overlap_H.getName().c_str(), cmd.getDelimiter(), "integer",
			p_search_radius_V.getName().c_str(), cmd.getDelimiter(), "integer",
			p_search_radius_H.getName().c_str(), cmd.getDelimiter(), "integer",
			p_search_radius_D.getName().c_str(), cmd.getDelimiter(), "integer",
			p_substk_dim_V.getName().c_str(), cmd.getDelimiter(), "integer",
			p_substk_dim_H.getName().c_str(), cmd.getDelimiter(), "integer"//,
			//p_enable_restore.getName().c_str(),
			//p_restoring_direction.getName().c_str(), cmd.getDelimiter(), "integer"
			);
			sprintf(errMsg, "%s\n\nUSAGE 2 is:\n--%s\n\t--%s%c<%s> \n\t--%s%c<%s> \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>]", 
			errMsg,
			p_computedisplacements.getName().c_str(),
			p_project_load_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_project_save_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_algo.getName().c_str(), cmd.getDelimiter(), "string",
			//p_start_stack_row.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_end_stack_row.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_start_stack_col.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_end_stack_col.getName().c_str(), cmd.getDelimiter(), "integer",
			p_start_layer.getName().c_str(), cmd.getDelimiter(), "integer",
			p_end_layer.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_overlap_V.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_overlap_H.getName().c_str(), cmd.getDelimiter(), "integer",
			p_search_radius_V.getName().c_str(), cmd.getDelimiter(), "integer",
			p_search_radius_H.getName().c_str(), cmd.getDelimiter(), "integer",
			p_search_radius_D.getName().c_str(), cmd.getDelimiter(), "integer",
			p_substk_dim_V.getName().c_str(), cmd.getDelimiter(), "integer",
			p_substk_dim_H.getName().c_str(), cmd.getDelimiter(), "integer"//,
			//p_enable_restore.getName().c_str(),
			//p_restoring_direction.getName().c_str(), cmd.getDelimiter(), "integer"
			);
		throw IOException(errMsg);
	}

	//STEP 3
	if(p_thresholdisplacements.isSet() &&!(p_project_load_path.isSet() && p_project_save_path.isSet() && p_reliability_threshold.isSet()))
	{
		sprintf(errMsg, "One or more required arguments missing for --%s!\n\nUSAGE is:\n--%s\n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s>",
			    p_thresholdisplacements.getName().c_str(), p_thresholdisplacements.getName().c_str(),
				p_project_load_path.getName().c_str(), cmd.getDelimiter(), "string",
				p_project_save_path.getName().c_str(), cmd.getDelimiter(), "string",
				p_reliability_threshold.getName().c_str(), cmd.getDelimiter(), "real");
		throw IOException(errMsg);
	}

	//STEP 4
	if((p_projdisplacements.isSet() || p_placetiles.isSet()) &&!(p_project_load_path.isSet() && p_project_save_path.isSet()))
	{
		sprintf(errMsg, "One or more required arguments missing for --%s!\n\nUSAGE is:\n--%s\n\t--%s%c<%s> \n\t--%s%c<%s>",
			    //p_projdisplacements.getName().c_str(), p_projdisplacements.getName().c_str(),
			    p_placetiles.getName().c_str(), p_placetiles.getName().c_str(),
				p_project_load_path.getName().c_str(), cmd.getDelimiter(), "string",
				p_project_save_path.getName().c_str(), cmd.getDelimiter(), "string");
		throw IOException(errMsg);
	}

	//STEP 5
	if(p_placelayers.isSet() && !(p_project_load_path.isSet() && p_project_save_path.isSet()))
	{
		sprintf(errMsg, "One or more required arguments missing for --%s!\n\nUSAGE is:\n--%s\n\t--%s%c<%s> \n\t--%s%c<%s> \n\t[--%s%c<%s>]",
			p_placelayers.getName().c_str(), p_placelayers.getName().c_str(),
			p_project_load_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_project_save_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_algo.getName().c_str(), cmd.getDelimiter(), "string");
		throw IOException(errMsg);
	}

	//STEP 6
	if(p_merge.isSet() &&!(p_project_load_path.isSet() && p_volume_save_path.isSet()))
	{
		sprintf(errMsg, "One or more required arguments missing for --%s!\n\nUSAGE is:\n--%s\n\t--%s%c<%s> \n\t--%s%c<%s> \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>]\n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>]",
			p_merge.getName().c_str(), p_merge.getName().c_str(),
			p_project_load_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_volume_save_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_slice_height.getName().c_str(), cmd.getDelimiter(), "integer",
			p_slice_width.getName().c_str(), cmd.getDelimiter(), "integer",
			p_resolutions.getName().c_str(), cmd.getDelimiter(), "string",
			//p_exclude_nonstitchables.getName().c_str(),
			//p_start_stack_row.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_end_stack_row.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_start_stack_col.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_end_stack_col.getName().c_str(), cmd.getDelimiter(), "integer",
			p_start_layer.getName().c_str(), cmd.getDelimiter(), "integer",
			p_end_layer.getName().c_str(), cmd.getDelimiter(), "integer",
			p_D0.getName().c_str(), cmd.getDelimiter(), "integer",
			p_D1.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_enable_restore.getName().c_str(),
			//p_restoring_direction.getName().c_str(), cmd.getDelimiter(), "integer",
			p_saved_img_depth.getName().c_str(), cmd.getDelimiter(), "integer",
			p_saved_img_format.getName().c_str(), cmd.getDelimiter(), "string");
		throw IOException(errMsg);
	}

	//STEP 1-6 (--stitch option)
	if(p_stitch.isSet() &&!(
		p_volume_load_path.isSet() && //p_project_save_path.isSet() && 
		//p_refsys_1.isSet() && p_refsys_2.isSet() && p_refsys_3.isSet() &&
		//p_vxl_1.isSet() && p_vxl_2.isSet() && p_vxl_3.isSet() &&
		//p_reliability_threshold.isSet() && 
		p_volume_save_path.isSet()))
	{
		sprintf(errMsg, "One or more required arguments missing for --%s!\nUSAGE is:\n--%s \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t[--%s%c<%s>]... (accepted multiple times) \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>]\n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>]", 
			p_stitch.getName().c_str(), p_stitch.getName().c_str(),
			p_volume_load_path.getName().c_str(), cmd.getDelimiter(), "string",
			//p_refsys_1.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_refsys_2.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_refsys_3.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_vxl_1.getName().c_str(), cmd.getDelimiter(), "real",
			//p_vxl_2.getName().c_str(), cmd.getDelimiter(), "real",
			//p_vxl_3.getName().c_str(), cmd.getDelimiter(), "real",
			p_volume_save_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_project_save_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_reliability_threshold.getName().c_str(), cmd.getDelimiter(), "real",
			p_algo.getName().c_str(), cmd.getDelimiter(), "string",
			//p_start_stack_row.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_end_stack_row.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_start_stack_col.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_end_stack_col.getName().c_str(), cmd.getDelimiter(), "integer",
			p_start_layer.getName().c_str(), cmd.getDelimiter(), "integer",
			p_end_layer.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_overlap_V.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_overlap_H.getName().c_str(), cmd.getDelimiter(), "integer",
			p_search_radius_V.getName().c_str(), cmd.getDelimiter(), "integer",
			p_search_radius_H.getName().c_str(), cmd.getDelimiter(), "integer",
			p_search_radius_D.getName().c_str(), cmd.getDelimiter(), "integer",
			p_substk_dim_V.getName().c_str(), cmd.getDelimiter(), "integer",
			p_substk_dim_H.getName().c_str(), cmd.getDelimiter(), "integer",
			p_slice_height.getName().c_str(), cmd.getDelimiter(), "integer",
			p_slice_width.getName().c_str(), cmd.getDelimiter(), "integer",
			p_resolutions.getName().c_str(), cmd.getDelimiter(), "string",
			//p_exclude_nonstitchables.getName().c_str(),
			p_D0.getName().c_str(), cmd.getDelimiter(), "integer",
			p_D1.getName().c_str(), cmd.getDelimiter(), "integer",
			//p_enable_restore.getName().c_str(),
			//p_restoring_direction.getName().c_str(), cmd.getDelimiter(), "integer",
			p_saved_img_depth.getName().c_str(), cmd.getDelimiter(), "integer",
			p_saved_img_format.getName().c_str(), cmd.getDelimiter(), "string");
		throw IOException(errMsg);
	}
		
	//dump
	//if(p_dump.isSet() &&!(p_volume_load_path.isSet()))
	//{
	//	sprintf(errMsg, "One or more required arguments missing for --%s!\n\nUSAGE is:\n--%s\n\t--%s%c<%s>",
	//		p_dump.getName().c_str(), p_dump.getName().c_str(),
	//		p_volume_load_path.getName().c_str(), cmd.getDelimiter(), "string");
	//	throw IOException(errMsg);
	//}
		
	//restoring parameters
	//if(p_enable_restore.isSet() && !p_restoring_direction.isSet())
	//{
	//	sprintf(errMsg, "One or more required arguments missing for --%s!\n\nUSAGE is:\n--%s --%s%c<%s>",
	//		p_enable_restore.getName().c_str(), p_enable_restore.getName().c_str(),
	//		p_restoring_direction.getName().c_str(), cmd.getDelimiter(), "integer");
	//	throw IOException(errMsg);
	//}

	//execution times parameters
	if(p_save_execution_times.isSet() && !p_execution_times_file.isSet())
	{
		sprintf(errMsg, "One or more required arguments missing for --%s!\n\nUSAGE is:\n--%s --%s%c<%s>",
			p_save_execution_times.getName().c_str(), p_save_execution_times.getName().c_str(),
			p_execution_times_file.getName().c_str(), cmd.getDelimiter(), "string");
		throw IOException(errMsg);
	}

	//checking that at least one operation has been selected
	//if(!(p_test.isSet() || p_import.isSet() || p_computedisplacements.isSet() || p_projdisplacements.isSet() || 
	if(!(p_import.isSet() || p_computedisplacements.isSet() || p_projdisplacements.isSet() || p_placetiles.isSet() || 
		p_thresholdisplacements.isSet() || p_placelayers.isSet() || p_merge.isSet() || p_stitch.isSet() ))
		throw IOException("No operation selected. See --help for usage.");

	//importing parameters
	//this->dumpMData = p_dump.getValue();
	//this->test = p_test.getValue();
	this->stitch = p_stitch.getValue();
	this->import = p_import.getValue();
	//this->image_format = p_image_format.getValue();
	this->computedisplacements = p_computedisplacements.getValue();
	this->projdisplacements = p_projdisplacements.getValue();
	this->placetiles = p_placetiles.getValue();
	this->thresholddisplacements = p_thresholdisplacements.getValue();
	this->placelayers = p_placelayers.getValue();
	this->mergetiles = p_merge.getValue();
	this->volume_load_path = p_volume_load_path.getValue();
	this->volume_save_path = p_volume_save_path.getValue();
	this->projfile_load_path = p_project_load_path.getValue();
	this->projfile_save_path = p_project_save_path.getValue();
	//this->reference_system.first  = axis(p_refsys_1.getValue());
	//this->reference_system.second = axis(p_refsys_2.getValue());
	//this->reference_system.third  = axis(p_refsys_3.getValue());
	//this->VXL_1 = p_vxl_1.getValue();
	//this->VXL_2 = p_vxl_2.getValue();
	//this->VXL_3 = p_vxl_3.getValue();
	//this->start_stack_row = p_start_stack_row.getValue();
	//this->end_stack_row = p_end_stack_row.getValue();
	//this->start_stack_col = p_start_stack_col.getValue();
	//this->end_stack_col = p_end_stack_col.getValue();
	this->start_layer = p_start_layer.getValue();
	this->end_layer = p_end_layer.getValue();
	//this->overlap_V = p_overlap_V.getValue();
	//this->overlap_H = p_overlap_H.getValue();
	this->search_radius_V = p_search_radius_V.getValue();
	this->search_radius_H = p_search_radius_H.getValue();
	this->search_radius_D = p_search_radius_D.getValue();
	this->substk_dim_V = p_substk_dim_V.getValue();
	this->substk_dim_H = p_substk_dim_H.getValue();
	this->reliability_threshold = p_reliability_threshold.getValue();
	this->slice_height = p_slice_height.getValue();
	this->slice_width = p_slice_width.getValue();
	this->slice_depth = p_slice_depth.getValue();
	for(int i=0; i<= S_MAX_MULTIRES; i++)
	{
		stringstream buf;
		buf << i;
		resolutions[i] = p_resolutions.getValue().find(buf.str()) != std::string::npos;
	}
	//this->exclude_nonstitchables = p_exclude_nonstitchables.getValue();
	this->D0 = p_D0.getValue();
	this->D1 = p_D1.getValue();
	//this->enable_restore = p_enable_restore.getValue();
	//this->restoring_direction = p_restoring_direction.getValue();
	this->save_execution_times = p_save_execution_times.getValue();
	this->execution_times_filename = p_execution_times_file.getValue();
	this->show_progress_bar = !p_hide_progress_bar.getValue();
	this->img_format = p_saved_img_format.getValue();
	this->img_depth = p_saved_img_depth.getValue();
	volumemanager::IMG_FILTER_REGEX = p_img_regex.getValue();

	if(p_img_channel_select.getValue().compare("all") == 0)
		iomanager::CHANS = iomanager::ALL;
	else if(p_img_channel_select.getValue().compare("R") == 0)
		iomanager::CHANS = iomanager::R;
	else if(p_img_channel_select.getValue().compare("G") == 0)
		iomanager::CHANS = iomanager::G;
	else if(p_img_channel_select.getValue().compare("B") == 0)
		iomanager::CHANS = iomanager::B;
	else
		throw iom::exception(iomanager::strprintf("Invalid argument \"%s\" for parameter --%s! Allowed values are {\"all\",\"R\",\"G\",\"B\"}", p_img_channel_select.getValue().c_str(), p_img_channel_select.getName().c_str()).c_str());

	// 2017-02-10. Giulio. A different blending for layer can be specified
	// the layer blending algorithm is the same used between tiles of the same layer if no specific blending algorithm for layes has benne specified
	bool layer_blending_init = false; // if a layer blending algorithm has been already found 

	//the [algorithm] parameter is multi-arguments
	vector<string> algorithms = p_algo.getValue();
 	for(int i = 0; i < algorithms.size(); i++)
		if(algorithms[i].compare(S_NCC_NAME) == 0)
			this->pd_algo = S_NCC_MODE;
		else if(algorithms[i].compare(S_PC_NAME) == 0)
			this->pd_algo = S_PHASE_CORRELATION_MODE;
		else if(algorithms[i].compare(S_FATPM_SP_TREE_NAME) == 0)
			this->tp_algo = S_FATPM_SP_TREE;
		else if(algorithms[i].compare(S_FATPM_SCAN_V_NAME) == 0)
			this->tp_algo = S_FATPM_SCAN_V;
		else if(algorithms[i].compare(S_FATPM_SCAN_H_NAME) == 0)
			this->tp_algo = S_FATPM_SCAN_H;
		else if(algorithms[i].compare(S_NO_BLENDING_NAME) == 0) {
			this->tm_blending = S_NO_BLENDING;
			if ( !layer_blending_init ) 
				this->lm_blending = S_NO_BLENDING;
		}
		else if(algorithms[i].compare(S_SINUSOIDAL_BLENDING_NAME) == 0) {
			this->tm_blending = S_SINUSOIDAL_BLENDING;
			if ( !layer_blending_init ) 
				this->lm_blending = S_SINUSOIDAL_BLENDING;
		}
		else if(algorithms[i].compare(S_SHOW_STACK_MARGIN_NAME) == 0) {
			this->tm_blending = S_SHOW_STACK_MARGIN;
			if ( !layer_blending_init ) 
				this->lm_blending = S_SHOW_STACK_MARGIN;
		}
		else if(algorithms[i].compare(S_ENHANCED_NO_BLENDING_NAME) == 0) {
			this->tm_blending = S_ENHANCED_NO_BLENDING;
			if ( !layer_blending_init ) 
				this->lm_blending = S_ENHANCED_NO_BLENDING;
		}
		else if(algorithms[i].compare(S_TOPLAYER_OVERWRITE_NAME) == 0) {
			this->lm_blending = S_TOPLAYER_OVERWRITE;
			layer_blending_init = true;
		}
		else
		{
			sprintf(errMsg, "Invalid argument \"%s\" for parameter --%s! Allowed values are:\n-\"%s\"\n-\"%s\"\n-\"%s\"\n-\"%s\"\n-\"%s\"\n-\"%s\"\n",	algorithms[i].c_str(), p_algo.getName().c_str(), S_NCC_NAME, S_FATPM_SP_TREE_NAME,S_FATPM_SCAN_V_NAME,S_FATPM_SCAN_H_NAME, S_NO_BLENDING_NAME, S_SINUSOIDAL_BLENDING_NAME);
			throw IOException(errMsg);
		}
	//this->ignoreUnequalStacksDepth = p_ignoreUnequalStacksDepth.getValue();
	this->norm_fact_D = p_norm_fact_D.getValue();
	this->cut_depth = p_cut_depth.getValue();

	this->libtiff_uncompressed = p_libtiff_uncompressed.getValue();
	this->libtiff_rowsPerStrip = p_libtiff_rowsperstrip.getValue();
}

//checks parameters correctness
void TeraStitcher2CLI::checkParams() throw (IOException)
{
	//parameters check should be done here.
	//We trust in current tool functions checks.
	//print();

	//checking that other operations are disabled when test mode is enabled
	//if(test && (stitch || import || computedisplacements || projdisplacements || thresholddisplacements || placetiles || mergetiles))
	//	throw IOException("Test has been selected! No other operations can be performed.");
}

//returns help text
string TeraStitcher2CLI::getHelpText()
{
	stringstream helptext;
	helptext << "TeraStitcher2 " << terastitcher::terastitcher2_version << "\n";
	helptext << "  developed at University Campus Bio-Medico of Rome by:\n";
	helptext << "   -\tAlessandro Bria (email: a.bria@unicas.it)                            ";
	helptext << "    \tPhD student at Departement of Electrical and Information Engineering";
	helptext << "    \tFaculty of Engineering of University of Cassino\n";
	helptext << "   -\tGiulio Iannello, Ph.D. (email: g.iannello@unicampus.it)              ";
	helptext << "    \tFull Professor of Computer Science and Computer Engineering          ";
	helptext << "    \tFaculty of Engineering of University Campus Bio-Medico of Rome\n";
	helptext << "  Official website/repo: http://abria.github.io/TeraStitcher";
	return helptext.str();
}

//print all arguments
void TeraStitcher2CLI::print()
{
	printf("\n\n");
	//printf("test = \t\t%s\n", test ? "ENABLED" : "disabled");
	printf("stitch = \t\t%s\n", stitch ? "ENABLED" : "disabled");
	printf("import = \t\t%s\n", import ? "ENABLED" : "disabled");
	printf("computedisplacements = \t%s\n", computedisplacements ? "ENABLED" : "disabled");
	//printf("projdisplacements = \t%s\n", projdisplacements ? "ENABLED" : "disabled");
	printf("placetiles = \t\t%s\n", placetiles ? "ENABLED" : "disabled");
	printf("thresholdisplacements = %s\n", thresholddisplacements ? "ENABLED" : "disabled");
	printf("placetiles = \t\t%s\n", placelayers ? "ENABLED" : "disabled");
	printf("mergetiles = \t\t%s\n", mergetiles ? "ENABLED" : "disabled");
	printf("volume_load_path = \t%s\n", volume_load_path.c_str());
	printf("volume_save_path = \t%s\n", volume_save_path.c_str());
	printf("projfile_load_path = \t%s\n", projfile_load_path.c_str());
	printf("projfile_save_path = \t%s\n", projfile_save_path.c_str());
	//printf("reference_system = \t{%d,%d,%d}\n", reference_system.first, reference_system.second, reference_system.third);
	//printf("VXL = \t\t\t%.1f x %.1f x %.1f\n", VXL_1, VXL_2, VXL_3);
	printf("pd_algo = \t\t%d\n", pd_algo);
	//printf("start_stack_row = \t%d\n", start_stack_row);
	//printf("end_stack_row = \t%d\n", end_stack_row);
	//printf("start_stack_col = \t%d\n", start_stack_col);
	//printf("end_stack_col = \t%d\n", end_stack_col);
	printf("start_layer = \t%d\n", start_layer);
	printf("end_layer = \t%d\n", end_layer);
	//printf("overlap_V = \t\t%d\n", overlap_V);
	//printf("overlap_H = \t\t%d\n", overlap_H);
	printf("search radius = \t%d(V) x %d(H) x %d(D)\n", search_radius_V, search_radius_H, search_radius_D);
	printf("substk_dim_V = \t\t%d\n", substk_dim_V);
	printf("substk_dim_H = \t\t%d\n", substk_dim_H);
	printf("reliability_threshold = %.2f\n", reliability_threshold);
	printf("tp_algo = \t\t%d\n", tp_algo);
	printf("slice dimensions = \t%d(V) x %d(H)\n", slice_height, slice_width);
	printf("resolutions = \t\t");
	for(int i=0; i<=S_MAX_MULTIRES; i++)
		if(resolutions[i])
			printf("%d ", i);
	printf("\n");
	//printf("exclude_nonstitchables =%s\n", exclude_nonstitchables ? "ENABLED" : "disabled");
	printf("D0 = \t\t\t%d\n", D0);
	printf("D1 = \t\t\t%d\n", D1);
	printf("tm_blending = \t\t%d\n", tm_blending);
	//printf("enable_restore = \t%s\n", enable_restore ? "ENABLED" : "disabled");
	//printf("restoring_direction = \t%d\n", restoring_direction);
	printf("norm_fact_D = \t\t%.1f\n", norm_fact_D);
	printf("cut_depth = \t\t%.1f\n", cut_depth);
}
