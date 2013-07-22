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
*       Bria, A., et al., (2012) "Stitching Terabyte-sized 3D Images Acquired in Confocal Ultramicroscopy", Proceedings of the 9th IEEE International Symposium on Biomedical Imaging.
*       Bria, A., Iannello, G., "A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Datasets", submitted on July 2012 to IEEE Transactions on Information Technology in Biomedicine.
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

#ifndef _VIRTUAL_VOLUME_H
#define _VIRTUAL_VOLUME_H

# define HALVE_BY_MEAN 1
# define HALVE_BY_MAX  2

#include <string>

#include "IM_defs.h"
#include "MyException.h"

class VirtualVolume {

protected:
	//******OBJECT ATTRIBUTES******
        char*  root_dir;			//C-string that contains the directory path of stacks matrix
	float  VXL_V, VXL_H, VXL_D;		//[microns]: voxel dimensions (in microns) along V(Vertical), H(horizontal) and D(Depth) axes
	float  ORG_V, ORG_H, ORG_D;		//[millimeters]: origin spatial coordinates (in millimeters) along VHD axes
	uint32 DIM_V, DIM_H, DIM_D;		//volume dimensions (in voxels) along VHD axes
        int    CHANS;				//number of channels

	virtual void initChannels ( ) throw (MyException) = 0;

public:
	//CONSTRUCTORS-DECONSTRUCTOR
    VirtualVolume(const char* _root_dir, float VXL_1=0, float VXL_2=0, float VXL_3=0) throw (MyException)
    {

		this->root_dir = new char[strlen(_root_dir)+1];
		strcpy(this->root_dir, _root_dir);

		VXL_V = VXL_1;
		VXL_H = VXL_2;
		VXL_D = VXL_3;

		ORG_V = ORG_H = ORG_D = (float) 0.0;
		DIM_V = DIM_H = DIM_D = 0;

		CHANS = 0;
    }

	virtual ~VirtualVolume() { 
		if(root_dir)
			delete[] root_dir;
	}

	//loads given subvolume in a 1-D array of REAL_T while releasing stacks slices memory when they are no longer needed
	virtual REAL_T *loadSubvolume_to_REAL_T(int V0,int V1, int H0, int H1, int D0, int D1)  throw (MyException) = 0;

    //loads given subvolume in a 1-D array of uint8 while releasing stacks slices memory when they are no longer needed
    virtual uint8 *loadSubvolume_to_UINT8(int V0,int V1, int H0, int H1, int D0, int D1, int *channels) throw (MyException) = 0;

    // ******GET METHODS******
    float   getORG_V() {return ORG_V;}
    float   getORG_H() {return ORG_H;}
    float   getORG_D() {return ORG_D;}
    int     getDIM_V() {return DIM_V;}
    int     getDIM_H() {return DIM_H;}
    int     getDIM_D() {return DIM_D;}
    int     getABS_V(int ABS_PIXEL_V) {return (int)ROUND( ORG_V * 1000 + ABS_PIXEL_V*this->getVXL_V());}
    int     getABS_H(int ABS_PIXEL_H) {return (int)ROUND( ORG_H * 1000 + ABS_PIXEL_H*this->getVXL_H());}
    int     getABS_D(int ABS_PIXEL_D) {return (int)ROUND( ORG_D * 1000 + ABS_PIXEL_D*this->getVXL_D());}
    float   getVXL_V() {return VXL_V;}
    float   getVXL_H() {return VXL_H;}
    float   getVXL_D() {return VXL_D;}
    int     getCHANS() {return CHANS;}
    char*   getROOT_DIR() {return this->root_dir;}

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
	static void saveImage(std::string img_path,   REAL_T* raw_img,       int raw_img_height,   int   raw_img_width, 
                              int start_height = 0,   int end_height = - 1,  int start_width = 0,  int end_width = - 1,
                              const char* img_format = IM_DEF_IMG_FORMAT,    int img_depth = IM_DEF_IMG_DEPTH		 )
																								   throw (MyException);

    /*************************************************************************************************************
    * Save image method from uint8 raw data. <> parameters are mandatory, while [] are optional.
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
                                      uint8* raw_ch1, uint8* raw_ch2, uint8* raw_ch3,
									  int raw_img_height, int raw_img_width,
                                      int start_height=0, int end_height =-1, int start_width=0, int end_width=-1,
                                      const char* img_format = IM_DEF_IMG_FORMAT, int img_depth = IM_DEF_IMG_DEPTH ) throw (MyException);

	/*************************************************************************************************************
	* Performs downsampling at a halved frequency on the given 3D image.  The given image is overwritten in order
	* to store its halvesampled version without allocating any additional resources.
	**************************************************************************************************************/
	static void halveSample( REAL_T* img, int height, int width, int depth, int method = HALVE_BY_MEAN );

	static void halveSample_UINT8 ( uint8** img, int height, int width, int depth, int channels, int method = HALVE_BY_MEAN );
};

#endif
