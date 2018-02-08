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
* 2017-09-11. Giulio.     @CHANGED interfaces of 'vcDriver' and 'convetTo' to enable passing parameters controlloing the compression algorithm to be used with HDf5 files
* 2017-09-11. Giulio.     @ADDED parameters controlloing the compression algorithm to be used with HDf5 files
* 2017-06-26. Giulio.     @ADDED parameter 'isotropic' and 'mdata_file' to method 'convertTo'
* 2017-05-25. Giulio.     @ADDED method for enabling lossy compression based on rescaling
* 2017-04-23. Giulio.     @ADDED auxiliary function vcDriver to call the actual conversion method
* 2017-04-18. Alessandro. @ADDED setSrcVolume that directly takes vm::VirtualVolume in input, and added 'volume_external' attribute
* 2017-04-09. Giulio.     @ADDED the ability to convert a subset of channels
* 2017-01-22. Giulio      @ADDED parameter for standard block depth for efficiency reasons
* 2016-10-09. Giulio.     @ADDED parameter 'ch_dir' to 'generateTilesVaa3DRawMC' interface
* 2014-06-20. Giulio.     @ADDED methods for conversion to 'simple' representation (series, 2D), including parallel support
* 2016-04-13  Giulio.     @ADDED methods to manage parallelization
*/

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

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "../imagemanager/VirtualVolume.h"

// possible output format
#define REAL_REPRESENTATION      "intensity"  // images are managed internally with REAL_INTERNAL_REP representation
                                              // and saved as graylevel images
#define UINT8_REPRESENTATION     "graylevel"  // images are managed internally with UINT8_INTERNAL_REP representation
                                              // and saved as graylevel images
#define UINT8x3_REPRESENTATION   "RGB"        // images are managed internally with UINT8_INTERNAL_REP representation
                                              // and saved as RGB images

#define REAL_INTERNAL_REP      1      // gray level images, pixels are represented ad real numbers in [0,1]
#define UINT8_INTERNAL_REP     2      // multi-channel images, pixels are represented as 8 bit integers

#define STANDARD_BLOCK_DEPTH   64      // standard block to be used when converting to tiled format (introduced for efficiency)

// thread-safe queue
template <typename T>
class Queue
{
public:

    T pop()
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        while (queue_.empty())
        {
            cond_.wait(mlock);
        }
        auto val = queue_.front();
        queue_.pop();
        return val;
    }

    void pop(T& item)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        while (queue_.empty())
        {
            cond_.wait(mlock);
        }
        item = queue_.front();
        queue_.pop();
    }

    void push(const T& item)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        queue_.push(item);
        mlock.unlock();
        cond_.notify_one();
    }
    Queue()=default;
    Queue(const Queue&) = delete;            // disable copying
    Queue& operator=(const Queue&) = delete; // disable assignment

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

// 2017-04-23. Giulio. @ADDED auxiliary function to call format conversion
void vcDriver (
	iim::VirtualVolume *vPtr = (iim::VirtualVolume *) 0, // if this in not null, parameters 'src_root_dir' and 'src_format' are not used
	std::string src_root_dir = "",
	std::string dst_root_dir = "",
	std::string src_format = "",
	std::string dst_format = "",
	int         img_depth = 0, // currently not used: imout depth is set internally
	bool       *resolutions = (bool *) 0,
	std::string chanlist = "",
	std::string ch_dir = "",                     // name of the subdirectory where image should be saved (only for a single channel converted to tiled 4D format)
	std::string mdata_fname = "",                // name of the file containing general metadata to be transferred to destination file (used only by some formats)
	int         slice_depth = -1,
	int         slice_height = -1,
	int         slice_width = -1,
	int         downsamplingFactor = 1,
	int         halving_method = HALVE_BY_MEAN,
	int         libtiff_rowsPerStrip = 1,
	bool        libtiff_uncompressed = false,
	bool        libtiff_bigtiff = false,
	bool        show_progress_bar = true,			//enables/disables progress bar with estimated time remaining
    bool        isotropic = true,                   //generate lowest resolutions with voxels as much isotropic as possible
	int V0 = -1, int V1 = -1, int H0 = -1, int H1 = -1, int D0 = -1,int D1 = -1,
	// the following parameters are for commandline version only; omit them in the GUI version (use default values)
	bool        timeseries = false,
    bool        makeDirs = false,                   //creates the directory hiererchy
    bool        metaData = false,                   //creates the mdata.bin file of the output volume
    bool        parallel = false,                   //parallel mode: does not perform side-effect operations during merge
	std::string outFmt = "RGB",                     //additional information about the output format (default: "")
	int         nbits  = 0
) throw (iim::IOException, iom::exception);

class VolumeConverter
{
	private:

		/******OBJECT MEMBERS******/
        iim::VirtualVolume *volume;             // pointer to the <EmptyVolume> object to be stitched
		bool volume_external;				    // whether 'volume' has been provided by an external caller (in which case it must not be deallocated / modified)
		int V0, V1, H0, H1, D0, D1;				// voxel intervals that identify the final stitched volume
        int ROW_START, COL_START, ROW_END, COL_END;             //stack indexes that identify the stacks involved in stitching

		int internal_rep; // internal representation of pixels: 
		                  // REAL_INTERNAL_REP:  gray level images, pixels are represented ad real numbers in [0,1]
		                  // UINT8_INTERNAL_REP: multi-channel images, pixels are represented as 8 bit integers

		int channels;     // set only if internal_rep = UINT8_INTERNAL_REP
		                  // possible values: 1 for gray level images
		                  //                  3 for color images represented in RGB format

		const char *out_fmt;    // output format (for future use, currently not used: the output format is derived
		                  // implicitly from internal_rep and the format of the source image)

		bool lossy_compression;
		int  nbits;

    public:

		// Constructors
		VolumeConverter(void);

		// Desctructor
		~VolumeConverter();

		/*************************************************************************************************************
		* Method to set (create) the source volume to be converted
		* _root_dir          : directory path where the volume is stored
		* _fmt               : format in which the source volume is stored (default: STACKED_FORMAT)            
		* _out_fmt           : format in which the pixels are represented internally (default: REAL_REPRESENTATION)
		* time_series        :
		* downsamplingFactor :
		* _res               : resolution
		* _timepoint         : time point
		*
		* The parameter 'downdamplingFactor' can be different from 1 used only if the internal volume is a 
		* serie of 2D slices (derived classes: SimpleVolume / SimpleVolumeRaw)
		*
		* When _out_fmt=REAL_REPRESENTATION the image must be graylevel
		* When _out_fmt=UINT8_REPRESENTATION or _out_fmt=UINT8x3_REPRESENTATION the image can be both 
		* graylevel and multi channel and it will be saved as RGB; at most three channels are supported; if channels
		* of original image are two, the third RGB channel (Blue channel) is set to all zero
		*************************************************************************************************************/
        void setSrcVolume(const char* _root_dir, const char* _fmt = iim::STACKED_FORMAT.c_str(),
                          const char* _out_fmt = REAL_REPRESENTATION, bool time_series = false, 
						  int downsamplingFactor = 1, std::string chanlist = "", int _res = 0, int _timepoint = 0) throw (iim::IOException, iom::exception);

		// additional setSrcVolume @ADDED by Alessandro on 2014-04-18: takes an external vm::VirtualVolume in input
		void setSrcVolume(iim::VirtualVolume * _imported_volume,
			const char* _out_fmt = REAL_REPRESENTATION, bool time_series = false, 
			int downsamplingFactor = 1, std::string chanlist = "", int _res = 0, int _timepoint = 0) throw (iim::IOException, iom::exception);


		/*************************************************************************************************************
		* Method to set the subvolume to be converted
		* _V0 ... _D1: vertices defining the subvolume to be converted
		*
		* default values correspond to the whole volume
		*************************************************************************************************************/
        void setSubVolume(int _V0 = -1, int _V1 = -1, int _H0 = -1, int _H1 = -1, int _D0 = -1, int _D1 = -1 ) throw (iim::IOException);

        // unified access point for volume conversion (@ADDED by Alessandro on 2014-02-24)
		// currently it does not support the possibility to provide a name for the subdirectory to store the converted image when output format is tiled 4D and the source has only one channel
        void convertTo(
            std::string output_path,                    // path where to save the converted volume
            std::string output_format,                  // format of the converted volume (see IM_config.h)
            int output_bitdepth = iim::NUL_IMG_DEPTH,   // output image bitdepth
            bool isTimeSeries = false,                  // whether the volume is a time series
            bool *resolutions = 0,                      // array of resolutions
            int block_height = -1,                      // tile's height (for tiled formats)
            int block_width  = -1,                      // tile's width  (for tiled formats)
            int block_depth  = -1,                      // tile's depth  (for tiled formats)
            int method = HALVE_BY_MEAN,                 // downsampling method
			bool isotropic = false,                     // perform an isotropic conversion 
			std::string metadata_file = "null",         // last parameter, used only by Imaris file format
 			std::string compression_info = ""           // last parameter, used only by Imaris file format
       ) throw (iim::IOException, iom::exception);

		/*************************************************************************************************************
		* Method to set a lossy compression algorithm
		* _algo: lossy algorithm 
		*        0: no algorithm (i.e. lossless), no additional parameters 
		*        1: scaling, additional parameters: 
		*           nbits: number of least significant bits to be set to 0
		*************************************************************************************************************/
		void setCompressionAlgorithm(int _nbits ) throw (iim::IOException, iom::exception);

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
			int slice_height = -1, int slice_width = -1, int method = HALVE_BY_MEAN, bool isotropic = false, 
            bool show_progress_bar = true, const char* saved_img_format = iim::DEF_IMG_FORMAT.c_str(), 
            int saved_img_depth = iim::NUL_IMG_DEPTH, std::string frame_dir = "", bool par_mode = false)	throw (iim::IOException, iom::exception);
		

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
		void generateTilesSimple(std::string output_path, bool* resolutions = NULL, 
			int slice_height = -1, int slice_width = -1, int method = HALVE_BY_MEAN, bool isotropic = false, 
            bool show_progress_bar = true, const char* saved_img_format = iim::DEF_IMG_FORMAT.c_str(), 
            int saved_img_depth = iim::NUL_IMG_DEPTH, std::string frame_dir = "", bool par_mode = false)	throw (iim::IOException, iom::exception);
		

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
			int block_height = -1, int block_width = -1, int block_depth = -1, int method = HALVE_BY_MEAN, bool isotropic=false, 
            bool show_progress_bar = true, const char* saved_img_format = "Vaa3DRaw", int saved_img_depth = iim::NUL_IMG_DEPTH,
            std::string frame_dir = "", bool par_mode=false) throw (iim::IOException, iom::exception);


        // similar to generateTilesVaa3DRaw but save a volume directly instead of slices
        void generate3DTiles(std::string output_path, bool* resolutions = NULL,
            int block_height = -1, int block_width = -1, int block_depth = -1, int method = HALVE_BY_MEAN, bool isotropic=false,
            bool show_progress_bar = true, const char* saved_img_format = "Vaa3DRaw", int saved_img_depth = iim::NUL_IMG_DEPTH,
            std::string frame_dir = "", bool par_mode=false) throw (iim::IOException, iom::exception);

        // multithreaded generate tiles and save
        void generateTilesMT(std::string output_path, bool* resolutions = NULL,
            int block_height = -1, int block_width = -1, int block_depth = -1, int method = HALVE_BY_MEAN, bool isotropic=false,
            bool show_progress_bar = true, const char* saved_img_format = "Vaa3DRaw", int saved_img_depth = iim::NUL_IMG_DEPTH,
            std::string frame_dir = "", bool par_mode=false) throw (iim::IOException, iom::exception);
		

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
        iim::VirtualVolume *getVolume() {return volume;}

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
		* [ch_dir]			    : if source has more than one channel it is ignored;
		*                         it it is not an empty string, it  provides the name of  the  subdirectory  in  which 
		*                         store the converted image
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
		void generateTilesVaa3DRawMC ( std::string output_path, std::string ch_dir = "", bool* resolutions = NULL, 
			int block_height = -1, int block_width = -1, int block_depth = -1, int method = HALVE_BY_MEAN, bool isotropic=false, 
			bool show_progress_bar = true, const char* saved_img_format = "Vaa3DRaw", int saved_img_depth = iim::NUL_IMG_DEPTH,
 			std::string frame_dir = "", bool par_mode=false)	throw (iim::IOException, iom::exception);

      /*************************************************************************************************************
        * NEW FORMAT SUPPORTING BDV HDF5 custom format
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
		void generateTilesBDV_HDF5 ( std::string output_path, bool* resolutions = NULL, 
			int block_height = -1, int block_width = -1, int block_depth = -1, int method = HALVE_BY_MEAN, bool isotropic=false, 
			bool show_progress_bar = true, const char* saved_img_format = "h5", int saved_img_depth = iim::NUL_IMG_DEPTH,
            std::string frame_dir = "")	throw (iim::IOException, iom::exception);


		void generateTilesIMS_HDF5 ( std::string output_path, std::string metadata_file, bool* resolutions = NULL, 
			int block_height = -1, int block_width = -1, int block_depth = -1, int method = HALVE_BY_MEAN, bool isotropic=false, 
			bool show_progress_bar = true, const char* saved_img_format = "ims", int saved_img_depth = iim::NUL_IMG_DEPTH,
            std::string frame_dir = "")	throw (iim::IOException, iom::exception);


		void createDirectoryHierarchy(std::string output_path, std::string ch_dir = "", bool* resolutions = NULL, 
			int block_height = -1, int block_width = -1, int block_depth = -1, int method = HALVE_BY_MEAN, bool isotropic=false, 
            bool show_progress_bar = true, const char* saved_img_format = "Vaa3DRaw", int saved_img_depth = iim::NUL_IMG_DEPTH,
            std::string frame_dir = "", bool par_mode=false) throw (iim::IOException, iom::exception);
		

		void createDirectoryHierarchySimple(std::string output_path, bool* resolutions = NULL, 
			int block_height = -1, int block_width = -1, int block_depth = -1, int method = HALVE_BY_MEAN, bool isotropic=false, 
            bool show_progress_bar = true, const char* saved_img_format = "Vaa3DRaw", int saved_img_depth = iim::NUL_IMG_DEPTH,
            std::string frame_dir = "", bool par_mode=false) throw (iim::IOException, iom::exception);


		void mdataGenerator(std::string output_path, std::string ch_dir = "", bool* resolutions = NULL, 
			int block_height = -1, int block_width = -1, int block_depth = -1, int method = HALVE_BY_MEAN, bool isotropic=false, 
            bool show_progress_bar = true, const char* saved_img_format = "Vaa3DRaw", int saved_img_depth = iim::NUL_IMG_DEPTH,
            std::string frame_dir = "", bool par_mode=false) throw (iim::IOException, iom::exception);	
};

#endif


