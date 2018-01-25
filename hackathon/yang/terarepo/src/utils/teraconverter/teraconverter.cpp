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
* 2017-09-11.  Giulio.     @ADDED parameter controlloing the compression algorithm to be used with HDf5 files
* 2017-05-25.  Giulio.     @ADDED a lossy compression based on rescalinghas been introduced
* 2017-04-23.  Giulio.     @CHANGED the call to conversion routines has been moved to VolumeConverter.cpp (vcDriver auxiliary function)
* 2017-04-07.  Giulio.     @ADDED the opssibility to specify a subset of channels to be converted
* 2017-04-02.  Giulio.     @ADDED support for creation of BigTiff files
* 2017-04-01.  Giulio.     @CHANGED the calls to the routine that creates the directory structure in the case of 'simple' formats
* 2017-01-23.  Giulio.     @FIXED bugs of parallel execution in case 4D formats are specified
* 2016-10-09.  Giulio.     @ADDED option ch_dir to calls to 'generateTilesVaa3DRawMC'
* 2016-09-13.  Giulio.     @ADDED support for time series
* 2016-09-04.  Giulio.     @ADDED the setting of the configuration of the LibTIFF library
* 2016-06-18.  Giulio.     @ADDED the possibility to dowsampling the reading of data
* 2016-05-02.  Giulio.     @ADDED generation of an info log file
* 2016-05-02.  Giulio.     @ADDED routines for file management
* 2016-04-13.  Giulio.     @ADDED parallelization
*/

# include <stdio.h> 
# include <stdlib.h>

#include "VolumeConverter.h"
#include <CmdLine.h>
#include "TemplateCLI.h"
#include "iomanager.config.h"
#include "IOPluginAPI.h"

#include "Tiff3DMngr.h"


/*
 * This is the driver program for the library class VolumeConverter
 *
 * User has to pass on the command line the following parameters (run teranconverter -h for more details):
 * - directory or file name of the source image (the file name if the image 
 *   is stored in a single file, e.g. in V3D raw format)
 * - directory where to store the output image
 * - format of the source image
 * - format of the output image: "RGB" for pixel represented
 *   according to RGB format (default), "graylevel" for integer valued pixels in [0,255], 
 *   "intensity" for real valued pixels in [0,1]
 * - which resolutions have to be generated; only specified resolutions are generated
 *
 * Allowed suffixes for V3D raw 4D format are: .raw .RAW, .v3draw .V3DRAW
 */


//extracts the filename from the given path and stores it into <filename>
inline std::string getFileName(std::string const & path, bool save_ext = true){

	std::string filename = path;

	// Remove directory if present.
	// Do this before extension removal in case directory has a period character.
	const size_t last_slash_idx = filename.find_last_of("\\/");
	if (std::string::npos != last_slash_idx)
		filename.erase(0, last_slash_idx + 1);

	// Remove extension if present.
	if(!save_ext)
	{
		const size_t period_idx = filename.rfind('.');
		if (std::string::npos != period_idx)
			filename.erase(period_idx);
	}

	return filename;
}

//returns true if the given string <fullString> ends with <ending>
inline bool hasEnding (std::string const &fullString, std::string const &ending){
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
	} else {
		return false;
	}
}

//returns file extension, if any (otherwise returns "")
inline std::string getFileExtension(const std::string& FileName){
	if(FileName.find_last_of(".") != std::string::npos)
		return FileName.substr(FileName.find_last_of(".")+1);
	return "";
}

//returns the absolute path (if present) of the given filepath, i.e. its path without the file name
//e.g. "C:/somedir/somefile.ext" -> "C:/somedir"
inline std::string getAbsolutePath(const std::string& filepath){

	string file_path_string = filepath;
	string file_name;
	string ris;

	//loading file name from 'file_path_string' into 'file_name' by strtoking with "/" OR "\" character
	char * tmp;
	tmp = strtok (&file_path_string[0],"/\\");
	while (tmp)
	{
		file_name = tmp;
		tmp = strtok (0, "/\\");
	}

	//restoring content of file_path_string due to possible alterations done by strtok
	file_path_string = filepath;

	//loading file name substring index in 'file_path_string'
	int index_of_filename= (int) file_path_string.find(file_name);

	//extract substring that contains absolute path but the filename
	ris = file_path_string.substr(0,index_of_filename);
	return ris;
}

inline string fillPath(const std::string & incompletePath, string defPath, string defName, string defExt){
	
	string ext = getFileExtension(incompletePath);
	string name = getFileName(incompletePath, false);
	string path = getAbsolutePath(incompletePath);

	string fullpath;
	if(path.empty())
		fullpath += defPath;
	else
		fullpath += path;

	if(name.empty())
		fullpath += "/" + defName;
	else
		fullpath += "/" + name;

	if(ext.empty())
		fullpath += "." + defExt;
	else
		fullpath += "." + ext;

	return fullpath;
}



int main ( int argc, char *argv[] ) {

	//char err_msg[IM_STATIC_STRINGS_SIZE];
	try {

		//importing command-line arguments from <TeraStitcherCLI> object
		TemplateCLI cli;
		cli.readParams(argc, argv);
		cli.checkParams();
		

		// start timer
		double proctime = -TIME(0);

		// perform simple tasks
		if ( cli.infofile_path != "" ) {
			// volume info has been requested
			VolumeConverter vc;
			FILE *fout;
			vc.setSrcVolume(cli.src_root_dir.c_str(),cli.src_format.c_str(),cli.outFmt.c_str());
			string foutName = fillPath(cli.infofile_path,getAbsolutePath(cli.src_root_dir),"err_log_file", "txt");
			if ( (fout = fopen(foutName.c_str(),"w")) == 0 ) 
				throw iom::exception(iom::strprintf("teraconverter: cannot open info log file %s", foutName.c_str()).c_str());

			// output volume info
			fprintf(fout,"HEIGHT=%d\n",vc.getV1() - vc.getV0());
			fprintf(fout,"WIDTH=%d\n",vc.getH1() - vc.getH0());
			fprintf(fout,"DEPTH=%d\n",vc.getD1() - vc.getD0());
			fprintf(fout,"BYTESxCHAN=%d\n",vc.getVolume()->getBYTESxCHAN());
			fprintf(fout,"DIM_C=%d\n",vc.getVolume()->getDIM_C());

			fclose(fout);
			return EXIT_SUCCESS;
		}

		if(cli.pluginsinfo)
		{
			std::cout << iom::IOPluginFactory::pluginsInfo();
			return EXIT_SUCCESS;
		}

		vcDriver(
			(iim::VirtualVolume *) 0, // the volume has not been imported, the source path has to be passed
			cli.src_root_dir, 
			cli.dst_root_dir, 
			cli.src_format, 
			cli.dst_format, 
			0, // invalid value: imout_depth is set inside
			cli.resolutions, 
			cli.chanlist, 
			cli.ch_dir, 
			cli.mdata_fname, 
			cli.slice_depth, 
			cli.slice_height, 
			cli.slice_width, 
			cli.downsamplingFactor, 
			cli.halving_method, 
			cli.libtiff_rowsPerStrip, 
			cli.libtiff_uncompressed, 
			cli.libtiff_bigtiff, 
			cli.show_progress_bar, 
			cli.isotropic, 
			cli.V0, cli.V1, cli.H0, cli.H1, cli.D0, cli.D1,
			cli.timeseries,
			cli.makeDirs, 
			cli.metaData, 
			cli.parallel,
			cli.compress_params,
			cli.rescale_nbits);

		// display elapsed time
		printf("\nTime elapsed: %.1f seconds\n\n", proctime + TIME(0));
	}
	catch( iom::exception& exception){
		cout<<"ERROR: "<<exception.what()<<endl<<endl;
	}
	catch( iim::IOException& exception){
		cout<<"ERROR: "<<exception.what()<<endl<<endl;
	}
	catch(bad_exception& be){
		cout<<"GENERIC ERROR: "<<be.what()<<endl<<endl;
	}
	catch(char* error){
		cout<<"GENERIC ERROR: "<<error<<endl<<endl;
	}

	return EXIT_SUCCESS;
}