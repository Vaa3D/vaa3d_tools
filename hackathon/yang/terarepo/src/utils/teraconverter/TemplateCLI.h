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
* 2016-09-13. Giulio.     @ADDED flag for time series
* 2016-09-04. Giulio.     @ADDED the options for setting the configuration of the LibTIFF library
* 2016-06-18  Giulio.     @ADDED option for downsampling the reading of data
* 2016-04-13  Giulio.     @ADDED options for parallelizing teraconverter
*/

#ifndef _TEMPLATE_COMMAND_LINE_INTERFACE_H
#define _TEMPLATE_COMMAND_LINE_INTERFACE_H

#include <string>
#include "iomanager.config.h"
#include "GUI_config.h"

using namespace std;

class TemplateCLI
{
	public:

		// switch parameters
		//bool highest_resolution;						// generate highest resolution (default: false)
        bool makeDirs;                          //creates the directory hiererchy
        bool metaData;                          //creates the mdata.bin file of the output volume
        bool parallel;                          //parallel mode: does not perform side-effect operations during merge
        bool isotropic;                         //generate lowest resolutions with voxels as much isotropic as possible

		bool pluginsinfo;						//display plugins information

		bool timeseries;						//the input is a time series

		// other parameters
		// int/float/double/string XXXX;	// description
		string src_root_dir;
		string dst_root_dir;
		string mdata_fname;                 // name of the file containing general metadata to be transferred to destination file (used only by some formats)
		string ch_dir;                      // name of the subdirectory where image should be saved (only for a single channel converted to tiled 4D format)
		int slice_depth;
		int slice_height;
		int slice_width;
		string src_format;
		string dst_format;
		bool resolutions[S_MAX_MULTIRES];
		string chanlist;
		int halving_method;
		bool show_progress_bar;					//enables/disables progress bar with estimated time remaining

		string outFmt;
		string infofile_path;					//file path of the info file to be saved
		int downsamplingFactor;                 //downsampling factor to be used to read source volume (only if it is a serie of 2D slices)

		// vertices defining the subvolume to be converted
		int V0;
		int V1;
		int H0;
		int H1;
		int D0;
		int D1;

		int tm_blending;						//tiles merging blending type

		// parameters to configure LibTIFF
		bool libtiff_uncompressed;
		bool libtiff_bigtiff;
		int libtiff_rowsPerStrip;

		// parameters for compression algorithms in HDF5 files
		std::string compress_params;

		// parameters for lossy compression algorithms
		int rescale_nbits;

		//constructor - deconstructor
		TemplateCLI(void);					//set default params
		~TemplateCLI(void){};

		//reads options and parameters from command line
		void readParams(int argc, char** argv) throw (iom::exception);

		//checks parameters correctness
		void checkParams() throw (iom::exception);

		//returns help text
		string getHelpText();

		//print all arguments
		void print();
};

#endif /* _TERASTITCHER_COMMAND_LINE_INTERFACE_H */


