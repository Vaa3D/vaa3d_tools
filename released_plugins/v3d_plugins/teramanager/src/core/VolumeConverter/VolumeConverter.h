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

#ifndef VOLUME_CONVERTER_H
#define VOLUME_CONVERTER_H

/*
 * class VolumeConverter implements a converter form 3D-4D images in several formats
 * to the stacked format used by the tolls TeraStitcher and TeraFly (aka TeraManager)
 *
 * To use the class the user has to:

 * 1. call the method setSrcVolume on a newly created instance of the class passing
 *    the following parameters: 
 *
 *    - directory or file name of the source image (the file name if the image 
 *      is stored in a single file, e.g. in V3D raw format)
 *    - format of the source image ("Stacked" for Terastitcher stacked format, 
 *      "Simple" for sequence of numbered .tif images in the same directory,
 *      "Raw" for V3D raw 4D format)
 *    - format of the output image ("intensity" for real valued pixels in [0,1],
 *      "graylevel" for integer valued pixels in [0,255], "RGB" for pixel represented
 *      according to RGB format)
 *
 * If the source image is multi channel the format of the output image is 
 * automatically set to "RGB"
 *
 * WARNINIG: 
 *      graylevel output format not supported yet for Simple source format
 *      intensity output format not supported yet for Raw source format
 *
 * Allowed suffixes for V3D raw 4D format are: .raw .RAW, .v3draw .V3DRAW
 *
 *
 * 2. call the method generateTiles passing the following parameters:
 *
 *    - directory where to store the output image
 *    - number of resolutions to be generated
 *    - the height of the slices of the substacks in the output image
 *    - the width of the slices of the substacks in the output image
 */


#include <string>
#include <math.h>

#include "../ImageManager/VirtualVolume.h"

// possible output format
#define REAL_REPRESENTATION      "intensity"  // images are managed internally with REAL_INTERNAL_REP representation
                                              // and saved as graylevel images
#define UINT8_REPRESENTATION     "graylevel"  // images are managed internally with UINT8_INTERNAL_REP representation
                                              // and saved as graylevel images
#define UINT8x3_REPRESENTATION   "RGB"        // images are managed internally with UINT8_INTERNAL_REP representation
                                              // and saved as RGB images

#define REAL_INTERNAL_REP      1      // gray level images, pixels are represented ad real numbers in [0,1]
#define UINT8_INTERNAL_REP     2      // multi-channel images, pixels are represented as 8 bit integers


class VolumeConverter
{
	private:

		/******OBJECT MEMBERS******/
        VirtualVolume *volume;           //pointer to the <EmptyVolume> object to be stitched
		int V0, V1, H0, H1, D0, D1;				//voxel intervals that identify the final stitched volume
        int ROW_START, COL_START, ROW_END, COL_END;             //stack indexes that identify the stacks involved in stitching

		int internal_rep; // internal representation of pixels: 
		                  // REAL_INTERNAL_REP:  gray level images, pixels are represented ad real numbers in [0,1]
		                  // UINT8_INTERNAL_REP: multi-channel images, pixels are represented as 8 bit integers

		int channels;     // set only if internal_rep = UINT8_INTERNAL_REP
		                  // possible values: 1 for gray level images
		                  //                  3 for color images represented in RGB format

		const char *out_fmt;    // output format (for future use, currently not used: the output format is derived
		                  // implicitly from internal_rep and the format of the source image)

    public:

		// Constructors
		VolumeConverter(void);

		// Desctructor
		~VolumeConverter();

		/*************************************************************************************************************
		* Method to set (create) the source volume to be converted
		* _root_dir     : directory path where the volume is stored
		* _fmt          : format in which the source volume is stored (default: STACKED_FORMAT)            
		* _internal_fmt : format in which the pixels are represented internally (default: REAL_REPRESENTATION)
		*
		* When _out_fmt=REAL_REPRESENTATION the image must be graylevel
		* When _out_fmt=UINT8_REPRESENTATION or _out_fmt=UINT8x3_REPRESENTATION the image can be both 
		* graylevel and multi channel and it will be saved as RGB; at most three channels are supported; if channels
		* of original image are two, the third RGB channel (Blue channel) is set to all zero
		*************************************************************************************************************/
        void setSrcVolume(const char* _root_dir, const char* _fmt = iim::STACKED_FORMAT.c_str(),
                          const char* _out_fmt = REAL_REPRESENTATION, bool time_series = false) throw (iim::IOException);

        // unified access point for volume conversion (@ADDED by Alessandro on 2014-02-24)
        void convertTo(
            std::string output_path,                    // path where to save the converted volume
            std::string output_format,                  // format of the converted volume (see IM_config.h)
            int output_bitdepth = iim::NUL_IMG_DEPTH,   // output image bitdepth
            bool isTimeSeries = false,                  // whether the volume is a time series
            bool *resolutions = 0,                      // array of resolutions
            int block_height = -1,                      // tile's height (for tiled formats)
            int block_width  = -1,                      // tile's width  (for tiled formats)
            int block_depth  = -1,                      // tile's depth  (for tiled formats)
            int method = HALVE_BY_MEAN                  // downsampling method
        ) throw (iim::IOException);

		/*************************************************************************************************************
		* Method to be called for tile generation. <> parameters are mandatory, while [] are optional.
		* <output_path>			: absolute directory path where generated tiles have to be stored.
		* [resolutions]			: pointer to an array of S_MAX_MULTIRES  size which boolean entries identify the acti-
		*						  vaction/deactivation of the i-th resolution.  If not given, all resolutions will  be
		*						  activated.
		* [slice_height/width]	: desired dimensions of tiles  slices after merging.  It is actually an upper-bound of
		*						  the actual slice dimensions, which will be computed in such a way that all tiles di-
		*						  mensions can differ by 1 pixel only along both directions. If not given, the maximum
		*						  allowed dimensions will be set, which will result in a volume composed by  one large 
		*						  tile only.
		* [method]              : method used to compute pixel whel halving image size (default: by mean)
		* [show_progress_bar]	: enables/disables progress bar with estimated time remaining.
		* [saved_img_format]	: determines saved images format ("png","tif","jpeg", etc.).
		* [saved_img_depth]		: determines saved images bitdepth (16 or 8).
		**************************************************************************************************************/
		void generateTiles(std::string output_path, bool* resolutions = NULL, 
			int slice_height = -1, int slice_width = -1, int method = HALVE_BY_MEAN, bool show_progress_bar = true, 
            const char* saved_img_format = iim::DEF_IMG_FORMAT.c_str(), int saved_img_depth = iim::NUL_IMG_DEPTH,
            std::string frame_dir = "")	throw (iim::IOException);
		

		/*************************************************************************************************************
		* Method to be called for tile generation. <> parameters are mandatory, while [] are optional.
		* <output_path>			: absolute directory path where generated tiles have to be stored.
		* [resolutions]			: pointer to an array of S_MAX_MULTIRES  size which boolean entries identify the acti-
		*						  vaction/deactivation of the i-th resolution.  If not given, all resolutions will  be
		*						  activated.
		* [block_height]	    : desired dimensions of tiled  blocks after merging.  It is actually an upper-bound of
		* [block_width]			  the actual slice dimensions, which will be computed in such a way that all tiles di-
		* [block_depth]			  mensions can differ by 1 pixel only along both directions. If not given, the maximum
		*						  allowed dimensions will be set, which will result in a volume composed by  one large 
		*						  tile only.
		* [method]              : method used to compute pixel whel halving image size (default: by mean)
		* [show_progress_bar]	: enables/disables progress bar with estimated time remaining.
		* [saved_img_format]	: determines saved images format ("raw", "png","tif","jpeg", etc.).
		* [saved_img_depth]		: determines saved images bitdepth (16 or 8).
		* [frame_dir]           : name of the directory containing a frame (without the final "/")
		*                         if it is a null string the image does not belong to a time serie (default)
		**************************************************************************************************************/
		void generateTilesVaa3DRaw(std::string output_path, bool* resolutions = NULL, 
			int block_height = -1, int block_width = -1, int block_depth = -1, int method = HALVE_BY_MEAN, bool show_progress_bar = true, 
            const char* saved_img_format = "raw", int saved_img_depth = iim::NUL_IMG_DEPTH,
            std::string frame_dir = "")	throw (iim::IOException);
		

        /*************************************************************************************************************
        * Get methods
        **************************************************************************************************************/
        int getV0(){return V0;}
        int getV1(){return V1;}
        int getH0(){return H0;}
        int getH1(){return H1;}
        int getD0(){return D0;}
        int getD1(){return D1;}
        int getROW0(){return ROW_START;}
        int getROW1(){return ROW_END;}
        int getCOL0(){return COL_START;}
        int getCOL1(){return COL_END;}
        VirtualVolume *getVolume() {return volume;}

        /*************************************************************************************************************
        * Functions used to obtain absolute coordinates at different resolutions from relative coordinates
        **************************************************************************************************************/
        int getMultiresABS_V(int res, int REL_V);
        std::string getMultiresABS_V_string(int res, int REL_V);
        int getMultiresABS_H(int res, int REL_H);
        std::string getMultiresABS_H_string(int res, int REL_H);
		int getMultiresABS_D(int res);

        /*************************************************************************************************************
        * NEW TILED FORMAT SUPPORTING MULTIPLE CHANNELS
        **************************************************************************************************************/

	   /*************************************************************************************************************
		* Method to be called for tile generation. <> parameters are mandatory, while [] are optional.
		* <output_path>			: absolute directory path where generated tiles have to be stored.
		* [resolutions]			: pointer to an array of S_MAX_MULTIRES  size which boolean entries identify the acti-
		*						  vaction/deactivation of the i-th resolution.  If not given, all resolutions will  be
		*						  activated.
		* [block_height]	    : desired dimensions of tiled  blocks after merging.  It is actually an upper-bound of
		* [block_width]			  the actual slice dimensions, which will be computed in such a way that all tiles di-
		* [block_depth]			  mensions can differ by 1 pixel only along both directions. If not given, the maximum
		*						  allowed dimensions will be set, which will result in a volume composed by  one large 
		*						  tile only.
		* [method]              : method used to compute pixel whel halving image size (default: by mean)
		* [show_progress_bar]	: enables/disables progress bar with estimated time remaining.
		* [saved_img_format]	: determines saved images format ("raw", "png","tif","jpeg", etc.).
		* [saved_img_depth]		: determines saved images bitdepth (16 or 8).
		**************************************************************************************************************/
		void generateTilesVaa3DRawMC ( std::string output_path, bool* resolutions = NULL, 
			int block_height = -1, int block_width = -1, int block_depth = -1, int method = HALVE_BY_MEAN, bool show_progress_bar = true, 
            const char* saved_img_format = "raw", int saved_img_depth = iim::NUL_IMG_DEPTH,
            std::string frame_dir = "")	throw (iim::IOException);

};

#endif


