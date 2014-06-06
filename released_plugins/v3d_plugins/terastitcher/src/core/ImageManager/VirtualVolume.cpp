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

#include "VirtualVolume.h"
#include "SimpleVolume.h"
#include "SimpleVolumeRaw.h"
#include "RawVolume.h"
#include "TiledVolume.h"
#include "TiledMCVolume.h"
#include "StackedVolume.h"
#include "RawFmtMngr.h"
#include "Tiff3DMngr.h"
#include "TimeSeries.h"

#include <cxcore.h>
#include <highgui.h>
#include <stdio.h>

using namespace iim;


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
void VirtualVolume::saveImage(std::string img_path, real32* raw_img, int raw_img_height, int  raw_img_width,
							 int start_height, int end_height, int start_width, int end_width, 
                             const char* img_format, int img_depth) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("img_path=%s, raw_img_height=%d, raw_img_width=%d, start_height=%d, end_height=%d, start_width=%d, end_width=%d",
                                        img_path.c_str(), raw_img_height, raw_img_width, start_height, end_height, start_width, end_width).c_str(), __iim__current__function__);

	IplImage *img;
	uint8  *row_data_8bit;
	uint16 *row_data_16bit;
	uint32 img_data_step;
	float scale_factor_16b, scale_factor_8b;
	int img_height, img_width;
	int i,j;
	char img_filepath[5000];

	//setting some default parameters and image dimensions
	end_height = (end_height == -1 ? raw_img_height - 1 : end_height);
	end_width  = (end_width  == -1 ? raw_img_width  - 1 : end_width );
	img_height = end_height - start_height + 1;
	img_width  = end_width  - start_width  + 1;

	//checking parameters correctness
	if(!(start_height>=0 && end_height>start_height && end_height<raw_img_height && start_width>=0 && end_width>start_width && end_width<raw_img_width))
	{
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"in saveImage(..., raw_img_height=%d, raw_img_width=%d, start_height=%d, end_height%d, start_width=%d, end_width=%d): invalid image portion\n",
			raw_img_height, raw_img_width, start_height, end_height, start_width, end_width);
        throw IOException(err_msg);
	}
	if(img_depth != 8 && img_depth != 16)
	{
		char err_msg[STATIC_STRINGS_SIZE];		
		sprintf(err_msg,"in saveImage(..., img_depth=%d, ...): unsupported bit depth\n",img_depth);
        throw IOException(err_msg);
	}

	//generating complete path for image to be saved
	sprintf(img_filepath, "%s.%s", img_path.c_str(), img_format);

	//converting raw data in image data
	img = cvCreateImage(cvSize(img_width, img_height), (img_depth == 8 ? IPL_DEPTH_8U : IPL_DEPTH_16U), 1);
	scale_factor_16b = 65535.0F;
	scale_factor_8b  = 255.0F;
	if(img->depth == IPL_DEPTH_8U)
	{
		img_data_step = img->widthStep / sizeof(uint8);
		for(i = 0; i <img_height; i++)
		{
			row_data_8bit = ((uint8*)(img->imageData)) + i*img_data_step;
			for(j = 0; j < img_width; j++)
				row_data_8bit[j] = (uint8) (raw_img[(i+start_height)*raw_img_width+j+start_width] * scale_factor_8b);
		}
	}
	else
	{
		img_data_step = img->widthStep / sizeof(uint16);
		for(i = 0; i <img_height; i++)
		{
			row_data_16bit = ((uint16*)(img->imageData)) + i*img_data_step;
			for(j = 0; j < img_width; j++)
				row_data_16bit[j] = (uint16) (raw_img[(i+start_height)*raw_img_width+j+start_width] * scale_factor_16b);
		}
	}

	//saving image
	try{cvSaveImage(img_filepath, img);}
    catch(...)
	{
		char err_msg[STATIC_STRINGS_SIZE];		
		sprintf(err_msg,"in saveImage(...): unable to save image at \"%s\". Unsupported format or wrong path.\n",img_filepath);
        throw IOException(err_msg);
	}

	//releasing memory of img
	cvReleaseImage(&img);
}

/*************************************************************************************************************
* Save image method from uint8 raw data. <> parameters are mandatory, while [] are optional.
* <img_path>                : absolute path of image to be saved. It DOES NOT include its extension, which is
*                             provided by the [img_format] parameter.
* <raw_img_height/width>    : dimensions of raw_img.
* <raw_ch1>                 : raw data of the first channel with values in [0,255].
*                             For grayscale images this is the pointer to the raw image data.
*                             For colour images this is the pointer to the raw image data of the RED channel.
* <raw_ch2>                 : raw data of the second channel with values in [0,255].
*                             For grayscale images this should be a null pointer (as it is by default).
*                             For colour images this is the pointer to the raw image data of the GREEN channel.
* <raw_ch3>                 : raw data of the third channel with values in [0,255].
*                             For grayscale images this should be a null pointer (as it is by default).
*                             For colour images this is the pointer to the raw image data of the BLUE channel.
* [start/end_height/width]  : optional ROI (region of interest) to be set on the given image.
* [img_format]              : image format extension to be used (e.g. "tif", "png", etc.)
* [img_depth]               : image bitdepth to be used (8 or 16)
**************************************************************************************************************/
void VirtualVolume::saveImage_from_UINT8 (std::string img_path, uint8* raw_ch1, uint8* raw_ch2, uint8* raw_ch3, 
                           int raw_img_height, int raw_img_width, int start_height, int end_height, int start_width,
                           int end_width, const char* img_format, int img_depth ) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("img_path=%s, raw_img_height=%d, raw_img_width=%d, start_height=%d, end_height=%d, start_width=%d, end_width=%d, img_format=%s, img_depth=%d",
                                        img_path.c_str(), raw_img_height, raw_img_width, start_height, end_height, start_width, end_width, img_format, img_depth).c_str(), __iim__current__function__);

    //checking for non implemented features
	if( img_depth != 8 ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"SimpleVolume::loadSubvolume_to_UINT8: invalid number of bits per channel (%d)",img_depth); 
        throw IOException(err_msg);
	}

    //LOCAL VARIABLES
    char buffer[STATIC_STRINGS_SIZE];
    IplImage* img = 0;
    int img_height, img_width;
    int nchannels = 0;

    //detecting the number of channels (WARNING: the number of channels is an attribute of the volume
    nchannels = static_cast<int>(raw_ch1!=0) + static_cast<int>(raw_ch2!=0) + static_cast<int>(raw_ch3!=0);

    //setting some default parameters and image dimensions
    end_height = (end_height == -1 ? raw_img_height - 1 : end_height);
    end_width  = (end_width  == -1 ? raw_img_width  - 1 : end_width );
    img_height = end_height - start_height + 1;
    img_width  = end_width  - start_width  + 1;

    //checking parameters correctness
    if(!(start_height>=0 && end_height>start_height && end_height<raw_img_height && start_width>=0 && end_width>start_width && end_width<raw_img_width))
    {
        sprintf(buffer,"in saveImage_from_UINT8(..., raw_img_height=%d, raw_img_width=%d, start_height=%d, end_height%d, start_width=%d, end_width=%d): invalid image portion\n",
                raw_img_height, raw_img_width, start_height, end_height, start_width, end_width);
        throw IOException(buffer);
    }
	// nchannels may be also 2
    //if(nchannels != 1 && nchannels != 3)
    if(nchannels > 3)
    {
        sprintf(buffer,"in saveImage_from_UINT8(): unsupported number of channels (= %d)\n",nchannels);
        throw IOException(buffer);
    }
    if(img_depth != 8 && img_depth != 16 && nchannels == 1)
    {
        sprintf(buffer,"in saveImage_from_UINT8(..., img_depth=%d, ...): unsupported bit depth for greyscale images\n",img_depth);
        throw IOException(buffer);
    }
    //if(img_depth != 8 && nchannels == 3) // nchannels may be also 2
    if(img_depth != 8 && nchannels > 1)
    {
        sprintf(buffer,"in saveImage_from_UINT8(..., img_depth=%d, ...): unsupported bit depth for multi-channels images\n",img_depth);
        throw IOException(buffer);
    }

    //converting raw data into OpenCV image data
    if(nchannels == 3)
    {
        img = cvCreateImage(cvSize(img_width, img_height), IPL_DEPTH_8U, 3);
        int img_data_step = img->widthStep / sizeof(uint8);
        for(int i = 0; i <img_height; i++)
        {
            uint8* row_data_8bit = ((uint8*)(img->imageData)) + i*img_data_step;
            for(int j1 = 0, j2 = start_width; j1 < img_width*3; j1+=3, j2++)
            {
                row_data_8bit[j1  ] = raw_ch3[(i+start_height)*raw_img_width + j2];
                row_data_8bit[j1+1] = raw_ch2[(i+start_height)*raw_img_width + j2];
                row_data_8bit[j1+2] = raw_ch1[(i+start_height)*raw_img_width + j2];
            }
        }
    }
    if(nchannels == 2)
    {
        img = cvCreateImage(cvSize(img_width, img_height), IPL_DEPTH_8U, 3);
        int img_data_step = img->widthStep / sizeof(uint8);
        for(int i = 0; i <img_height; i++)
        {
            uint8* row_data_8bit = ((uint8*)(img->imageData)) + i*img_data_step;
            for(int j1 = 0, j2 = start_width; j1 < img_width*3; j1+=3, j2++)
            {
                row_data_8bit[j1  ] = 0;
                row_data_8bit[j1+1] = raw_ch2[(i+start_height)*raw_img_width + j2];
                row_data_8bit[j1+2] = raw_ch1[(i+start_height)*raw_img_width + j2];
            }
        }
    }
    else if(nchannels == 1)
    {
        img = cvCreateImage(cvSize(img_width, img_height), (img_depth == 8 ? IPL_DEPTH_8U : IPL_DEPTH_16U), 1);
        float scale_factor_16b = 65535.0F/255;
        if(img->depth == IPL_DEPTH_8U)
        {
            int img_data_step = img->widthStep / sizeof(uint8);
            for(int i = 0; i <img_height; i++)
            {
                uint8* row_data_8bit = ((uint8*)(img->imageData)) + i*img_data_step;
                for(int j = 0; j < img_width; j++)
                    row_data_8bit[j] = raw_ch1[(i+start_height)*raw_img_width+j+start_width];
            }
        }
        else
        {
            int img_data_step = img->widthStep / sizeof(uint16);
            for(int i = 0; i <img_height; i++)
            {
                uint16* row_data_16bit = ((uint16*)(img->imageData)) + i*img_data_step;
                for(int j = 0; j < img_width; j++)
                    row_data_16bit[j] = (uint16) (raw_ch1[(i+start_height)*raw_img_width+j+start_width] * scale_factor_16b);
            }
        }
    }


    //generating complete path for image to be saved
    sprintf(buffer, "%s.%s", img_path.c_str(), img_format);

    //saving image
    try{cvSaveImage(buffer, img);}
    catch(std::exception ex)
    {
        char err_msg[STATIC_STRINGS_SIZE];
        sprintf(err_msg,"in saveImage_from_UINT8(...): unable to save image at \"%s\". Unsupported format or wrong path.\n",buffer);
        throw IOException(err_msg);
    }

    //releasing memory
    cvReleaseImage(&img);
}



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
void VirtualVolume::saveImage_to_Vaa3DRaw(int slice, std::string img_path, real32* raw_img, int raw_img_height, int  raw_img_width,
						 int start_height, int end_height, int start_width, int end_width, 
						 const char* img_format, int img_depth
                         ) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("img_path=%s, raw_img_height=%d, raw_img_width=%d, start_height=%d, end_height=%d, start_width=%d, end_width=%d", img_path.c_str(), raw_img_height, raw_img_width, start_height, end_height, start_width, end_width).c_str(), __iim__current__function__);

    //checking for non implemented features
	if ( strcmp(img_format,"Vaa3DRaw")!=0 && strcmp(img_format,"Tiff3D")!=0 ) { // WARNING: there is not a well defined convention yet for specify image format
		char msg[STATIC_STRINGS_SIZE];
		sprintf(msg,"in VirtualVolume::saveImage_to_Vaa3DRaw: format \"%s\" not implemented yet",img_format);
		throw IOException(msg);
	}

	uint8  *row_data_8bit;
	uint16 *row_data_16bit;
	//uint32 img_data_step;
	float scale_factor_16b, scale_factor_8b;
	int img_height, img_width;
	int i, j, k;
	char img_filepath[5000];

	// WARNING: currently supported only 8/16 bits depth by VirtualVolume::saveImage_from_UINT8_to_Vaa3DRaw
	if(img_depth != 8 && img_depth != 16)
	{
		char err_msg[STATIC_STRINGS_SIZE];		
		sprintf(err_msg,"in saveImage_to_Vaa3DRaw(..., img_depth=%d, ...): unsupported bit depth\n",img_depth);
        throw IOException(err_msg);
	}

	//setting some default parameters and image dimensions
	end_height = (end_height == -1 ? raw_img_height - 1 : end_height);
	end_width  = (end_width  == -1 ? raw_img_width  - 1 : end_width );
	img_height = end_height - start_height + 1;
	img_width  = end_width  - start_width  + 1;

	//checking parameters correctness
	if(!(start_height>=0 && end_height>start_height && end_height<raw_img_height && start_width>=0 && end_width>start_width && end_width<raw_img_width))
	{
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"in saveImage_to_Vaa3DRaw(..., raw_img_height=%d, raw_img_width=%d, start_height=%d, end_height%d, start_width=%d, end_width=%d): invalid image portion\n",
			raw_img_height, raw_img_width, start_height, end_height, start_width, end_width);
        throw IOException(err_msg);
	}

	if ( strcmp(img_format,"Vaa3DRaw")==0 ) {
		//generating complete path for image to be saved
		sprintf(img_filepath, "%s.%s", img_path.c_str(), VAA3D_SUFFIX);
	}
	else if ( strcmp(img_format,"Tiff3D")==0 ) {
		sprintf(img_filepath, "%s.%s", img_path.c_str(), "tif");
	}

	//converting raw data in image data
	scale_factor_16b = 65535.0F;
	scale_factor_8b  = 255.0F;
	if(img_depth == 8)
	{
		row_data_8bit = new uint8[img_height * img_width];
		for(i = 0, k = 0; i <img_height; i++)
		{
			for(j = 0; j < img_width; j++, k++)
				row_data_8bit[k] = (uint8) (raw_img[(i+start_height)*raw_img_width+j+start_width] * scale_factor_8b);
		}
	}
	else // img_depth = 16 (because of the check above)
	{
		row_data_16bit = new uint16[img_height * img_width];
		for(i = 0, k = 0; i <img_height; i++)
		{
			for(j = 0; j < img_width; j++, k++)
				row_data_16bit[k] = (uint16) (raw_img[(i+start_height)*raw_img_width+j+start_width] * scale_factor_16b);
		}
		row_data_8bit = (uint8 *) row_data_16bit;
	}

	if ( strcmp(img_format,"Vaa3DRaw")==0 ) {
		VirtualVolume::saveImage_from_UINT8_to_Vaa3DRaw (slice,img_path, &row_data_8bit, 1, 0, 
								raw_img_height, raw_img_width, 0, -1, 0, -1, img_format, img_depth); // the ROI coincides with the whole buffer (row_data8bit) 
	}
	else if ( strcmp(img_format,"Tiff3D")==0 ) {
		VirtualVolume::saveImage_from_UINT8_to_Tiff3D (slice,img_path, &row_data_8bit, 1, 0, 
								raw_img_height, raw_img_width, 0, -1, 0, -1, img_format, img_depth); // the ROI coincides with the whole buffer (row_data8bit) 
	}

	delete row_data_8bit;
}


/*************************************************************************************************************
* Save image method from uint8 raw data to Vaa3D raw format. <> parameters are mandatory, while [] are optional.
* <img_path>                : absolute path of image to be saved. It DOES NOT include its extension, which is
*                             provided by the [img_format] parameter.
* <raw_ch>                  : array of pointers to raw data of the channels with values in [0,255].
*                             For grayscale images raw_ch[0] is the pointer to the raw image data.
*                             For colour images raw_ch[0] is the pointer to the raw image data of the RED channel.
*                             WARNING: raw_ch elements should not be overwritten
* <n_chans>                 : number of channels (length of raw_ch).
* <offset>                  : offset to be added to raw_ch[i] to get actual data
* <raw_img_height/width>    : dimensions of raw_img.        
* [start/end_height/width]  : optional ROI (region of interest) to be set on the given image.
* [img_format]              : image format extension to be used (e.g. "tif", "png", etc.)
* [img_depth]               : image bitdepth to be used (8 or 16)
**************************************************************************************************************/
void VirtualVolume::saveImage_from_UINT8_to_Vaa3DRaw (int slice, std::string img_path, uint8** raw_ch, int n_chans, sint64 offset, 
                       int raw_img_height, int raw_img_width, int start_height, int end_height, int start_width,
                       int end_width, const char* img_format, int img_depth ) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("img_path=%s, raw_img_height=%d, raw_img_width=%d, start_height=%d, end_height=%d, start_width=%d, end_width=%d", img_path.c_str(), raw_img_height, raw_img_width, start_height, end_height, start_width, end_width).c_str(), __iim__current__function__);

    //LOCAL VARIABLES
    char buffer[STATIC_STRINGS_SIZE];
    sint64 img_height, img_width;
	sint64 img_width_b; // image width in bytes
	int img_bytes_per_chan;

	//add offset to raw_ch
	//for each channel adds to raw_ch the offset of current slice from the beginning of buffer 
	uint8** raw_ch_temp = new uint8 *[n_chans];
	for (int i=0; i<n_chans; i++)
		raw_ch_temp[i] = raw_ch[i] + offset;

    //setting some default parameters and image dimensions
    end_height = (end_height == -1 ? raw_img_height - 1 : end_height);
    end_width  = (end_width  == -1 ? raw_img_width  - 1 : end_width );
    img_height = end_height - start_height + 1;
    img_width  = end_width  - start_width  + 1;

    //checking parameters correctness
    if(!(start_height>=0 && end_height>start_height && end_height<raw_img_height && start_width>=0 && end_width>start_width && end_width<raw_img_width))
    {
        sprintf(buffer,"in saveImage_from_UINT8(..., raw_img_height=%d, raw_img_width=%d, start_height=%d, end_height%d, start_width=%d, end_width=%d): invalid image portion\n",
                raw_img_height, raw_img_width, start_height, end_height, start_width, end_width);
        throw IOException(buffer);
    }

 //if(img_depth != 8 && img_depth != 16 && n_chans == 1)
 //   {
 //       sprintf(buffer,"in saveImage_from_UINT8(..., img_depth=%d, ...): unsupported bit depth for greyscale images\n",img_depth);
 //       throw MyException(buffer);
 //   }
 //   if(img_depth != 8 && n_chans > 1)
 //   {
 //       sprintf(buffer,"in saveImage_from_UINT8(..., img_depth=%d, ...): unsupported bit depth for multi-channel images\n",img_depth);
 //       throw MyException(buffer);
 //   }
	if(img_depth != 8 && img_depth != 16 && n_chans == 1)
	{
		sprintf(buffer,"in saveImage_from_UINT8(..., img_depth=%d, ...): unsupported bit depth\n",img_depth);
        throw IOException(buffer);
	}
	img_bytes_per_chan = (img_depth == 8) ? 1 : 2;
	// all width parameters have to be multiplied by the number of bytes per channel
	img_width_b    = img_width * img_bytes_per_chan; 
	raw_img_width *= img_bytes_per_chan;
	start_width   *= img_bytes_per_chan;

	uint8 *imageData = new uint8[img_height * img_width_b * n_chans];
	for ( int c=0; c<n_chans; c++ ) {
		for(sint64 i=0; i<img_height; i++)
		{
			uint8* row_data_8bit = imageData + c*img_height*img_width_b + i*img_width_b;
			for(sint64 j=0; j<img_width_b; j++)
				row_data_8bit[j] = raw_ch_temp[c][(i+start_height)*raw_img_width + (j+start_width)];
            }
	}

    //generating complete path for image to be saved
    sprintf(buffer, "%s.%s", img_path.c_str(), VAA3D_SUFFIX);

	char *err_rawfmt;
	if ( (err_rawfmt = writeSlice2RawFile (buffer,slice,(unsigned char *)imageData,(int)img_height,(int)img_width)) != 0 ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VirtualVolume::saveImage_from_UINT8_to_Vaa3DRaw: error in saving slice %d (%lld x %lld) in file %s (writeSlice2RawFile: %s)", slice,img_height,img_width,buffer,err_rawfmt);
        throw IOException(err_msg);
	};

	delete imageData;
}

/*************************************************************************************************************
* Save image method from uint8 raw data to Tiff 3D (multiPAGE) format. <> parameters are mandatory, while [] are optional.
* <img_path>                : absolute path of image to be saved. It DOES NOT include its extension, which is
*                             provided by the [img_format] parameter.
* <raw_ch>                  : array of pointers to raw data of the channels with values in [0,255].
*                             For grayscale images raw_ch[0] is the pointer to the raw image data.
*                             For colour images raw_ch[0] is the pointer to the raw image data of the RED channel.
*                             WARNING: raw_ch elements should not be overwritten
* <n_chans>                 : number of channels (length of raw_ch).
* <offset>                  : offset to be added to raw_ch[i] to get actual data
* <raw_img_height/width>    : dimensions of raw_img.        
* [start/end_height/width]  : optional ROI (region of interest) to be set on the given image.
* [img_format]              : image format extension to be used (e.g. "tif", "png", etc.)
* [img_depth]               : image bitdepth to be used (8 or 16)
**************************************************************************************************************/
void VirtualVolume::saveImage_from_UINT8_to_Tiff3D (int slice, std::string img_path, uint8** raw_ch, int n_chans, sint64 offset, 
                       int raw_img_height, int raw_img_width, int start_height, int end_height, int start_width,
                       int end_width, const char* img_format, int img_depth ) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("img_path=%s, raw_img_height=%d, raw_img_width=%d, start_height=%d, end_height=%d, start_width=%d, end_width=%d", img_path.c_str(), raw_img_height, raw_img_width, start_height, end_height, start_width, end_width).c_str(), __iim__current__function__);

    //LOCAL VARIABLES
    char buffer[STATIC_STRINGS_SIZE];
    sint64 img_height, img_width;
	sint64 img_width_b; // image width in bytes
	int img_bytes_per_chan;

	//add offset to raw_ch
	//for each channel adds to raw_ch the offset of current slice from the beginning of buffer 
	uint8** raw_ch_temp = new uint8 *[n_chans];
	for (int i=0; i<n_chans; i++)
		raw_ch_temp[i] = raw_ch[i] + offset;

    //setting some default parameters and image dimensions
    end_height = (end_height == -1 ? raw_img_height - 1 : end_height);
    end_width  = (end_width  == -1 ? raw_img_width  - 1 : end_width );
    img_height = end_height - start_height + 1;
    img_width  = end_width  - start_width  + 1;

    //checking parameters correctness
    if(!(start_height>=0 && end_height>start_height && end_height<raw_img_height && start_width>=0 && end_width>start_width && end_width<raw_img_width))
    {
        sprintf(buffer,"in saveImage_from_UINT8(..., raw_img_height=%d, raw_img_width=%d, start_height=%d, end_height%d, start_width=%d, end_width=%d): invalid image portion\n",
                raw_img_height, raw_img_width, start_height, end_height, start_width, end_width);
        throw IOException(buffer);
    }

	if(img_depth != 8 && img_depth != 16 && n_chans == 1)
	{
		sprintf(buffer,"in saveImage_from_UINT8(..., img_depth=%d, ...): unsupported bit depth\n",img_depth);
        throw IOException(buffer);
	}
	img_bytes_per_chan = (img_depth == 8) ? 1 : 2;
	// all width parameters have to be multiplied by the number of bytes per channel
	img_width_b    = img_width * img_bytes_per_chan; 
	raw_img_width *= img_bytes_per_chan;
	start_width   *= img_bytes_per_chan;

	uint8 *imageData = new uint8[img_height * img_width_b * n_chans];
	for(sint64 i=0; i<img_height; i++)
	{
		uint8* row_data_8bit = imageData + i*img_width_b*n_chans;
		for(sint64 j=0; j<img_width_b; j++) {
			for ( int c=0; c<n_chans; c++ ) {
				row_data_8bit[j*n_chans + c] = raw_ch_temp[c][(i+start_height)*raw_img_width + (j+start_width)];
            }
		}
	}

    //generating complete path for image to be saved
    sprintf(buffer, "%s.%s", img_path.c_str(), TIFF3D_SUFFIX);

	char *err_tiff_fmt;
	if ( (err_tiff_fmt = appendSlice2Tiff3DFile(buffer,slice,(unsigned char *)imageData,(int)img_height,(int)img_width)) != 0 ) {
	//if ( (err_rawfmt = writeSlice2RawFile (buffer,slice,(unsigned char *)imageData,(int)img_height,(int)img_width)) != 0 ) {
		char err_msg[STATIC_STRINGS_SIZE];
		sprintf(err_msg,"VirtualVolume::saveImage_from_UINT8_to_Tiff3D: error in saving slice %d (%lld x %lld) in file %s (appendSlice2Tiff3DFile: %s)", slice,img_height,img_width,buffer,err_tiff_fmt);
        throw IOException(err_msg);
	};

	delete imageData;
}


/*************************************************************************************************************
* Performs downsampling at a halved frequency on the given 3D image.  The given image is overwritten in order
* to store its halvesampled version without allocating any additional resources.
**************************************************************************************************************/
void VirtualVolume::halveSample ( real32* img, int height, int width, int depth, int method )
{
	float A,B,C,D,E,F,G,H;

	// indices are sint64 because offsets can be larger that 2^31 - 1

	if ( method == HALVE_BY_MEAN ) {

		for(sint64 z=0; z<depth/2; z++)
		{
			for(sint64 i=0; i<height/2; i++)
			{
				for(sint64 j=0; j<width/2; j++)
				{
					//computing 8-neighbours
					A = img[2*z*width*height +2*i*width + 2*j];
					B = img[2*z*width*height +2*i*width + (2*j+1)];
					C = img[2*z*width*height +(2*i+1)*width + 2*j];
					D = img[2*z*width*height +(2*i+1)*width + (2*j+1)];
					E = img[(2*z+1)*width*height +2*i*width + 2*j];
					F = img[(2*z+1)*width*height +2*i*width + (2*j+1)];
					G = img[(2*z+1)*width*height +(2*i+1)*width + 2*j];
					H = img[(2*z+1)*width*height +(2*i+1)*width + (2*j+1)];

					//computing mean
					img[z*(width/2)*(height/2)+i*(width/2)+j] = (A+B+C+D+E+F+G+H)/(float)8;
				}
			}
		}

	}
	else if ( method == HALVE_BY_MAX ) {

		for(sint64 z=0; z<depth/2; z++)
		{
			for(sint64 i=0; i<height/2; i++)
			{
				for(sint64 j=0; j<width/2; j++)
				{
					//computing max of 8-neighbours
					A = img[2*z*width*height + 2*i*width + 2*j];
					B = img[2*z*width*height + 2*i*width + (2*j+1)];
					if ( B > A ) A = B;
					B = img[2*z*width*height + (2*i+1)*width + 2*j];
					if ( B > A ) A = B;
					B = img[2*z*width*height + (2*i+1)*width + (2*j+1)];
					if ( B > A ) A = B;
					B = img[(2*z+1)*width*height + 2*i*width + 2*j];
					if ( B > A ) A = B;
					B = img[(2*z+1)*width*height + 2*i*width + (2*j+1)];
					if ( B > A ) A = B;
					B = img[(2*z+1)*width*height + (2*i+1)*width + 2*j];
					if ( B > A ) A = B;
					B = img[(2*z+1)*width*height + (2*i+1)*width + (2*j+1)];
					if ( B > A ) A = B;

					//computing mean
					img[z*(width/2)*(height/2) + i*(width/2) + j] = A;
				}
			}
		}

	}
	else {
		char buffer[STATIC_STRINGS_SIZE];
		sprintf(buffer,"in halveSample(...): invalid halving method\n");
        throw IOException(buffer);
	}
	
}


void VirtualVolume::halveSample_UINT8 ( uint8** img, int height, int width, int depth, int channels, int method, int bytes_chan )
{

	float A,B,C,D,E,F,G,H;

	// indices are sint64 because offsets can be larger that 2^31 - 1

	if ( bytes_chan == 1 ) {

		if ( method == HALVE_BY_MEAN ) {   

			for(sint64 c=0; c<channels; c++)
			{
				for(sint64 z=0; z<depth/2; z++)
				{
					for(sint64 i=0; i<height/2; i++)
					{
						for(sint64 j=0; j<width/2; j++)
						{
							//computing 8-neighbours
							A = img[c][2*z*width*height + 2*i*width + 2*j];
							B = img[c][2*z*width*height + 2*i*width + (2*j+1)];
							C = img[c][2*z*width*height + (2*i+1)*width + 2*j];
							D = img[c][2*z*width*height + (2*i+1)*width + (2*j+1)];
							E = img[c][(2*z+1)*width*height + 2*i*width + 2*j];
							F = img[c][(2*z+1)*width*height + 2*i*width + (2*j+1)];
							G = img[c][(2*z+1)*width*height + (2*i+1)*width + 2*j];
							H = img[c][(2*z+1)*width*height + (2*i+1)*width + (2*j+1)];

							//computing mean
                            img[c][z*(width/2)*(height/2) + i*(width/2) + j] = (uint8) iim::round((A+B+C+D+E+F+G+H)/(float)8);
						}
					}
				}
			}
		}

		else if ( method == HALVE_BY_MAX ) {

		for(sint64 c=0; c<channels; c++)
		{
			for(sint64 z=0; z<depth/2; z++)
			{
				for(sint64 i=0; i<height/2; i++)
				{
					for(sint64 j=0; j<width/2; j++)
					{
						//computing max of 8-neighbours
						A = img[c][2*z*width*height + 2*i*width + 2*j];
						B = img[c][2*z*width*height + 2*i*width + (2*j+1)];
						if ( B > A ) A = B;
						B = img[c][2*z*width*height + (2*i+1)*width + 2*j];
						if ( B > A ) A = B;
						B = img[c][2*z*width*height + (2*i+1)*width + (2*j+1)];
						if ( B > A ) A = B;
						B = img[c][(2*z+1)*width*height + 2*i*width + 2*j];
						if ( B > A ) A = B;
						B = img[c][(2*z+1)*width*height + 2*i*width + (2*j+1)];
						if ( B > A ) A = B;
						B = img[c][(2*z+1)*width*height + (2*i+1)*width + 2*j];
						if ( B > A ) A = B;
						B = img[c][(2*z+1)*width*height + (2*i+1)*width + (2*j+1)];
						if ( B > A ) A = B;

						//computing mean
                        img[c][z*(width/2)*(height/2) + i*(width/2) + j] = (uint8) iim::round(A);
					}
				}
			}
		}

		}
		else {
			char buffer[STATIC_STRINGS_SIZE];
			sprintf(buffer,"in VirtualVolume::halveSample_UINT8(...): invalid halving method\n");
            throw IOException(buffer);
		}

	}
	else if ( bytes_chan == 2 ) {

		uint16 **img16 = (uint16 **) img;

		if ( method == HALVE_BY_MEAN ) {   

			for(sint64 c=0; c<channels; c++)
			{
				for(sint64 z=0; z<depth/2; z++)
				{
					for(sint64 i=0; i<height/2; i++)
					{
						for(sint64 j=0; j<width/2; j++)
						{
							//computing 8-neighbours
							A = img16[c][2*z*width*height + 2*i*width + 2*j];
							B = img16[c][2*z*width*height + 2*i*width + (2*j+1)];
							C = img16[c][2*z*width*height + (2*i+1)*width + 2*j];
							D = img16[c][2*z*width*height + (2*i+1)*width + (2*j+1)];
							E = img16[c][(2*z+1)*width*height + 2*i*width + 2*j];
							F = img16[c][(2*z+1)*width*height + 2*i*width + (2*j+1)];
							G = img16[c][(2*z+1)*width*height + (2*i+1)*width + 2*j];
							H = img16[c][(2*z+1)*width*height + (2*i+1)*width + (2*j+1)];

							//computing mean
                            img16[c][z*(width/2)*(height/2) + i*(width/2) + j] = (uint16) iim::round((A+B+C+D+E+F+G+H)/(float)8);
						}
					}
				}
			}
		}

		else if ( method == HALVE_BY_MAX ) {

		for(sint64 c=0; c<channels; c++)
		{
			for(sint64 z=0; z<depth/2; z++)
			{
				for(sint64 i=0; i<height/2; i++)
				{
					for(sint64 j=0; j<width/2; j++)
					{
						//computing max of 8-neighbours
						A = img16[c][2*z*width*height + 2*i*width + 2*j];
						B = img16[c][2*z*width*height + 2*i*width + (2*j+1)];
						if ( B > A ) A = B;
						B = img16[c][2*z*width*height + (2*i+1)*width + 2*j];
						if ( B > A ) A = B;
						B = img16[c][2*z*width*height + (2*i+1)*width + (2*j+1)];
						if ( B > A ) A = B;
						B = img16[c][(2*z+1)*width*height + 2*i*width + 2*j];
						if ( B > A ) A = B;
						B = img16[c][(2*z+1)*width*height + 2*i*width + (2*j+1)];
						if ( B > A ) A = B;
						B = img16[c][(2*z+1)*width*height + (2*i+1)*width + 2*j];
						if ( B > A ) A = B;
						B = img16[c][(2*z+1)*width*height + (2*i+1)*width + (2*j+1)];
						if ( B > A ) A = B;

						//computing mean
                        img[c][z*(width/2)*(height/2) + i*(width/2) + j] = (uint8) iim::round(A);
					}
				}
			}
		}

		}
		else {
			char buffer[STATIC_STRINGS_SIZE];
			sprintf(buffer,"in VirtualVolume::halveSample_UINT8(...): invalid halving method\n");
            throw IOException(buffer);
		}

	}
	else {
		char buffer[STATIC_STRINGS_SIZE];
		sprintf(buffer,"VirtualVolume::in halveSample_UINT8(...): invalid number of bytes per channel (%d)\n", bytes_chan);
        throw IOException(buffer);
	}
}

// tries to automatically detect the volume format and returns the imported volume if succeeds (otherwise returns 0)
// WARNING: all metadata files (if needed by that format) are assumed to be present. Otherwise, that format will be skipped.
VirtualVolume* VirtualVolume::instance(const char* path) throw (IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("path = \"%s\"", path).c_str(), __iim__current__function__);

    VirtualVolume* volume = 0;

    // try all directory formats
    if(isDirectory(path))
    {
        try
        {
            volume = new TiledMCVolume(path);
        }
        catch(IOException &ex)
        {
            debug(LEV3, strprintf("Cannot import <TiledMCVolume> at \"%s\": %s", path, ex.what()).c_str(),__iim__current__function__);
            try
            {
                volume = new StackedVolume(path);
            }
            catch(IOException &ex)
            {
                debug(LEV3, strprintf("Cannot import <StackedVolume> at \"%s\": %s", path, ex.what()).c_str(),__iim__current__function__);
                try
                {
                    volume = new TiledVolume(path);
                }
                catch(IOException &ex)
                {
                    debug(LEV3, strprintf("Cannot import <TiledVolume> at \"%s\": %s", path, ex.what()).c_str(),__iim__current__function__);
                    try
                    {
                        volume = new SimpleVolume(path);
                    }
                    catch(IOException &ex)
                    {
                        debug(LEV3, strprintf("Cannot import <SimpleVolume> at \"%s\": %s", path, ex.what()).c_str(),__iim__current__function__);
                        try
                        {
                            volume = new SimpleVolumeRaw(path);
                        }
                        catch(IOException &ex)
                        {
                            debug(LEV3, strprintf("Cannot import <SimpleVolumeRaw> at \"%s\": %s", path, ex.what()).c_str(),__iim__current__function__);
                            try
                            {
                                volume = new TimeSeries(path);
                            }
                            catch(IOException &ex)
                            {
                                debug(LEV3, strprintf("Cannot import <TimeSeries> at \"%s\": %s", path, ex.what()).c_str(),__iim__current__function__);
                            }
                        }
                    }
                }
            }
        }
        catch(...)
        {
            debug(LEV3, strprintf("generic error occurred when importing volume at \"%s\"", path).c_str(),__iim__current__function__);
        }
    }
    // try all file formats
    else if(isFile(path))
    {
        try
        {
            volume = new RawVolume(path);
        }
        catch(IOException &ex)
        {
            debug(LEV3, strprintf("Cannot import <RawVolume> at \"%s\": %s", path, ex.what()).c_str(),__iim__current__function__);
        }
        catch(...)
        {
            debug(LEV3, strprintf("generic error occurred when importing volume at \"%s\"", path).c_str(),__iim__current__function__);
        }
    }
    else
        throw IOException(strprintf("Path = \"%s\" does not exist", path), __iim__current__function__);

    return volume;
}

// returns the imported volume if succeeds (otherwise returns 0)
// WARNING: no assumption is made on metadata files, which are possibly (re-)generated using the additional informations provided.
VirtualVolume* VirtualVolume::instance(const char* path, std::string format,
                                       iim::axis AXS_1, iim::axis AXS_2, iim::axis AXS_3, /* = iim::axis_invalid */
                                       float VXL_1 /* = 0 */, float VXL_2 /* = 0 */, float VXL_3 /* = 0 */) throw (iim::IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("path = \"%s\", format = %s, AXS_1 = %s, AXS_2 = %s, AXS_3 = %s, VXL_1 = %.2f, VXL_2 = %.2f, VXL_3 = %.2f",
                                        path, format.c_str(), axis_to_str(AXS_1), axis_to_str(AXS_2), axis_to_str(AXS_3),
                                        VXL_1, VXL_2, VXL_3).c_str(), __iim__current__function__);

    VirtualVolume* volume = 0;

    // directory formats
    if(isDirectory(path))
    {
        if(format.compare(TILED_MC_FORMAT) == 0)
        {
            if(AXS_1 != axis_invalid && AXS_2 != axis_invalid && AXS_3 != axis_invalid && VXL_1 != 0 && VXL_2 != 0 && VXL_3 != 0)
                volume = new TiledMCVolume(path, ref_sys(AXS_1,AXS_2,AXS_3), VXL_1, VXL_2, VXL_3, true, true);
            else
                throw IOException(strprintf("Invalid parameters AXS_1(%s), AXS_2(%s), AXS_3(%s), VXL_1(%.2f), VXL_2(%.2f), VXL_3(%.2f)",
                                            axis_to_str(AXS_1), axis_to_str(AXS_2), axis_to_str(AXS_3), VXL_1, VXL_2, VXL_3).c_str());
        }
        else if(format.compare(STACKED_FORMAT) == 0)
        {
            if(AXS_1 != axis_invalid && AXS_2 != axis_invalid && AXS_3 != axis_invalid && VXL_1 != 0 && VXL_2 != 0 && VXL_3 != 0)
                volume = new StackedVolume(path, ref_sys(AXS_1,AXS_2,AXS_3), VXL_1, VXL_2, VXL_3, true, true);
            else
                throw IOException(strprintf("Invalid parameters AXS_1(%s), AXS_2(%s), AXS_3(%s), VXL_1(%.2f), VXL_2(%.2f), VXL_3(%.2f)",
                                            axis_to_str(AXS_1), axis_to_str(AXS_2), axis_to_str(AXS_3), VXL_1, VXL_2, VXL_3).c_str());
        }
        else if((format.compare(TILED_FORMAT) == 0) || (format.compare(TILED_TIF3D_FORMAT) == 0))
        {
            if(AXS_1 != axis_invalid && AXS_2 != axis_invalid && AXS_3 != axis_invalid && VXL_1 != 0 && VXL_2 != 0 && VXL_3 != 0)
                volume = new TiledVolume(path, ref_sys(AXS_1,AXS_2,AXS_3), VXL_1, VXL_2, VXL_3, true, true);
            else
                throw IOException(strprintf("Invalid parameters AXS_1(%s), AXS_2(%s), AXS_3(%s), VXL_1(%.2f), VXL_2(%.2f), VXL_3(%.2f)",
                                            axis_to_str(AXS_1), axis_to_str(AXS_2), axis_to_str(AXS_3), VXL_1, VXL_2, VXL_3).c_str());
        }
        else if(format.compare(SIMPLE_RAW_FORMAT) == 0)
            volume = new SimpleVolumeRaw(path);
        else if(format.compare(SIMPLE_FORMAT) == 0)
            volume = new SimpleVolume(path);
        else
            throw IOException(strprintf("in VirtualVolume::instance(): Unsupported format \"%s\" for path \"%s\" which is a directory", format.c_str(), path), __iim__current__function__);
    }
    // file formats
    else if(isFile(path))
    {
        if(format.compare(RAW_FORMAT) == 0)
            volume = new RawVolume(path);
        else
            throw IOException(strprintf("in VirtualVolume::instance(): Unsupported format \"%s\" for path \"%s\" which is a file", format.c_str(), path), __iim__current__function__);
    }
    else
        throw IOException(strprintf("in VirtualVolume::instance(): path = \"%s\" does not exist", path), __iim__current__function__);


    return volume;
}

// (@MOVED from TiledMCVolume.cpp by Alessandro on 2014-02-20)
void VirtualVolume::setActiveChannels ( uint32 *_active, int _n_active )
{
    /**/iim::debug(iim::LEV3, 0, __iim__current__function__);

    if ( active )
        delete[] active;
    active   = _active;
    n_active = _n_active;
}

// returns true if the given format is hierarchical, i.e. if it consists of nested folders (1 level at least)
bool VirtualVolume::isHierarchical(std::string format) throw (iim::IOException)
{
    /**/iim::debug(iim::LEV3, strprintf("format = %s", format.c_str()).c_str(), __iim__current__function__);

    if(format.compare(TILED_FORMAT) == 0)
        return true;
    else if(format.compare(TILED_MC_FORMAT) == 0)
        return true;
    else if(format.compare(STACKED_FORMAT) == 0)
        return true;
    else if(format.compare(TIME_SERIES) == 0)
        return true;
    else if(format.compare(SIMPLE_FORMAT) == 0)
        return true;
    else if(format.compare(SIMPLE_RAW_FORMAT) == 0)
        return true;
    else if(format.compare(TILED_TIF3D_FORMAT) == 0)
        return true;
    else if(format.compare(RAW_FORMAT) == 0)
        return false;
    else if(format.compare(TIF3D_FORMAT) == 0)
        return false;
    else
        throw IOException(strprintf("Unsupported format %s", format.c_str()), __iim__current__function__);

}
