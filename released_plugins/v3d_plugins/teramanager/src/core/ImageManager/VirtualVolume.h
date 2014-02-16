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

#ifndef _VIRTUAL_VOLUME_H
#define _VIRTUAL_VOLUME_H

# define HALVE_BY_MEAN 1
# define HALVE_BY_MAX  2

#include <string>

#include "IM_config.h"

class VirtualVolume {

protected:
	//******OBJECT ATTRIBUTES******
	char*  root_dir;				//C-string that contains the directory path of stacks matrix
	float  VXL_V, VXL_H, VXL_D;		//[microns]: voxel dimensions (in microns) along V(Vertical), H(horizontal) and D(Depth) axes
	float  ORG_V, ORG_H, ORG_D;		//[millimeters]: origin spatial coordinates (in millimeters) along VHD axes
    iim::uint32 DIM_V, DIM_H, DIM_D;//volume dimensions (in voxels) along VHD axes
	int    CHANS;					//number of channels
	int    BYTESxCHAN;              //number of bytes per channel

    virtual void initChannels ( ) throw (iim::IOException) = 0;

public:
	//CONSTRUCTORS-DECONSTRUCTOR
    VirtualVolume(const char* _root_dir, float VXL_1=0, float VXL_2=0, float VXL_3=0) throw (iim::IOException)
    {

		this->root_dir = new char[strlen(_root_dir)+1];
		strcpy(this->root_dir, _root_dir);

		VXL_V = VXL_1;
		VXL_H = VXL_2;
		VXL_D = VXL_3;

		ORG_V = ORG_H = ORG_D = (float) 0.0;
		DIM_V = DIM_H = DIM_D = 0;

		CHANS = 0;
		BYTESxCHAN = 0;
    }

	virtual ~VirtualVolume() { 
		if(root_dir)
			delete[] root_dir;
	}

    //loads given subvolume in a 1-D array of iim::real32 while releasing stacks slices memory when they are no longer needed
    virtual iim::real32 *loadSubvolume_to_real32(int V0,int V1, int H0, int H1, int D0, int D1)  throw (iim::IOException) = 0;

    //loads given subvolume in a 1-D array of iim::uint8 while releasing stacks slices memory when they are no longer needed
    virtual iim::uint8 *loadSubvolume_to_UINT8(int V0,int V1, int H0, int H1, int D0, int D1, int *channels=0, int ret_type=iim::DEF_IMG_DEPTH) throw (iim::IOException) = 0;

    // ******GET METHODS******
    float   getORG_V() {return ORG_V;}
    float   getORG_H() {return ORG_H;}
    float   getORG_D() {return ORG_D;}
    int     getDIM_V() {return DIM_V;}
    int     getDIM_H() {return DIM_H;}
    int     getDIM_D() {return DIM_D;}
    int     getABS_V(int ABS_PIXEL_V) {return iim::round( ORG_V * 1000 + ABS_PIXEL_V*this->getVXL_V());}
    int     getABS_H(int ABS_PIXEL_H) {return iim::round( ORG_H * 1000 + ABS_PIXEL_H*this->getVXL_H());}
    int     getABS_D(int ABS_PIXEL_D) {return iim::round( ORG_D * 1000 + ABS_PIXEL_D*this->getVXL_D());}
    float   getVXL_V() {return VXL_V;}
    float   getVXL_H() {return VXL_H;}
    float   getVXL_D() {return VXL_D;}
    int     getCHANS() {return CHANS;}
    int     getBYTESxCHAN() {return BYTESxCHAN;}
    char*   getROOT_DIR() {return this->root_dir;}
    float  getMVoxels(){return (DIM_V/1024.0f)*(DIM_H/1024.0f)*DIM_D;}

	/*************************************************************************************************************
    * Save image method. <> parameters are mandatory, while [] are optional.
    * <img_path>                : absolute path of image to be saved. It DOES NOT include its extension, which is
    *                             provided by the [img_format] parameter.
    * <raw_img>                 : image to be saved. Raw data is in [0,1] and it is stored row-wise in a 1D array.
    * <raw_img_height/width>    : dimensions of raw_img.
    * [start/end_height/width]  : optional ROI (region of interest) to be set on the given image.
    * [img_format]              : image format extension to be used (e.g. "tif", "png", etc.)
    * [img_depth]               : image bitdepth to be used (8 or 16)
	**************************************************************************************************************/
    static void saveImage(std::string img_path,   iim::real32* raw_img,       int raw_img_height,   int   raw_img_width,
                              int start_height = 0,   int end_height = - 1,  int start_width = 0,  int end_width = - 1,
                              const char* img_format = iim::DEF_IMG_FORMAT.c_str(),    int img_depth = iim::DEF_IMG_DEPTH		 )
                                                                                                   throw (iim::IOException);

    /*************************************************************************************************************
    * Save image method from iim::uint8 raw data. <> parameters are mandatory, while [] are optional.
    * <img_path>                : absolute path of image to be saved. It DOES NOT include its extension, which is
    *                             provided by the [img_format] parameter.
    * <raw_ch1>                 : raw data of the first channel with values in [0,255].
    *                             For grayscale images this is the pointer to the raw image data.
    *                             For colour images this is the pointer to the raw image data of the RED channel.
    * <raw_ch2>                 : raw data of the second channel with values in [0,255].
    *                             For grayscale images this should be a null pointer.
    *                             For colour images this is the pointer to the raw image data of the GREEN channel.
    * <raw_ch3>                 : raw data of the second channel with values in [0,255].
    *                             For grayscale images this should be a null pointer.
    *                             For colour images this is the pointer to the raw image data of the BLUE channel.
    * <raw_img_height/width>    : dimensions of raw_img.        
    * [start/end_height/width]  : optional ROI (region of interest) to be set on the given image.
    * [img_format]              : image format extension to be used (e.g. "tif", "png", etc.)
    * [img_depth]               : image bitdepth to be used (8 or 16)
    **************************************************************************************************************/
    static void saveImage_from_UINT8 (std::string img_path, 
                                      iim::uint8* raw_ch1, iim::uint8* raw_ch2, iim::uint8* raw_ch3,
									  int raw_img_height, int raw_img_width,
                                      int start_height=0, int end_height =-1, int start_width=0, int end_width=-1,
                                      const char* img_format = iim::DEF_IMG_FORMAT.c_str(), int img_depth = iim::DEF_IMG_DEPTH ) throw (iim::IOException);


 	/*************************************************************************************************************
    * Save image method to Vaa3D raw format. <> parameters are mandatory, while [] are optional.
    * <img_path>                : absolute path of image to be saved. It DOES NOT include its extension, which is
    *                             provided by the [img_format] parameter.
    * <raw_img>                 : image to be saved. Raw data is in [0,1] and it is stored row-wise in a 1D array.
    * <raw_img_height/width>    : dimensions of raw_img.
    * [start/end_height/width]  : optional ROI (region of interest) to be set on the given image.
    * [img_format]              : image format extension to be used (e.g. "tif", "png", etc.)
    * [img_depth]               : image bitdepth to be used (8 or 16)
	**************************************************************************************************************/
    static void saveImage_to_Vaa3DRaw(int slice, std::string img_path, iim::real32* raw_img, int raw_img_height, int raw_img_width,
                              int start_height = 0, int end_height = - 1, int start_width = 0, int end_width = - 1,
                              const char* img_format = iim::DEF_IMG_FORMAT.c_str(), int img_depth = iim::DEF_IMG_DEPTH
							  )
                                                                                                   throw (iim::IOException);

   /*************************************************************************************************************
    * Save image method from iim::uint8 raw data to Vaa3D raw format. <> parameters are mandatory, while [] are optional.
    * <img_path>                : absolute path of image to be saved. It DOES NOT include its extension, which is
    *                             provided by the [img_format] parameter.
    * <raw_ch>                  : array of pointers to raw data of the channels with values in [0,255].
    *                             For grayscale images raw_ch[0] is the pointer to the raw image data.
    *                             For colour images raw_ch[0] is the pointer to the raw image data of the RED channel.
    * <n_chans>                 : number of channels (length of raw_ch).
	* <offset>                  : offset to be added to raw_ch[i] to get actual data
    * <raw_img_height/width>    : dimensions of raw_img.        
    * [start/end_height/width]  : optional ROI (region of interest) to be set on the given image.
    * [img_format]              : image format extension to be used (e.g. "tif", "png", etc.)
    * [img_depth]               : image bitdepth to be used (8 or 16)
    **************************************************************************************************************/
    static void saveImage_from_UINT8_to_Vaa3DRaw (int slice, std::string img_path, 
                                      iim::uint8** raw_ch, int n_chans, iim::sint64 offset,
									  int raw_img_height, int raw_img_width,
                                      int start_height=0, int end_height =-1, int start_width=0, int end_width=-1,
                                      const char* img_format = iim::DEF_IMG_FORMAT.c_str(), int img_depth = iim::DEF_IMG_DEPTH ) throw (iim::IOException);



	/*************************************************************************************************************
	* Performs downsampling at a halved frequency on the given 3D image.  The given image is overwritten in order
	* to store its halvesampled version without allocating any additional resources.
	*
	* WARNING: Since the downsampling is carried out for more slices, a stride is introduced between downsampled
	* slices. The stride introduced is (height*width)
	**************************************************************************************************************/
    static void halveSample( iim::real32* img, int height, int width, int depth, int method = HALVE_BY_MEAN );

    static void halveSample_UINT8 ( iim::uint8** img, int height, int width, int depth, int channels, int method = HALVE_BY_MEAN, int bytes_chan = 1 );

	//utility function: returns true if "fullString" ends with "ending"
	inline static bool hasEnding (std::string const &fullString, std::string const &ending)
	{
	   if (fullString.length() >= ending.length())
		  return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
	   else
		  return false;
	}

    // tries to automatically detect the volume format and returns the imported volume if succeeds (otherwise throws an exception)
    VirtualVolume* instance(const char* path) throw (iim::IOException);
};

#endif
