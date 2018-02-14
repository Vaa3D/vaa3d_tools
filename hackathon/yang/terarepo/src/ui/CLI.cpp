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
* 2017-06-30. Giulio.     @ADDED control over displacement computation of last row and last column of tiles
* 2017-03-27. Giulio.     @ADDED full support for multi-channel images (more than three channels are allowed) 
* 2016-09-04. Giulio.     @ADDED the options for setting the configuration of the LibTIFF library
* 2015-08-16. Giulio.     @ADDED the 'method' and 'isotropi' parameters 
* 2015-06-12. Giulio      @ADDED parameter to specify the path and name of an error log file
* 2014-12-10. Giulio.     @ADDED parallel flag
* 2014-12-10. Alessandro. @FIXED check of "Import" step if a project file is provided.
* 2014-12-06. Giulio.     @ADDED makedirs flag.
* 2014-11-22. Giulio.     @CHANGED input plugin used for StackedVolume volumes is "tiff2D"
*/

#include "CLI.h"
#include "volumemanager.config.h"
#include "IOPluginAPI.h"
#include "config.h"
#include <tclap/CmdLine.h>
#include <sstream>

bool isNumber ( std::string chan ) {
	bool isDigit = true;
	for ( int i=0; i<chan.size() && isDigit; i++ ) 
		isDigit = isdigit(chan.c_str()[i]) != 0;
	return isDigit;
}

TeraStitcherCLI::TeraStitcherCLI(void)
{
	//initialization should be done for all parameters, but we use TCLAP default values for arguments (see readParams)
	this->pd_algo = S_NCC_MODE;
	this->tp_algo = S_FATPM_SP_TREE;
	this->tm_blending = S_SINUSOIDAL_BLENDING;
}

//reads options and parameters from command line
void TeraStitcherCLI::readParams(int argc, char** argv) throw (iom::exception)
{
	//command line object definition
	TCLAP::CmdLine cmd(getHelpText(), '=', terastitcher::version + (ts::qtversion.empty() ? "" : " (with Qt " + ts::qtversion + ")"));

	//argument objects definitions
	TCLAP::SwitchArg p_stitch("S","stitch","Stitch a volume by executing the entire pipeline (steps 1-6).",false);
	TCLAP::SwitchArg p_import("1","import","Step 1: import a volume into TeraStitcher and prepare it for processing.", false);
	TCLAP::SwitchArg p_computedisplacements("2","displcompute","Step 2: compute pairwise stacks displacements.", false);
	TCLAP::SwitchArg p_projdisplacements("3","displproj","Step 3: project existing displacements along Z axis for each stack by selecting the most reliable one.", false);
	TCLAP::SwitchArg p_thresholdisplacements("4","displthres","Step 4: threshold displacements using the given reliability threshold.", false);
	TCLAP::SwitchArg p_placetiles("5","placetiles","Step 5: places tiles in the image space using a globally optimal tiles placement algorithm.", false);
	TCLAP::SwitchArg p_merge("6","merge","Step 6: merges tiles at different resolutions.", false);
	TCLAP::SwitchArg p_test("t","test","(deprecated) Stitches the middle slice of the whole volume and saves it locally. Stage coordinates will be used, so this can be used to test their precision as well as the selected reference system.",false);
	TCLAP::SwitchArg p_parallel("","parallel","Does not perform side-effect operations during the merge step. Use this flag when more merge steps are launched in parallel",false);
	TCLAP::SwitchArg p_isotropic("","isotropic","Generate lowest resolution with voxels as much isotropic as possible. Use this flag when the high resolution image has highy anistropic voxels",false);
	TCLAP::SwitchArg p_dump("d","dump","Print the entire content of metadata file mdata.bin",false);
	TCLAP::SwitchArg p_pluginsinfo("p","pluginsinfo","Display plugins informations",false);
	TCLAP::ValueArg<std::string> p_vol_in_path("","volin","Directory path where the volume is stored.",false,"null","string");
	TCLAP::ValueArg<std::string> p_vol_out_path("","volout","Directory path where to save the stitched volume.",false,"null","string");
	TCLAP::ValueArg<std::string> p_vol_in_plugin("","volin_plugin",vm::strprintf("Plugin that manages the input volume format/organization. Available plugins are: {%s}. Default is \"%s\".", vm::VirtualVolumeFactory::registeredPlugins().c_str(), vm::VOLUME_INPUT_FORMAT_PLUGIN.c_str()),false,vm::VOLUME_INPUT_FORMAT_PLUGIN,"string");
	TCLAP::ValueArg<std::string> p_vol_out_plugin("","volout_plugin",vm::strprintf("Plugin that manages the output volume format/organization. Available plugins are: {%s}. Default is \"%s\".", vm::VirtualVolumeFactory::registeredPlugins().c_str(), vm::VOLUME_OUTPUT_FORMAT_PLUGIN.c_str()),false,vm::VOLUME_OUTPUT_FORMAT_PLUGIN,"string");
	TCLAP::ValueArg<std::string> p_proj_in_path("","projin","File path of the XML project file to be loaded.",false,"null","string");
	TCLAP::ValueArg<std::string> p_proj_out_path("","projout","File path of the XML project file to be saved.",false,"","string");
	TCLAP::ValueArg<std::string> p_errlogfile_path("","errlog","File path of the error log file to be saved.",false,"","string");
	TCLAP::ValueArg<std::string> p_refsys_1("","ref1",	"First axis of the used reference system.                              "
														"'Y' or 'V' or '1' = Vertical                                          "
														"'X' or 'H' or '2' = Horizontal                                        "
														"'Z' or 'D' or '3' = Depth                                             "
														"use '-' prefix for inverted axis (e.g. '-X')",false,"null","string");
	TCLAP::ValueArg<std::string> p_refsys_2("","ref2","Second axis of the used reference system (see --ref1)",false,"null","string");
	TCLAP::ValueArg<std::string> p_refsys_3("","ref3","Third axis of the used reference system (see --ref1)",false,"null","string");
	TCLAP::ValueArg<float> p_vxl_1("","vxl1","Voxel dimension along first axis (in microns).",false,0,"real");
	TCLAP::ValueArg<float> p_vxl_2("","vxl2","Voxel dimension along second axis (in microns).",false,0,"real");
	TCLAP::ValueArg<float> p_vxl_3("","vxl3","Voxel dimension along third axis (in microns).",false,0,"real");
	TCLAP::MultiArg<std::string> p_algo("","algorithm","Forces the use of the given algorithm.",false,"string");
	TCLAP::ValueArg<int> p_start_stack_row("","R0","First row of stacks to be processed.",false,-1,"integer");
	TCLAP::ValueArg<int> p_end_stack_row("","R1","Last row of stacks to be processed.",false,-1,"integer");
	TCLAP::ValueArg<int> p_start_stack_col("","C0","First column of stacks to be processed.",false,-1,"integer");
	TCLAP::ValueArg<int> p_end_stack_col("","C1","Last column of stacks to be processed.",false,-1,"integer");
	TCLAP::ValueArg<int> p_D0("","D0","First slice to be processed (index along D).",false,-1,"integer");
	TCLAP::ValueArg<int> p_D1("","D1","Last slice to be processed (index along D).",false,-1,"integer");
	TCLAP::ValueArg<int> p_overlap_V("","oV","Overlap (in pixels) between two adjacent tiles along V.",false,-1,"integer");
	TCLAP::ValueArg<int> p_overlap_H("","oH","Overlap (in pixels) between two adjacent tiles along H.",false,-1,"integer");
	TCLAP::ValueArg<int> p_search_radius_V("","sV","Displacements search radius along V (in pixels).",false,S_DISPL_SEARCH_RADIUS_DEF,"integer");
	TCLAP::ValueArg<int> p_search_radius_H("","sH","Displacements search radius along H (in pixels).",false,S_DISPL_SEARCH_RADIUS_DEF,"integer");
	TCLAP::ValueArg<int> p_search_radius_D("","sD","Displacements search radius along D (in pixels).",false,S_DISPL_SEARCH_RADIUS_DEF,"integer");
	TCLAP::ValueArg<int> p_subvol_dim_D("","subvoldim","Number of slices per subvolume partition used in the pairwise displacements computation step.",false,S_SUBVOL_DIM_D_DEFAULT,"integer");
	TCLAP::ValueArg<float> p_reliability_threshold("","threshold","Reliability threshold. Values are in [0.0, 1.0] where 0 = unreliable, 1.0 = totally reliable. Default is 0.7.",false,(float)0.7,"real");
	TCLAP::ValueArg<int> p_slice_height("","sliceheight","Desired slice height of merged tiles (in pixels).",false,-1,"integer");
	TCLAP::ValueArg<int> p_slice_width("","slicewidth","Desired slice width of merged tiles (in pixels).",false,-1,"integer");
	TCLAP::ValueArg<int> p_slice_depth("","slicedepth","Desired block depth of merged tiles (in pixels).",false,-1,"integer");
	TCLAP::ValueArg<std::string> p_resolutions("","resolutions","Resolutions to be produced. Possible values are [[i]...]             where i = 0,..,5 and 2^i is the subsampling factor.",false,"0","string");
	TCLAP::SwitchArg p_exclude_nonstitchables("","stitchablesonly","Excludes non-stitchables stacks from the stitched volume.", false);
	TCLAP::SwitchArg p_enable_restore("","restoreSPIM","Enables SPIM artifacts removal (zebrated pattern).", false);
	TCLAP::ValueArg<std::string> p_restoring_direction("","restoredir","SPIM zebrated pattern direction (see --ref1)",false,"","string");
	TCLAP::SwitchArg p_save_execution_times("","exectimes","Saves execution times into a file located in the volume directory.", false);
	TCLAP::ValueArg<std::string> p_execution_times_file("","exectimesfile","Name of file where to write execution times. ",false,"null","string");
	TCLAP::SwitchArg p_hide_progress_bar("","noprogressbar","Disables progress bar and estimated time remaining", false);
	
	// input image parameters
	TCLAP::ValueArg<std::string> p_im_in_regex("",			"imin_regex","A regular expression to be used to match image filenames when the volume is imported (by default, all the images whose file extension is supported by the selected I/O plugin are found)",false,"","string");
	TCLAP::ValueArg<std::string> p_im_in_channel("",		"imin_channel","The channel(s) to be selected when the images are loaded. Allowed values are {\"all\",\"R\",\"G\",\"B\"}. Default is \"all\" (color images will be converted to grayscale).",false,"all","string");
	TCLAP::ValueArg<std::string> p_im_in_plugin("",			"imin_plugin",vm::strprintf("Plugin that manages the input image format. Available plugins are: {%s}. Default is \"auto\".", iom::IOPluginFactory::registeredPlugins().c_str()), false, "auto","string");
	TCLAP::ValueArg<std::string> p_im_in_plugin_params("",	"imin_plugin_params","A series of parameters \"param1=val,param2=val,...\" to configure the input image plugin (see --pluginsinfo for the list of accepted parameters)", false, "","string");
	
	// output image parameters
	TCLAP::ValueArg<std::string> p_im_out_format("",		"imout_format","Output image format extension (\"tif\" is default, others: see --pluginsinfo). ",false,"tif","string");
	TCLAP::ValueArg<int> im_out_depth("",					"imout_depth","Output image colordepth/bits per pixel (\"8\" is default). ",false,8,"integer");
	TCLAP::ValueArg<std::string> p_im_out_plugin("",		"imout_plugin",vm::strprintf("Plugin that manages the output image format. Available plugins are: {%s}. Default is \"auto\".", iom::IOPluginFactory::registeredPlugins().c_str()), false, "auto","string");
	TCLAP::ValueArg<std::string> p_im_out_plugin_params("",	"imout_plugin_params","A series of parameters \"param1=val,param2=val,...\" to configure the output image plugin (see --pluginsinfo for the list of accepted parameters)", false, "","string");

	TCLAP::SwitchArg p_sparse_data("","sparse_data","If enabled, this option allows to import sparse data organizations, i.e. with empty or incomplete tiles",false);
    TCLAP::SwitchArg p_rescan("","rescan","If enabled, TeraStitcher will rescan all acquisition files according to the given import parameters",false);
	TCLAP::SwitchArg p_makedirs("","makedirs","Creates the mdata.bin file of the output volume.", false);
	TCLAP::SwitchArg p_metadata("","metadata","Creates the directory hierarchy.", false);
	TCLAP::ValueArg<std::string> p_halving_method("","halve","Halving method (mean/max, default: mean).",false,"mean","string");

	TCLAP::SwitchArg p_libtiff_uncompressed("","libtiff_uncompress","Configure libtiff library to not compress output files (default: compression enabled).", false);
	TCLAP::SwitchArg p_libtiff_bigtiff("","libtiff_bigtiff","Foces the creation of BigTiff files (default: BigTiff disabled).", false);
	TCLAP::ValueArg<int> p_libtiff_rowsperstrip("","libtiff_rowsperstrip","Configure libtiff library to pack n rows per strip when compression is enabled (default: 1 row per strip).",false,1,"integer");

	TCLAP::SwitchArg p_disable_last_row("","disable_last_row","disable displacement computation of last row (default: last row enabled, active only in parallel mode).", false);
	TCLAP::SwitchArg p_disable_last_col("","disable_last_col","disable displacement computation of last column (default: last column enabled, active only in parallel mode).", false);

	// argument objects must be inserted using FIFO policy (first inserted, first shown)
	cmd.add(p_disable_last_col);
	cmd.add(p_disable_last_row);

	cmd.add(p_libtiff_rowsperstrip);
	cmd.add(p_libtiff_bigtiff);
	cmd.add(p_libtiff_uncompressed);

	cmd.add(p_im_out_plugin_params);
	cmd.add(p_im_out_plugin);
	cmd.add(im_out_depth);
	cmd.add(p_im_out_format);

	cmd.add(p_im_in_plugin_params);
	cmd.add(p_im_in_plugin);
	cmd.add(p_im_in_channel);
	cmd.add(p_im_in_regex);

	cmd.add(p_halving_method);
    cmd.add(p_metadata);
    cmd.add(p_makedirs);
    cmd.add(p_rescan);
	cmd.add(p_sparse_data);
	cmd.add(p_hide_progress_bar);
	cmd.add(p_execution_times_file);
	cmd.add(p_save_execution_times);
	cmd.add(p_restoring_direction);
	cmd.add(p_enable_restore);
	cmd.add(p_exclude_nonstitchables);
	cmd.add(p_resolutions);
	cmd.add(p_slice_depth);
	cmd.add(p_slice_width);
	cmd.add(p_slice_height);
	cmd.add(p_reliability_threshold);
	cmd.add(p_subvol_dim_D);
	cmd.add(p_search_radius_D);
	cmd.add(p_search_radius_H);
	cmd.add(p_search_radius_V);
	cmd.add(p_overlap_H);
	cmd.add(p_overlap_V);
	cmd.add(p_D1);
	cmd.add(p_D0);
	cmd.add(p_end_stack_col);
	cmd.add(p_start_stack_col);
	cmd.add(p_end_stack_row);
	cmd.add(p_start_stack_row);
	cmd.add(p_algo);
	cmd.add(p_vxl_3);
	cmd.add(p_vxl_2);
	cmd.add(p_vxl_1);
	cmd.add(p_refsys_3);
	cmd.add(p_refsys_2);
	cmd.add(p_refsys_1);
	cmd.add(p_errlogfile_path);
	cmd.add(p_proj_out_path);
	cmd.add(p_proj_in_path);
	cmd.add(p_vol_out_plugin);
	cmd.add(p_vol_out_path);
	cmd.add(p_vol_in_plugin);
	cmd.add(p_vol_in_path);
	cmd.add(p_pluginsinfo);
	cmd.add(p_dump);
	cmd.add(p_stitch);
	cmd.add(p_merge);
	cmd.add(p_placetiles);
	cmd.add(p_thresholdisplacements);
	cmd.add(p_projdisplacements);
	cmd.add(p_computedisplacements);
	cmd.add(p_import);
	cmd.add(p_test);
	cmd.add(p_parallel);
	cmd.add(p_isotropic);

	// Parse the argv array and catch <TCLAP> exceptions, which are translated into <iom::iim::IOException> exceptions
	char errMsg[S_STATIC_STRINGS_SIZE];
	try{ cmd.parse( argc, argv ); } 
	catch (TCLAP::ArgException &e)
	{ 
		sprintf(errMsg, "%s for arg %s\n", e.error().c_str(), e.argId().c_str());
		throw iom::exception(errMsg);
	}

	/* Checking mandatory parameters for each step */

	//TEST
	if(p_test.isSet() && !(
		p_vol_in_path.isSet() && 
		p_refsys_1.isSet() && p_refsys_2.isSet() && p_refsys_3.isSet() &&
		p_vxl_1.isSet() && p_vxl_2.isSet() && p_vxl_3.isSet() ) && !p_proj_in_path.isSet())
	{
		sprintf(errMsg, "One or more required arguments missing for --%s!\nUSAGE is:\n--%s\n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s>\n\nor\n\n--%s\n\t--%s%c<%s>", 
			p_test.getName().c_str(), p_test.getName().c_str(),
			p_vol_in_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_refsys_1.getName().c_str(), cmd.getDelimiter(), "string",
			p_refsys_2.getName().c_str(), cmd.getDelimiter(), "string",
			p_refsys_3.getName().c_str(), cmd.getDelimiter(), "string",
			p_vxl_1.getName().c_str(), cmd.getDelimiter(), "real",
			p_vxl_2.getName().c_str(), cmd.getDelimiter(), "real",
			p_vxl_3.getName().c_str(), cmd.getDelimiter(), "real",
			p_test.getName().c_str(),
			p_proj_in_path.getName().c_str(), cmd.getDelimiter(), "string");
		throw iom::exception(errMsg);
	}

	if(p_parallel.isSet() && !(p_stitch.isSet() || p_computedisplacements.isSet() || p_merge.isSet())) {
		throw iom::exception("--parallel option is set, but neither --stitch, nor --displcompute, nor --merge have been launched");
	}

	//STEP 1
	// @FIXED by Alessandro on 2014-12-10. Skip this check if a project file has been provided.
	if(p_import.isSet() && !p_proj_in_path.isSet() && !(
		p_vol_in_path.isSet() && 
		p_refsys_1.isSet() && p_refsys_2.isSet() && p_refsys_3.isSet() &&
		p_vxl_1.isSet() && p_vxl_2.isSet() && p_vxl_3.isSet() ))
	{
		sprintf(errMsg, "One or more required arguments missing for --%s!\nUSAGE is:\n--%s\n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s>", 
			p_import.getName().c_str(), p_import.getName().c_str(),
			p_vol_in_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_proj_out_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_refsys_1.getName().c_str(), cmd.getDelimiter(), "string",
			p_refsys_2.getName().c_str(), cmd.getDelimiter(), "string",
			p_refsys_3.getName().c_str(), cmd.getDelimiter(), "string",
			p_vxl_1.getName().c_str(), cmd.getDelimiter(), "real",
			p_vxl_2.getName().c_str(), cmd.getDelimiter(), "real",
			p_vxl_3.getName().c_str(), cmd.getDelimiter(), "real");
		throw iom::exception(errMsg);
	}

	//STEP 2
	if(p_computedisplacements.isSet() &&!( 
		(p_vol_in_path.isSet() && p_refsys_1.isSet() && p_refsys_2.isSet() && p_refsys_3.isSet() && p_vxl_1.isSet() && p_vxl_2.isSet() && p_vxl_3.isSet())
		|| 	
		(p_proj_in_path.isSet())))
	{
		sprintf(errMsg, "One or more required arguments missing for --%s!\n\nUSAGE 1 is:\n--%s\n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s --%s%c<%s>]", 
			p_computedisplacements.getName().c_str(), p_computedisplacements.getName().c_str(),
			p_vol_in_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_refsys_1.getName().c_str(), cmd.getDelimiter(), "string",
			p_refsys_2.getName().c_str(), cmd.getDelimiter(), "string",
			p_refsys_3.getName().c_str(), cmd.getDelimiter(), "string",
			p_vxl_1.getName().c_str(), cmd.getDelimiter(), "real",
			p_vxl_2.getName().c_str(), cmd.getDelimiter(), "real",
			p_vxl_3.getName().c_str(), cmd.getDelimiter(), "real",
			p_proj_out_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_algo.getName().c_str(), cmd.getDelimiter(), "string",
			p_start_stack_row.getName().c_str(), cmd.getDelimiter(), "integer",
			p_end_stack_row.getName().c_str(), cmd.getDelimiter(), "integer",
			p_start_stack_col.getName().c_str(), cmd.getDelimiter(), "integer",
			p_end_stack_col.getName().c_str(), cmd.getDelimiter(), "integer",
			p_overlap_V.getName().c_str(), cmd.getDelimiter(), "integer",
			p_overlap_H.getName().c_str(), cmd.getDelimiter(), "integer",
			p_search_radius_V.getName().c_str(), cmd.getDelimiter(), "integer",
			p_search_radius_H.getName().c_str(), cmd.getDelimiter(), "integer",
			p_search_radius_D.getName().c_str(), cmd.getDelimiter(), "integer",
			p_subvol_dim_D.getName().c_str(), cmd.getDelimiter(), "integer",
			p_enable_restore.getName().c_str(),
			p_restoring_direction.getName().c_str(), cmd.getDelimiter(), "string");
			sprintf(errMsg, "%s\n\nUSAGE 2 is:\n--%s\n\t--%s%c<%s> \n\t--%s%c<%s> \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s --%s%c<%s>]", 
			errMsg,
			p_computedisplacements.getName().c_str(),
			p_proj_in_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_proj_out_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_algo.getName().c_str(), cmd.getDelimiter(), "string",
			p_start_stack_row.getName().c_str(), cmd.getDelimiter(), "integer",
			p_end_stack_row.getName().c_str(), cmd.getDelimiter(), "integer",
			p_start_stack_col.getName().c_str(), cmd.getDelimiter(), "integer",
			p_end_stack_col.getName().c_str(), cmd.getDelimiter(), "integer",
			p_overlap_V.getName().c_str(), cmd.getDelimiter(), "integer",
			p_overlap_H.getName().c_str(), cmd.getDelimiter(), "integer",
			p_search_radius_V.getName().c_str(), cmd.getDelimiter(), "integer",
			p_search_radius_H.getName().c_str(), cmd.getDelimiter(), "integer",
			p_search_radius_D.getName().c_str(), cmd.getDelimiter(), "integer",
			p_subvol_dim_D.getName().c_str(), cmd.getDelimiter(), "integer",
			p_enable_restore.getName().c_str(),
			p_restoring_direction.getName().c_str(), cmd.getDelimiter(), "string");
		throw iom::exception(errMsg);
	}

	//STEP 3
	if(p_projdisplacements.isSet() &&!(p_proj_in_path.isSet()))
	{
		sprintf(errMsg, "One or more required arguments missing for --%s!\n\nUSAGE is:\n--%s\n\t--%s%c<%s> \n\t--%s%c<%s>",
			    p_projdisplacements.getName().c_str(), p_projdisplacements.getName().c_str(),
				p_proj_in_path.getName().c_str(), cmd.getDelimiter(), "string",
				p_proj_out_path.getName().c_str(), cmd.getDelimiter(), "string");
		throw iom::exception(errMsg);
	}

	//STEP 4
	if(p_thresholdisplacements.isSet() &&!(p_proj_in_path.isSet() && p_reliability_threshold.isSet()))
	{
		sprintf(errMsg, "One or more required arguments missing for --%s!\n\nUSAGE is:\n--%s\n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s>",
			    p_thresholdisplacements.getName().c_str(), p_thresholdisplacements.getName().c_str(),
				p_proj_in_path.getName().c_str(), cmd.getDelimiter(), "string",
				p_proj_out_path.getName().c_str(), cmd.getDelimiter(), "string",
				p_reliability_threshold.getName().c_str(), cmd.getDelimiter(), "real");
		throw iom::exception(errMsg);
	}

	//STEP 5
	if(p_placetiles.isSet() && !(p_proj_in_path.isSet()))
	{
		sprintf(errMsg, "One or more required arguments missing for --%s!\n\nUSAGE is:\n--%s\n\t--%s%c<%s> \n\t--%s%c<%s> \n\t[--%s%c<%s>]",
			p_placetiles.getName().c_str(), p_placetiles.getName().c_str(),
			p_proj_in_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_proj_out_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_algo.getName().c_str(), cmd.getDelimiter(), "string");
		throw iom::exception(errMsg);
	}

	//STEP 6 or makedirs or metadata is set
	if((p_merge.isSet() || p_makedirs.isSet() ||p_metadata.isSet()) &&!(p_proj_in_path.isSet() && p_vol_out_path.isSet()))
	{
		sprintf(errMsg, "One or more required arguments missing for --%s!\n\nUSAGE is:\n--%s\n\t--%s%c<%s> \n\t--%s%c<%s> \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s --%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>]",
			p_merge.getName().c_str(), p_merge.getName().c_str(),
			p_proj_in_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_vol_out_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_slice_height.getName().c_str(), cmd.getDelimiter(), "integer",
			p_slice_width.getName().c_str(), cmd.getDelimiter(), "integer",
			p_resolutions.getName().c_str(), cmd.getDelimiter(), "string",
			p_exclude_nonstitchables.getName().c_str(),
			p_start_stack_row.getName().c_str(), cmd.getDelimiter(), "integer",
			p_end_stack_row.getName().c_str(), cmd.getDelimiter(), "integer",
			p_start_stack_col.getName().c_str(), cmd.getDelimiter(), "integer",
			p_end_stack_col.getName().c_str(), cmd.getDelimiter(), "integer",
			p_D0.getName().c_str(), cmd.getDelimiter(), "integer",
			p_D1.getName().c_str(), cmd.getDelimiter(), "integer",
			p_enable_restore.getName().c_str(),
			p_restoring_direction.getName().c_str(), cmd.getDelimiter(), "string",
			im_out_depth.getName().c_str(), cmd.getDelimiter(), "integer",
			p_im_out_format.getName().c_str(), cmd.getDelimiter(), "string");
		throw iom::exception(errMsg);
	}

	//STEP 1-6 (--stitch option)
	if(p_stitch.isSet() &&!(
		p_vol_in_path.isSet() && p_proj_out_path.isSet() && 
		p_refsys_1.isSet() && p_refsys_2.isSet() && p_refsys_3.isSet() &&
		p_vxl_1.isSet() && p_vxl_2.isSet() && p_vxl_3.isSet() &&
		p_reliability_threshold.isSet() && p_vol_out_path.isSet()))
	{
		sprintf(errMsg, "One or more required arguments missing for --%s!\nUSAGE is:\n--%s \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t--%s%c<%s> \n\t[--%s%c<%s>]... (accepted multiple times) \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s --%s%c<%s>] \n\t[--%s%c<%s>] \n\t[--%s%c<%s>]", 
			p_stitch.getName().c_str(), p_stitch.getName().c_str(),
			p_vol_in_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_refsys_1.getName().c_str(), cmd.getDelimiter(), "string",
			p_refsys_2.getName().c_str(), cmd.getDelimiter(), "string",
			p_refsys_3.getName().c_str(), cmd.getDelimiter(), "string",
			p_vxl_1.getName().c_str(), cmd.getDelimiter(), "real",
			p_vxl_2.getName().c_str(), cmd.getDelimiter(), "real",
			p_vxl_3.getName().c_str(), cmd.getDelimiter(), "real",
			p_vol_out_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_proj_out_path.getName().c_str(), cmd.getDelimiter(), "string",
			p_reliability_threshold.getName().c_str(), cmd.getDelimiter(), "real",
			p_algo.getName().c_str(), cmd.getDelimiter(), "string",
			p_start_stack_row.getName().c_str(), cmd.getDelimiter(), "integer",
			p_end_stack_row.getName().c_str(), cmd.getDelimiter(), "integer",
			p_start_stack_col.getName().c_str(), cmd.getDelimiter(), "integer",
			p_end_stack_col.getName().c_str(), cmd.getDelimiter(), "integer",
			p_overlap_V.getName().c_str(), cmd.getDelimiter(), "integer",
			p_overlap_H.getName().c_str(), cmd.getDelimiter(), "integer",
			p_search_radius_V.getName().c_str(), cmd.getDelimiter(), "integer",
			p_search_radius_H.getName().c_str(), cmd.getDelimiter(), "integer",
			p_search_radius_D.getName().c_str(), cmd.getDelimiter(), "integer",
			p_subvol_dim_D.getName().c_str(), cmd.getDelimiter(), "integer",
			p_slice_height.getName().c_str(), cmd.getDelimiter(), "integer",
			p_slice_width.getName().c_str(), cmd.getDelimiter(), "integer",
			p_resolutions.getName().c_str(), cmd.getDelimiter(), "string",
			p_exclude_nonstitchables.getName().c_str(),
			p_D0.getName().c_str(), cmd.getDelimiter(), "integer",
			p_D1.getName().c_str(), cmd.getDelimiter(), "integer",
			p_enable_restore.getName().c_str(),
			p_restoring_direction.getName().c_str(), cmd.getDelimiter(), "string",
			im_out_depth.getName().c_str(), cmd.getDelimiter(), "integer",
			p_im_out_format.getName().c_str(), cmd.getDelimiter(), "string");
		throw iom::exception(errMsg);
	}
		
	//dump
	if(p_dump.isSet() &&!(p_vol_in_path.isSet()))
	{
		sprintf(errMsg, "One or more required arguments missing for --%s!\n\nUSAGE is:\n--%s\n\t--%s%c<%s>",
			p_dump.getName().c_str(), p_dump.getName().c_str(),
			p_vol_in_path.getName().c_str(), cmd.getDelimiter(), "string");
		throw iom::exception(errMsg);
	}
		
	//restoring parameters
	if(p_enable_restore.isSet() && !p_restoring_direction.isSet())
	{
		sprintf(errMsg, "One or more required arguments missing for --%s!\n\nUSAGE is:\n--%s --%s%c<%s>",
			p_enable_restore.getName().c_str(), p_enable_restore.getName().c_str(),
			p_restoring_direction.getName().c_str(), cmd.getDelimiter(), "string");
		throw iom::exception(errMsg);
	}

	//execution times parameters
	if(p_save_execution_times.isSet() && !p_execution_times_file.isSet())
	{
		sprintf(errMsg, "One or more required arguments missing for --%s!\n\nUSAGE is:\n--%s --%s%c<%s>",
			p_save_execution_times.getName().c_str(), p_save_execution_times.getName().c_str(),
			p_execution_times_file.getName().c_str(), cmd.getDelimiter(), "string");
		throw iom::exception(errMsg);
	}

	//checking that at least one operation has been selected
	if(!(p_test.isSet() || p_makedirs.isSet() || p_metadata.isSet() || p_import.isSet() || p_computedisplacements.isSet() || p_projdisplacements.isSet() || 
		p_thresholdisplacements.isSet() || p_placetiles.isSet() || p_merge.isSet() || p_stitch.isSet() || p_dump.isSet() || p_pluginsinfo.isSet()))
		throw iom::exception("No operation selected. See --help for usage.");

	//importing parameters
	vm::VOLUME_INPUT_FORMAT_PLUGIN = p_vol_in_plugin.getValue();
	vm::VOLUME_OUTPUT_FORMAT_PLUGIN = p_vol_out_plugin.getValue();

	// 2014-09-29. Alessandro. @ADDED automated selection of IO plugin if not provided.
	if(p_im_in_plugin.getValue().compare("auto") == 0)
	{
		// try to automatically load the volume format from the xml, if it has been provided
		if(p_proj_in_path.getValue().compare("null") != 0)
		{
			try{vm::VOLUME_INPUT_FORMAT_PLUGIN = vm::VirtualVolume::getVolumeFormat(p_proj_in_path.getValue());}
			catch(...){}
		}

		if(vm::VOLUME_INPUT_FORMAT_PLUGIN.compare(vm::StackedVolume::id) == 0)
			iom::IMIN_PLUGIN = "tiff2D";
		else if(vm::VOLUME_INPUT_FORMAT_PLUGIN.compare(vm::BlockVolume::id) == 0)
			iom::IMIN_PLUGIN = "tiff3D";
	}
	else
		iom::IMIN_PLUGIN = p_im_in_plugin.getValue();
	iom::IMIN_PLUGIN_PARAMS = p_im_in_plugin_params.getValue();

	// 2014-09-29. Alessandro. @ADDED automated selection of IO plugin if not provided.
	if(p_im_out_plugin.getValue().compare("auto") == 0)
	{
		if(p_vol_out_plugin.getValue().compare(vm::StackedVolume::id) == 0)
			iom::IMOUT_PLUGIN = "tiff2D";
		else if(p_vol_out_plugin.getValue().compare(vm::BlockVolume::id) == 0)
			iom::IMOUT_PLUGIN = "tiff3D";
	}
	else
		iom::IMOUT_PLUGIN = p_im_in_plugin.getValue();
	iom::IMOUT_PLUGIN_PARAMS = p_im_out_plugin_params.getValue();

	vm::SPARSE_DATA = p_sparse_data.getValue();
    this->metaData = p_metadata.getValue();
    this->makeDirs = p_makedirs.getValue();
    this->rescanFiles = p_rescan.getValue();
	this->pluginsinfo = p_pluginsinfo.getValue();
	this->dumpMData = p_dump.getValue();
	this->parallel = p_parallel.getValue();
	this->isotropic = p_isotropic.getValue();
	this->test = p_test.getValue();
	this->stitch = p_stitch.getValue();
	this->import = p_import.getValue();
	this->computedisplacements = p_computedisplacements.getValue();
	this->projdisplacements = p_projdisplacements.getValue();
	this->thresholddisplacements = p_thresholdisplacements.getValue();
	this->placetiles = p_placetiles.getValue();
	this->mergetiles = p_merge.getValue();
	this->volume_load_path = p_vol_in_path.getValue();
	this->volume_save_path = p_vol_out_path.getValue();
	this->projfile_load_path = p_proj_in_path.getValue();
	this->projfile_save_path = p_proj_out_path.getValue();
	this->errlogfile_path = p_errlogfile_path.getValue();
	this->reference_system.first  = vm::axis(vm::str2axis(p_refsys_1.getValue()));
	this->reference_system.second = vm::axis(vm::str2axis(p_refsys_2.getValue()));
	this->reference_system.third  = vm::axis(vm::str2axis(p_refsys_3.getValue()));
	this->VXL_1 = p_vxl_1.getValue();
	this->VXL_2 = p_vxl_2.getValue();
	this->VXL_3 = p_vxl_3.getValue();
	this->start_stack_row = p_start_stack_row.getValue();
	this->end_stack_row = p_end_stack_row.getValue();
	this->start_stack_col = p_start_stack_col.getValue();
	this->end_stack_col = p_end_stack_col.getValue();
	this->overlap_V = p_overlap_V.getValue();
	this->overlap_H = p_overlap_H.getValue();
	this->search_radius_V = p_search_radius_V.getValue();
	this->search_radius_H = p_search_radius_H.getValue();
	this->search_radius_D = p_search_radius_D.getValue();
	this->subvol_dim_D = p_subvol_dim_D.getValue();
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
	this->exclude_nonstitchables = p_exclude_nonstitchables.getValue();
	this->D0 = p_D0.getValue();
	this->D1 = p_D1.getValue();
	this->enable_restore = p_enable_restore.getValue();
	this->restoring_direction = vm::str2axis(p_restoring_direction.getValue());
	this->save_execution_times = p_save_execution_times.getValue();
	this->execution_times_filename = p_execution_times_file.getValue();
	this->show_progress_bar = !p_hide_progress_bar.getValue();
	this->img_format = p_im_out_format.getValue();
	this->img_depth = im_out_depth.getValue();
	volumemanager::IMG_FILTER_REGEX = p_im_in_regex.getValue();
	if(p_im_in_channel.getValue().compare("all") == 0)
		iomanager::CHANS = iomanager::ALL;
	else if(p_im_in_channel.getValue().compare("R") == 0)
		iomanager::CHANS = iomanager::R;
	else if(p_im_in_channel.getValue().compare("G") == 0)
		iomanager::CHANS = iomanager::G;
	else if(p_im_in_channel.getValue().compare("B") == 0)
		iomanager::CHANS = iomanager::B;
	// 2017-03-27. Giulio. Added full multi-channel support
	else if ( isNumber(p_im_in_channel.getValue()) ) {
		// check if the input plugin support more than three channels
		bool flag = false;
		std::string plugin_type;
		try{
			plugin_type = "3D image-based I/O plugin";
			flag = iom::IOPluginFactory::getPlugin3D(iom::IMIN_PLUGIN)->desc().find(plugin_type) != std::string::npos;
		}
		catch (...) {
			plugin_type = "2D image-based I/O plugin";
			flag = iom::IOPluginFactory::getPlugin2D(iom::IMIN_PLUGIN)->desc().find(plugin_type) != std::string::npos;
		}
		if ( !flag )
 			throw iom::exception(iomanager::strprintf("cannot determine the type of the input plugin"), __iom__current__function__);
		if ( (plugin_type.compare("3D image-based I/O plugin") == 0) ?
								iomanager::IOPluginFactory::getPlugin3D(iom::IMIN_PLUGIN)->isChansInterleaved() :
								iomanager::IOPluginFactory::getPlugin2D(iom::IMIN_PLUGIN)->isChansInterleaved() )
  			throw iom::exception(iomanager::strprintf("pulgins with interleaved channels do not allow to specify a channel number: use R, G, or B"), __iom__current__function__);
		else
			// channels are not interleaved, more than three channels are allowed
			iomanager::CHANS_no = atoi(p_im_in_channel.getValue().c_str());
		// channels are specified by a number: variable 'iomanager::CHANS' must be set to invalid value 
		iomanager::CHANS = iomanager::NONE;

	}
	else
		throw iom::exception(iomanager::strprintf("Invalid argument \"%s\" for parameter --%s! Allowed values are {\"all\",\"R\",\"G\",\"B\"}", p_im_in_channel.getValue().c_str(), p_im_in_channel.getName().c_str()).c_str());

	//the [algorithm] parameter is multi-arguments
	vector<string> algorithms = p_algo.getValue();
	for(int i = 0; i < algorithms.size(); i++) {
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
		else if(algorithms[i].compare(S_NO_BLENDING_NAME) == 0)
			this->tm_blending = S_NO_BLENDING;
		else if(algorithms[i].compare(S_SINUSOIDAL_BLENDING_NAME) == 0)
			this->tm_blending = S_SINUSOIDAL_BLENDING;
		else if(algorithms[i].compare(S_SHOW_STACK_MARGIN_NAME) == 0)
			this->tm_blending = S_SHOW_STACK_MARGIN;
		else
		{
			sprintf(errMsg, "Invalid argument \"%s\" for parameter --%s! Allowed values are:\n-\"%s\"\n-\"%s\"\n-\"%s\"\n-\"%s\"\n-\"%s\"\n-\"%s\"\n",	algorithms[i].c_str(), p_algo.getName().c_str(), S_NCC_NAME, S_FATPM_SP_TREE_NAME,S_FATPM_SCAN_V_NAME,S_FATPM_SCAN_H_NAME, S_NO_BLENDING_NAME, S_SINUSOIDAL_BLENDING_NAME);
			throw iom::exception(errMsg);
		}
	}
	if ( p_halving_method.getValue() == "mean" )
		this->halving_method = HALVE_BY_MEAN;
	else if ( p_halving_method.getValue() == "max" )
		this->halving_method = HALVE_BY_MAX;

	this->libtiff_uncompressed = p_libtiff_uncompressed.getValue();
	this->libtiff_bigtiff = p_libtiff_bigtiff.getValue();
	this->libtiff_rowsPerStrip = p_libtiff_rowsperstrip.getValue();

	if(p_parallel.isSet() && p_computedisplacements.isSet() ) { // set only if compute displacements is performed in parallel
		this->disable_last_row = p_disable_last_row.getValue();
		this->disable_last_col = p_disable_last_col.getValue();
	}

}

//checks parameters correctness
void TeraStitcherCLI::checkParams() throw (iom::exception)
{
	//parameters check should be done here.
	//We trust in current tool functions checks.
	//print();

	//checking that other operations are disabled when test mode is enabled
	if(test && (stitch || import || computedisplacements || projdisplacements || thresholddisplacements || placetiles || mergetiles))
		throw iom::exception("Test has been selected! No other operations can be performed.");
}

//returns help text
string TeraStitcherCLI::getHelpText()
{
	stringstream helptext;
	helptext << "TeraStitcher " << terastitcher::version << (ts::qtversion.empty() ? "\n" : " (with Qt " + ts::qtversion + ")\n");
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
void TeraStitcherCLI::print()
{
	printf("\n\n");
	printf("test = \t\t%s\n", test ? "ENABLED" : "disabled");
	printf("stitch = \t\t%s\n", stitch ? "ENABLED" : "disabled");
	printf("import = \t\t%s\n", import ? "ENABLED" : "disabled");
	printf("computedisplacements = \t%s\n", computedisplacements ? "ENABLED" : "disabled");
	printf("projdisplacements = \t%s\n", projdisplacements ? "ENABLED" : "disabled");
	printf("thresholdisplacements = %s\n", thresholddisplacements ? "ENABLED" : "disabled");
	printf("placetiles = \t\t%s\n", placetiles ? "ENABLED" : "disabled");
	printf("mergetiles = \t\t%s\n", mergetiles ? "ENABLED" : "disabled");
	printf("volume_load_path = \t%s\n", volume_load_path.c_str());
	printf("volume_save_path = \t%s\n", volume_save_path.c_str());
	printf("projfile_load_path = \t%s\n", projfile_load_path.c_str());
	printf("projfile_save_path = \t%s\n", projfile_save_path.c_str());
	printf("reference_system = \t{%d,%d,%d}\n", reference_system.first, reference_system.second, reference_system.third);
	printf("VXL = \t\t\t%.1f x %.1f x %.1f\n", VXL_1, VXL_2, VXL_3);
	printf("pd_algo = \t\t%d\n", pd_algo);
	printf("start_stack_row = \t%d\n", start_stack_row);
	printf("end_stack_row = \t%d\n", end_stack_row);
	printf("start_stack_col = \t%d\n", start_stack_col);
	printf("end_stack_col = \t%d\n", end_stack_col);
	printf("overlap_V = \t\t%d\n", overlap_V);
	printf("overlap_H = \t\t%d\n", overlap_H);
	printf("search radius = \t%d(V) x %d(H) x %d(D)\n", search_radius_V, search_radius_H, search_radius_D);
	printf("subvol_dim_D = \t\t%d\n", subvol_dim_D);
	printf("reliability_threshold = %.2f\n", reliability_threshold);
	printf("tp_algo = \t\t%d\n", tp_algo);
	printf("slice dimensions = \t%d(V) x %d(H)\n", slice_height, slice_width);
	printf("resolutions = \t\t");
	for(int i=0; i<=S_MAX_MULTIRES; i++)
		if(resolutions[i])
			printf("%d ", i);
	printf("\n");
	printf("exclude_nonstitchables =%s\n", exclude_nonstitchables ? "ENABLED" : "disabled");
	printf("D0 = \t\t\t%d\n", D0);
	printf("D1 = \t\t\t%d\n", D1);
	printf("tm_blending = \t\t%d\n", tm_blending);
	printf("enable_restore = \t%s\n", enable_restore ? "ENABLED" : "disabled");
	printf("restoring_direction = \t%d\n", restoring_direction);
}
