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
* 2017-09-11. Giulio.     @ADDED parameter controlloing the compression algorithm to be used with HDf5 files
* 2017-05-25. Giulio.     @ADDED parameters for lossy compression based on rescaling
* 2017-04-07  Giulio.     @ADDED the opssibility to specify a subset of channels to be converted
* 2016-09-13. Giulio.     @ADDED flag for channel subdirectory name (single channel to tiled 4D format only)
* 2016-09-13. Giulio.     @FIXED initialized flag for time series
* 2016-09-13. Giulio.     @ADDED flag for time series
* 2016-09-04. Giulio.     @ADDED the options for setting the configuration of the LibTIFF library
* 2016-06-18  Giulio.     @ADDED option for downsampling the reading of data
* 2016-05-11  Giulio.     @ADDED other formats in help messages
* 2016-05-11  Giulio.     @ADDED plugins command line options and initialization
* 2016-04-13  Giulio.     @ADDED options for parallelizing teraconverter
* 2015-02-10. Giulio.     @CHANGED changed how the resolutions parameter works (resolutions can be chosen individually)
*/

#include "TemplateCLI.h"
#include "IM_config.h"
#include "StackedVolume.h"
#include "../../stitcher/S_config.h"
#include "config.h"
#include "../../volumemanager/volumemanager.config.h"
#include "IOPluginAPI.h"

#include <CmdLine.h>
#include <sstream>


TemplateCLI::TemplateCLI(void)
{
	// we use TCLAP default values to initialize most parameters (see readParams)

	// initialize other parameters that are not provided in the command line
	this->tm_blending = S_SINUSOIDAL_BLENDING;
}

//reads options and parameters from command line
void TemplateCLI::readParams(int argc, char** argv) throw (iom::exception)
{
	//command line object definition
	TCLAP::CmdLine cmd(getHelpText(), '=', terastitcher::teraconverter_version);
		/**
		 * Command line constructor. Defines how the arguments will be
		 * parsed.
		 * \param message - The message to be used in the usage
		 * output.
		 * \param delimiter - The character that is used to separate
		 * the argument flag/name from the value.  Defaults to ' ' (space).
		 * \param version - The version number to be used in the
		 * --version switch.
		 * \param helpAndVersion - Whether or not to create the Help and
		 * Version switches. Defaults to true.
		 */


	//argument objects definitions
	//TCLAP::SwitchArg p_highest_resolution("a","all","Generate also the highest resolution.",false); 
        /**
		 * SwitchArg constructor.
		 * \param flag - The one character flag that identifies this
		 * argument on the command line.
		 * \param name - A one word name for the argument.  Can be
		 * used as a long flag on the command line.
		 * \param desc - A description of what the argument is for or
		 * does.
		 * \param def - The default value for this Switch. 
		 * \param v - An optional visitor.  You probably should not
		 * use this unless you have a very good reason.
		 */
	TCLAP::SwitchArg p_parallel("","parallel","Does not perform side-effect operations during the merge step. Use this flag when more merge steps are launched in parallel",false);
	TCLAP::SwitchArg p_isotropic("","isotropic","Generate lowest resolution with voxels as much isotropic as possible. Use this flag when the high resolution image has highy anistropic voxels",false);
	TCLAP::SwitchArg p_makedirs("","makedirs","Creates the mdata.bin file of the output volume.", false);
	TCLAP::SwitchArg p_metadata("","metadata","Creates the directory hierarchy.", false);

	TCLAP::SwitchArg p_timeseries("","timeseries","The input is a time series.", false);

	TCLAP::ValueArg<std::string> p_src_root_dir("s","src","Source file / root directory path.",true,"","string");
	TCLAP::ValueArg<std::string> p_dst_root_dir("d","dst","Destination root directory path.",false,"","string");
	TCLAP::ValueArg<std::string> p_mdata_fname("","mdata_fname","File containing general metadata of the image.",false,"","string");
	TCLAP::ValueArg<std::string> p_ch_dir("","ch_dir","subdirectory to store the channel (single channel in tiled 4D format only).",false,"","string");
	TCLAP::ValueArg<int> p_slice_depth("","depth","Slice depth.",false,-1,"unsigned");
	TCLAP::ValueArg<int> p_slice_height("","height","Slice height.",false,-1,"unsigned");
	TCLAP::ValueArg<int> p_slice_width("","width","Slice width.",false,-1,"unsigned");

	string temp = "Source format (\"" + 
		iim::RAW_FORMAT + "\"/\"" + 
		iim::SIMPLE_RAW_FORMAT + "\"/\"" + 
		//iim::STACKED_RAW_FORMAT + "\"/\"" + 
		iim::TILED_FORMAT + "\"/\"" + 
		iim::TILED_MC_FORMAT + "\"/\"" + 
		iim::TIF3D_FORMAT + "\"/\"" + 
		iim::SIMPLE_FORMAT + "\"/\"" + 
		iim::STACKED_FORMAT + "\"/\"" + 
		iim::TILED_TIF3D_FORMAT  + "\"/\"" +
		iim::TILED_MC_TIF3D_FORMAT  + "\"/\"" +
		iim::UNST_TIF3D_FORMAT  + "\"/\"" +
		iim::BDV_HDF5_FORMAT  + "\"/\"" +
		iim::IMS_HDF5_FORMAT  + "\"/\"" +
		iim::MAPPED_FORMAT  + "\")";
	TCLAP::ValueArg<string> p_src_format("","sfmt",temp.c_str(),true,"","string");

	string tempd = "Destination format (\"" + 
		iim::SIMPLE_RAW_FORMAT + "\"/\"" + 
		//iim::STACKED_RAW_FORMAT + "\"/\"" + 
		iim::TILED_FORMAT + "\"/\"" + 
		iim::TILED_MC_FORMAT + "\"/\"" + 
		iim::SIMPLE_FORMAT + "\"/\"" + 
		iim::STACKED_FORMAT + "\"/\"" + 
		iim::TILED_TIF3D_FORMAT  + "\"/\"" +
		iim::TILED_MC_TIF3D_FORMAT  + "\"/\"" +
		iim::BDV_HDF5_FORMAT  + "\"/\"" +
		iim::IMS_HDF5_FORMAT  + "\")";
	TCLAP::ValueArg<string> p_dst_format("","dfmt",tempd.c_str(),true,"","string");

	//TCLAP::ValueArg<int> p_n_resolutions("","res","Number of resolutions.",true,2,"unsigned");
	TCLAP::ValueArg<std::string> p_resolutions("","resolutions","Resolutions to be produced. Possible values are [[i]...] where i = 0,..,5 and 2^i is the subsampling factor.",false,"0","string");
	TCLAP::ValueArg<std::string> p_chanlist("","clist","Subset of channel to be converted (default: all).",false,"","string");
	TCLAP::ValueArg<string> p_halving_method("","halve","Halving method (mean/max, default: mean).",false,"mean","unsigned");

	TCLAP::ValueArg<string> p_outFmt("f","outFmt","Voxel format (graylevel or RGB (default)/intensity.",false,"RGB","string");
	//TCLAP::ValueArg<std::string> p_outFmt("f","outFmt","Output format (Tiff2DStck/Vaa3DRaw/Tiff3D/Vaa3DRawMC/Tiff3DMC/Fiji_HDF5, default: Tiff2DStck).",false,"Tiff2DStck","string");

	TCLAP::ValueArg<std::string> p_infofile_path("","info","File path of the info file to be saved.",false,"","string");
 	TCLAP::SwitchArg p_hide_progress_bar("","noprogressbar","Disables progress bar and estimated time remaining", false);
	TCLAP::SwitchArg p_verbose("","verbose","set verbosity to maximum level (to be activated ONLY for debugging)");
	TCLAP::ValueArg<int> p_V0("","V0","First V vertex (included).",false,-1,"unsigned");
	TCLAP::ValueArg<int> p_V1("","V1","Last V vertex (excluded).",false,-1,"unsigned");
	TCLAP::ValueArg<int> p_H0("","H0","First H vertex (included).",false,-1,"unsigned");
	TCLAP::ValueArg<int> p_H1("","H1","Last H vertex (excluded).",false,-1,"unsigned");
	TCLAP::ValueArg<int> p_D0("","D0","First D vertex (included).",false,-1,"unsigned");
	TCLAP::ValueArg<int> p_D1("","D1","Last D vertex (excluded).",false,-1,"unsigned");

	TCLAP::MultiArg<std::string> p_algo("","algorithm","Forces the use of the given algorithm.",false,"string");
	TCLAP::SwitchArg p_pluginsinfo("p","pluginsinfo","Display plugins informations",false);
	//TCLAP::ValueArg<std::string> p_vol_out_plugin("","volout_plugin",iom::strprintf("Plugin that manages the output volume format/organization. Available plugins are: {%s}. Default is \"%s\".", iom::IOPluginFactory::registeredPlugins().c_str(), vm::VOLUME_OUTPUT_FORMAT_PLUGIN.c_str()),false,vm::VOLUME_OUTPUT_FORMAT_PLUGIN,"string");
	TCLAP::ValueArg<std::string> p_im_in_plugin("","imin_plugin",iom::strprintf("Plugin that manages the input image format. Available plugins are: {%s}. Default is \"auto\".", iom::IOPluginFactory::registeredPlugins().c_str()), false, "auto","string");
	TCLAP::ValueArg<std::string> p_im_in_plugin_params("","imin_plugin_params","A series of parameters \"param1=val,param2=val,...\" to configure the input image plugin (see --pluginsinfo for the list of accepted parameters)", false, "","string");
	TCLAP::ValueArg<std::string> p_im_out_plugin("","imout_plugin",iom::strprintf("Plugin that manages the output image format. Available plugins are: {%s}. Default is \"auto\".", iom::IOPluginFactory::registeredPlugins().c_str()), false, "auto","string");
	TCLAP::ValueArg<std::string> p_im_out_plugin_params("","imout_plugin_params","A series of parameters \"param1=val,param2=val,...\" to configure the output image plugin (see --pluginsinfo for the list of accepted parameters)", false, "","string");

	TCLAP::ValueArg<int> p_dwnsmplngFactor("","dsfactor","Dowsampling factor to be used to read the source volume (only for serie of 2D slices).",false,1,"unsigned");

	TCLAP::SwitchArg p_libtiff_uncompressed("","libtiff_uncompress","Configure libtiff library to not compress output files (default: compression enabled).", false);
	TCLAP::SwitchArg p_libtiff_bigtiff("","libtiff_bigtiff","Forces the creation of BigTiff files (default: BigTiff disabled).", false);
	TCLAP::ValueArg<int> p_libtiff_rowsperstrip("","libtiff_rowsperstrip","Configure libtiff library to pack n rows per strip when compression is enabled (default: 1 row per strip).",false,1,"integer");

	TCLAP::ValueArg<int> p_rescale_nbits("","rescale","Applies a lossy ccompression based on rescaling the values to multiples of 2^n (default: n=0).",false,0,"integer");

	TCLAP::ValueArg<std::string> p_compress_params("","compress_params","ID and list of parameters of HDF5 registerd compression algorithms given in the form: \"ID:param1:param2:...\" where ID is an integer identfying the compression algorithm (see https://support.hdfgroup.org/services/filters.html for IDs of registered compression algorithms)", false, "","string");

	//argument objects must be inserted using LIFO policy (last inserted, first shown)
	cmd.add(p_rescale_nbits);

	cmd.add(p_compress_params);
	cmd.add(p_libtiff_rowsperstrip);
	cmd.add(p_libtiff_bigtiff);
	cmd.add(p_libtiff_uncompressed);

	cmd.add(p_timeseries);

	cmd.add(p_dwnsmplngFactor);
	cmd.add(p_im_out_plugin_params);
	cmd.add(p_im_out_plugin);
	cmd.add(p_im_in_plugin_params);
	cmd.add(p_im_in_plugin);
	//cmd.add(p_vol_out_plugin);
	cmd.add(p_pluginsinfo);
	cmd.add(p_algo);

	cmd.add(p_metadata);
    cmd.add(p_makedirs);
	cmd.add(p_parallel);
	cmd.add(p_isotropic);

	cmd.add(p_D1);
	cmd.add(p_D0);
	cmd.add(p_H1);
	cmd.add(p_H0);
	cmd.add(p_V1);
	cmd.add(p_V0);

	cmd.add(p_verbose);
	cmd.add(p_hide_progress_bar);
	cmd.add(p_infofile_path);
	cmd.add(p_outFmt);
	cmd.add(p_halving_method);
	//cmd.add(p_highest_resolution);
	//cmd.add(p_n_resolutions);
	cmd.add(p_chanlist);
	cmd.add(p_resolutions);
	cmd.add(p_dst_format);
	cmd.add(p_src_format);
	cmd.add(p_slice_width);
	cmd.add(p_slice_height);
	cmd.add(p_slice_depth);
	cmd.add(p_ch_dir);
	cmd.add(p_mdata_fname);
	cmd.add(p_dst_root_dir);
	cmd.add(p_src_root_dir);

	// Parse the argv array and catch <TCLAP> exceptions, which are translated into <iim::IOException> exceptions
	char errMsg[S_STATIC_STRINGS_SIZE];
	try{ cmd.parse( argc, argv ); } 
	catch (TCLAP::ArgException &e)
	{ 
		sprintf(errMsg, "%s for arg %s\n", e.error().c_str(), e.argId().c_str());
		throw iom::exception(errMsg);
	}

	int i;

	/* Checking parameter consistency */
	if ( p_infofile_path.getValue() == "" && p_dst_root_dir.getValue() == "" ) {
		sprintf(errMsg, "Missing destination directory (option -d)");
		throw iom::exception(errMsg);
	}

	if ( p_timeseries.isSet() && (p_parallel.isSet() || p_makedirs.isSet()) ) {
		sprintf(errMsg, "Parallel options are not allowed for time series");
		throw iom::exception(errMsg);
	}

	if ( p_src_format.getValue() != iim::RAW_FORMAT  && 
		 p_src_format.getValue() != iim::SIMPLE_RAW_FORMAT  && 
		 //p_src_format.getValue() != iim::STACKED_RAW_FORMAT && 
		 p_src_format.getValue() != iim::TILED_FORMAT  && 
		 p_src_format.getValue() != iim::TILED_MC_FORMAT &&
		 p_src_format.getValue() != iim::TIF3D_FORMAT  && 
		 p_src_format.getValue() != iim::SIMPLE_FORMAT  && 
		 p_src_format.getValue() != iim::STACKED_FORMAT && 
		 p_src_format.getValue() != iim::TILED_TIF3D_FORMAT  && 
		 p_src_format.getValue() != iim::TILED_MC_TIF3D_FORMAT  && 
		 p_src_format.getValue() != iim::UNST_TIF3D_FORMAT  && 
		 p_src_format.getValue() != iim::BDV_HDF5_FORMAT  && 
		 p_src_format.getValue() != iim::IMS_HDF5_FORMAT  && 
		 p_src_format.getValue() != iim::MAPPED_FORMAT ) {
		temp = "Unknown source format!\nAllowed formats are:\n\t\"" + 
			iim::RAW_FORMAT + "\"/\"" + 
			iim::SIMPLE_RAW_FORMAT + "\"/\"" + 
			//iim::STACKED_RAW_FORMAT + "\"/\"" + 
			iim::TILED_FORMAT + "\"/\"" + 
			iim::TILED_MC_FORMAT + "\"/\"" + 
			iim::TIF3D_FORMAT + "\"/\"" + 
			iim::SIMPLE_FORMAT + "\"/\"" + 
			iim::STACKED_FORMAT + "\"/\"" + 
			iim::TILED_TIF3D_FORMAT  + "\"/\"" +
			iim::TILED_MC_TIF3D_FORMAT  + "\"/\"" +
			iim::UNST_TIF3D_FORMAT  + "\"/\"" +
			iim::BDV_HDF5_FORMAT  + "\"/\"" +
			iim::IMS_HDF5_FORMAT  + "\"/\"" +
			iim::MAPPED_FORMAT  + "\"";
		//sprintf(errMsg, "Unknown source format!\nAllowed formats are:\n\tStacked / Simple / SimpeRaw / Raw / Tiled / TiledMC");
		sprintf(errMsg, "%s", temp.c_str());
		throw iom::exception(errMsg);
	}
	if ( p_dst_format.getValue() != iim::SIMPLE_RAW_FORMAT && 
		 //p_dst_format.getValue() != iim::STACKED_RAW_FORMAT && 
		 p_dst_format.getValue() != iim::TILED_FORMAT  && 
		 p_dst_format.getValue() != iim::TILED_MC_FORMAT &&
		 p_dst_format.getValue() != iim::SIMPLE_FORMAT  && 
		 p_dst_format.getValue() != iim::STACKED_FORMAT && 
		 p_dst_format.getValue() != iim::TILED_TIF3D_FORMAT  && 
		 p_dst_format.getValue() != iim::TILED_MC_TIF3D_FORMAT  && 
		 p_dst_format.getValue() != iim::BDV_HDF5_FORMAT &&
		 p_dst_format.getValue() != iim::IMS_HDF5_FORMAT ) {
		tempd = "Unknown destination format!\nAllowed formats are:\n\t\"" + 
			iim::SIMPLE_RAW_FORMAT + "\"/\"" + 
			iim::STACKED_RAW_FORMAT + "\"/\"" + 
			iim::TILED_FORMAT + "\"/\"" + 
			iim::TILED_MC_FORMAT + "\"/\"" + 
			iim::SIMPLE_FORMAT + "\"/\"" + 
			iim::STACKED_FORMAT + "\"/\"" + 
			iim::TILED_TIF3D_FORMAT  + "\"/\"" +
			iim::TILED_MC_TIF3D_FORMAT  + "\"/\"" +
			iim::BDV_HDF5_FORMAT  + "\"/\"" +
			iim::IMS_HDF5_FORMAT  + "\"";
		//sprintf(errMsg, "Unknown output format!\nAllowed formats are:\n\tTiff2DStck / Vaa3DRaw / Vaa3DRawMC / Tiff3D / Tiff3DMC / Fiji_HDF5");
		sprintf(errMsg, "%s", tempd.c_str());
		throw iom::exception(errMsg);
	}

	//if ( p_ch_dir.getValue() != "" && p_dst_format.getValue() == iim::TILED_MC_TIF3D_FORMAT && (p_makedirs.isSet() || p_metadata.isSet() || p_parallel.isSet())) {
	//	sprintf(errMsg, "A subdirectory name for tiled 4D format cannot be provided if options makedirs, parallel or metadata options");
	//	throw iom::exception(errMsg);
	//}

	if ( p_outFmt.getValue() != "intensity" && 
		 p_outFmt.getValue() != "graylevel" && 
		 p_outFmt.getValue() != "RGB" ) {
		sprintf(errMsg, "Unknown destination format!\nAllowed formats are:\n\tgraylevel / RGB / intensity");
		throw iom::exception(errMsg);
	}
	if ( p_halving_method.getValue() != "mean" && 
		 p_halving_method.getValue() != "max" ) {
		sprintf(errMsg, "Unknown halving method!\nAllowed methods are:\n\tmean / max");
		throw iom::exception(errMsg);
	}

	if ( (p_dst_format.getValue() == iim::BDV_HDF5_FORMAT || p_dst_format.getValue() == iim::IMS_HDF5_FORMAT) && (p_makedirs.isSet() || p_metadata.isSet() || p_parallel.isSet()) ) {
		sprintf(errMsg, "makedirs, parallel, metadata options are not allowed with BDV_HDF5 or IMS_HDF5 output formats");
		throw iom::exception(errMsg);
	}

	if ( (p_dst_format.getValue() == iim::IMS_HDF5_FORMAT) && (p_mdata_fname.getValue() == "") ) {
		sprintf(errMsg, "With IMS_HDF5 output formats the metadata file name must be specified or explicitly set to 'null'");
		throw iom::exception(errMsg);
	}

	if ( (p_dst_format.getValue() == iim::SIMPLE_RAW_FORMAT || p_dst_format.getValue() == iim::SIMPLE_FORMAT) && 
						p_dwnsmplngFactor.getValue()!=1 && (p_V0.isSet() || p_V1.isSet() || p_H0.isSet() || p_H1.isSet()) ) {
		sprintf(errMsg, "V0, V1, H0, H1 are not allowed when destination formats %s or %s are chosen with a downsampling factor different from 1",
																		iim::SIMPLE_RAW_FORMAT.c_str(),iim::SIMPLE_RAW_FORMAT.c_str());
		throw iom::exception(errMsg);
	}

	//checking that parallel-related options have been correctly selected
	if(p_makedirs.isSet() && (p_metadata.isSet() || p_parallel.isSet()) )
		throw iom::exception("parallel or metadata options cannot be set with makedirs option. See --help for usage.");

	if(p_parallel.isSet() && p_metadata.isSet() )
		throw iom::exception("metadata option cannot be set with parallel option. See --help for usage.");

	//importing parameters not set yet
	this->src_root_dir  = p_src_root_dir.getValue();
	this->dst_root_dir  = p_dst_root_dir.getValue();
	this->mdata_fname   = p_mdata_fname.getValue();
	this->ch_dir        = p_ch_dir.getValue();
	this->slice_depth   = p_slice_depth.getValue();
	this->slice_height  = p_slice_height.getValue();
	this->slice_width   = p_slice_width.getValue();
	this->src_format    = p_src_format.getValue();
	this->dst_format    = p_dst_format.getValue();
    this->metaData      = p_metadata.getValue();
    this->makeDirs      = p_makedirs.getValue();
	this->parallel      = p_parallel.getValue();
	this->isotropic     = p_isotropic.getValue();
	this->timeseries    = p_timeseries.getValue();
	this->infofile_path = p_infofile_path.getValue();

	//the [algorithm] parameter is multi-arguments
	vector<string> algorithms = p_algo.getValue();
	for(int i = 0; i < algorithms.size(); i++) {
		if(algorithms[i].compare(S_NO_BLENDING_NAME) == 0)
			this->tm_blending = S_NO_BLENDING;
		else if(algorithms[i].compare(S_SINUSOIDAL_BLENDING_NAME) == 0)
			this->tm_blending = S_SINUSOIDAL_BLENDING;
		else if(algorithms[i].compare(S_SHOW_STACK_MARGIN_NAME) == 0)
			this->tm_blending = S_SHOW_STACK_MARGIN;
		else
		{
			sprintf(errMsg, "Invalid argument \"%s\" for parameter --%s! Allowed values are:\n-\"%s\"\n-\"%s\"\n",	algorithms[i].c_str(), p_algo.getName().c_str(), S_NO_BLENDING_NAME, S_SINUSOIDAL_BLENDING_NAME);
			throw iom::exception(errMsg);
		}
	}

	this->pluginsinfo = p_pluginsinfo.getValue();

	// 2014-09-29. Alessandro. @ADDED automated selection of IO plugin if not provided.
 	if(p_im_in_plugin.getValue().compare("auto") == 0)
 	{
 		if(p_src_format.getValue().compare(iim::STACKED_FORMAT) == 0)
 			iom::IMIN_PLUGIN = "tiff2D";
 		else if(p_src_format.getValue().compare(iim::TILED_TIF3D_FORMAT) == 0 || 
				p_src_format.getValue().compare(iim::TILED_MC_TIF3D_FORMAT) == 0 /* || p_src_format.getValue().compare(iim::UNST_TIF3D_FORMAT) == 0 */ )
 			iom::IMIN_PLUGIN = "tiff3D";
		else
 			; // already initialized to empty
 	}
 	else
 		iom::IMIN_PLUGIN = p_im_in_plugin.getValue();
 	iom::IMIN_PLUGIN_PARAMS = p_im_in_plugin_params.getValue();

	// 2014-09-29. Alessandro. @ADDED automated selection of IO plugin if not provided.
 	if(p_im_out_plugin.getValue().compare("auto") == 0)
 	{
 		if(p_dst_format.getValue().compare(iim::STACKED_FORMAT) == 0)
 			iom::IMOUT_PLUGIN = "tiff2D";
 		else if(p_dst_format.getValue().compare(iim::TILED_TIF3D_FORMAT) == 0 || p_dst_format.getValue().compare(iim::TILED_MC_TIF3D_FORMAT) == 0)
 			iom::IMOUT_PLUGIN = "tiff3D";
		else
 			; // already initialized to empty
 	}
 	else
 		iom::IMOUT_PLUGIN = p_im_in_plugin.getValue();
 	iom::IMOUT_PLUGIN_PARAMS = p_im_out_plugin_params.getValue();

	//this->resolutions[0] = p_highest_resolution.getValue() ? 1 : 0;
	//for ( i=1; i<p_n_resolutions.getValue(); i++ )
	//	this->resolutions[i] = 1;
	//for ( ; i<S_MAX_MULTIRES; i++ )
	//	this->resolutions[i] = 0;
	for(i=0; i< S_MAX_MULTIRES; i++) {
		stringstream buf;
		buf << i;
		this->resolutions[i] = p_resolutions.getValue().find(buf.str()) != std::string::npos;
	}

	this->chanlist = p_chanlist.getValue();

	if ( p_halving_method.getValue() == "mean" )
		this->halving_method = HALVE_BY_MEAN;
	else if ( p_halving_method.getValue() == "max" )
		this->halving_method = HALVE_BY_MAX;

	this->outFmt = p_outFmt.getValue();
	this->show_progress_bar = !p_hide_progress_bar.getValue();

	this->V0  = p_V0.getValue();
	this->V1  = p_V1.getValue();
	this->H0  = p_H0.getValue();
	this->H1  = p_H1.getValue();
	this->D0  = p_D0.getValue();
	this->D1  = p_D1.getValue();

	if ( p_dwnsmplngFactor.getValue() > 1 ) {
		if ( p_src_format.getValue() == iim::SIMPLE_FORMAT || p_src_format.getValue() == iim::SIMPLE_RAW_FORMAT )
			this->downsamplingFactor = p_dwnsmplngFactor.getValue();
		else {
			sprintf(errMsg, "Invalid downsampling factor %d for format \"%s\"\n", p_dwnsmplngFactor.getValue(),p_src_format.getValue().c_str());
			throw iom::exception(errMsg);
		}
	}
	else
		this->downsamplingFactor = 1;

	this->libtiff_uncompressed = p_libtiff_uncompressed.getValue();
	this->libtiff_bigtiff = p_libtiff_bigtiff.getValue();
	this->libtiff_rowsPerStrip = p_libtiff_rowsperstrip.getValue();

	if ( p_dst_format.getValue() == iim::IMS_HDF5_FORMAT )
		this->compress_params = p_compress_params.getValue();
	else
		this->compress_params = "";

	this->rescale_nbits = p_rescale_nbits.getValue();

	if(p_verbose.getValue())
	{
		terastitcher::DEBUG = terastitcher::LEV_MAX;
		iim::DEBUG = iim::LEV_MAX;
		iom::DEBUG = iom::LEV_MAX;
	}
}

//checks parameters correctness
void TemplateCLI::checkParams() throw (iom::exception)
{
	//parameters check should be done here.
	//We trust in current tool functions checks.
	//print();

	;
}

//returns help text
string TemplateCLI::getHelpText()
{
	stringstream helptext;


    helptext << "BIConverter " << "1.0.2" << "\n";
    helptext << "  developed at Allen Institute for Brain Science by:\n";
    helptext << "   -\tYang Yu (email: yangy@alleninstitute.org)                            ";
    helptext << "   -\tHanchuan Peng (email: hanchuanp@alleninstitute.org)              ";
    helptext << "   -\tAlessandro Bria (email: a.bria@unicas.it)                            ";
    helptext << "   -\tGiulio Iannello, Ph.D. (email: g.iannello@unicampus.it)              ";
	
    return helptext.str();

    //helptext << "TeraConverter v3.1.0\n";
    //return helptext.str();
}

//print all arguments
void TemplateCLI::print()
{
	printf("\n\n");
}
