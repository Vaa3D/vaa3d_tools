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

# include "VirtualVolume.h"

# include "RawFmtMngr.h"

#include <cxcore.h>
#include <highgui.h>
#include <stdio.h>



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
void VirtualVolume::saveImage(std::string img_path, REAL_T* raw_img, int raw_img_height, int  raw_img_width, 
							 int start_height, int end_height, int start_width, int end_width, 
							 const char* img_format, int img_depth) throw (MyException)
{
	#if IO_M_VERBOSE > 4
        printf("\t\t\t\tin VirtualVolume::saveImage(img_path=%s, raw_img_height=%d, raw_img_width=%d, start_height=%d, end_height=%d, start_width=%d, end_width=%d)\n",
		img_path.c_str(), raw_img_height, raw_img_width, start_height, end_height, start_width, end_width);
	#endif

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
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"in saveImage(..., raw_img_height=%d, raw_img_width=%d, start_height=%d, end_height%d, start_width=%d, end_width=%d): invalid image portion\n",
			raw_img_height, raw_img_width, start_height, end_height, start_width, end_width);
		throw MyException(err_msg);
	}
	if(img_depth != 8 && img_depth != 16)
	{
		char err_msg[IM_STATIC_STRINGS_SIZE];		
		sprintf(err_msg,"in saveImage(..., img_depth=%d, ...): unsupported bit depth\n",img_depth);
		throw MyException(err_msg);
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
	catch(std::exception ex)
	{
		char err_msg[IM_STATIC_STRINGS_SIZE];		
		sprintf(err_msg,"in saveImage(...): unable to save image at \"%s\". Unsupported format or wrong path.\n",img_filepath);
		throw MyException(err_msg);
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
                           int end_width, const char* img_format, int img_depth ) throw (MyException)
{
    #if IO_M_VERBOSE > 4
    printf("\t\t\t\tin VirtualVolume::saveImage_from_UINT8(img_path=%s, raw_img_height=%d, raw_img_width=%d, start_height=%d, end_height=%d, start_width=%d, end_width=%d)\n",
            img_path.c_str(), raw_img_height, raw_img_width, start_height, end_height, start_width, end_width);
    #endif

    //checking for non implemented features
	if( img_depth != 8 ) {
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"SimpleVolume::loadSubvolume_to_UINT8: invalid number of bits per channel (%d)",img_depth); 
		throw MyException(err_msg);
	}

    //LOCAL VARIABLES
    char buffer[IM_STATIC_STRINGS_SIZE];
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
        throw MyException(buffer);
    }
	// nchannels may be also 2
    //if(nchannels != 1 && nchannels != 3)
    if(nchannels > 3)
    {
        sprintf(buffer,"in saveImage_from_UINT8(): unsupported number of channels (= %d)\n",nchannels);
        throw MyException(buffer);
    }
    if(img_depth != 8 && img_depth != 16 && nchannels == 1)
    {
        sprintf(buffer,"in saveImage_from_UINT8(..., img_depth=%d, ...): unsupported bit depth for greyscale images\n",img_depth);
        throw MyException(buffer);
    }
    //if(img_depth != 8 && nchannels == 3) // nchannels may be also 2
    if(img_depth != 8 && nchannels > 1)
    {
        sprintf(buffer,"in saveImage_from_UINT8(..., img_depth=%d, ...): unsupported bit depth for multi-channels images\n",img_depth);
        throw MyException(buffer);
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
        char err_msg[IM_STATIC_STRINGS_SIZE];
        sprintf(err_msg,"in saveImage_from_UINT8(...): unable to save image at \"%s\". Unsupported format or wrong path.\n",buffer);
        throw MyException(err_msg);
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
void VirtualVolume::saveImage_to_Vaa3DRaw(int slice, std::string img_path, REAL_T* raw_img, int raw_img_height, int  raw_img_width, 
						 int start_height, int end_height, int start_width, int end_width, 
						 const char* img_format, int img_depth
						 ) throw (MyException)
{
	#if IO_M_VERBOSE > 4
        printf("\t\t\t\tin VirtualVolume::saveImage(img_path=%s, raw_img_height=%d, raw_img_width=%d, start_height=%d, end_height=%d, start_width=%d, end_width=%d)\n",
		img_path.c_str(), raw_img_height, raw_img_width, start_height, end_height, start_width, end_width);
	#endif

    //checking for non implemented features
	char msg[IM_STATIC_STRINGS_SIZE];
	sprintf(msg,"in VirtualVolume::saveImage_to_Vaa3DRaw: not implemented yet");
	throw MyException(msg);

	if( img_depth != 8 ) {
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"SimpleVolume::loadSubvolume_to_UINT8: invalid number of bits per channel (%d)",img_depth); 
		throw MyException(err_msg);
	}

	//uint8  *row_data_8bit;
	//uint16 *row_data_16bit;
	//uint32 img_data_step;
	//float scale_factor_16b, scale_factor_8b;
	int img_height, img_width;
	//int i, j;
	char img_filepath[5000];

	//setting some default parameters and image dimensions
	end_height = (end_height == -1 ? raw_img_height - 1 : end_height);
	end_width  = (end_width  == -1 ? raw_img_width  - 1 : end_width );
	img_height = end_height - start_height + 1;
	img_width  = end_width  - start_width  + 1;

	//checking parameters correctness
	if(!(start_height>=0 && end_height>start_height && end_height<raw_img_height && start_width>=0 && end_width>start_width && end_width<raw_img_width))
	{
		char err_msg[IM_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"in saveImage(..., raw_img_height=%d, raw_img_width=%d, start_height=%d, end_height%d, start_width=%d, end_width=%d): invalid image portion\n",
			raw_img_height, raw_img_width, start_height, end_height, start_width, end_width);
		throw MyException(err_msg);
	}
	if(img_depth != 8 && img_depth != 16)
	{
		char err_msg[IM_STATIC_STRINGS_SIZE];		
		sprintf(err_msg,"in saveImage(..., img_depth=%d, ...): unsupported bit depth\n",img_depth);
		throw MyException(err_msg);
	}

	//generating complete path for image to be saved
	sprintf(img_filepath, "%s.%s", img_path.c_str(), img_format);

	//converting raw data in image data
	//scale_factor_16b = 65535.0F;
	//scale_factor_8b  = 255.0F;
	//if(img->depth == IPL_DEPTH_8U)
	//{
	//	img_data_step = img->widthStep / sizeof(uint8);
	//	for(i = 0; i <img_height; i++)
	//	{
	//		row_data_8bit = ((uint8*)(img->imageData)) + i*img_data_step;
	//		for(j = 0; j < img_width; j++)
	//			row_data_8bit[j] = (uint8) (raw_img[(i+start_height)*raw_img_width+j+start_width] * scale_factor_8b);
	//	}
	//}
	//else
	//{
	//	img_data_step = img->widthStep / sizeof(uint16);
	//	for(i = 0; i <img_height; i++)
	//	{
	//		row_data_16bit = ((uint16*)(img->imageData)) + i*img_data_step;
	//		for(j = 0; j < img_width; j++)
	//			row_data_16bit[j] = (uint16) (raw_img[(i+start_height)*raw_img_width+j+start_width] * scale_factor_16b);
	//	}
	//}
	
	// dummy assignment: just to avoid warnings
	raw_img[slice] = 0.0;
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
                       int end_width, const char* img_format, int img_depth ) throw (MyException)
{
    #if IO_M_VERBOSE > 4
    printf("\t\t\t\tin VirtualVolume::saveImage_from_UINT8(img_path=%s, raw_img_height=%d, raw_img_width=%d, start_height=%d, end_height=%d, start_width=%d, end_width=%d)\n",
            img_path.c_str(), raw_img_height, raw_img_width, start_height, end_height, start_width, end_width);
    #endif

    //LOCAL VARIABLES
    char buffer[IM_STATIC_STRINGS_SIZE];
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
        throw MyException(buffer);
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
		throw MyException(buffer);
	}
	img_bytes_per_chan = (img_depth == 8) ? 1 : 2;
	// all width parameters have to be multiplied by the number of bytes per channel
	img_width_b    = img_width * img_bytes_per_chan; 
	raw_img_width *= img_bytes_per_chan;
	start_width   *= img_bytes_per_chan;

	uint8 *imageData = new uint8[img_height * img_width_b * n_chans];
	for ( int c=0; c<n_chans; c++ ) {
		for(int i=0; i<img_height; i++)
		{
			uint8* row_data_8bit = imageData + c*img_height*img_width_b + i*img_width_b;
			for(int j=0; j<img_width_b; j++)
				row_data_8bit[j] = raw_ch_temp[c][(i+start_height)*raw_img_width + (j+start_width)];
            }
	}

    //generating complete path for image to be saved
    sprintf(buffer, "%s.%s", img_path.c_str(), img_format);

	char *err_rawfmt;
	if ( (err_rawfmt = writeSlice2RawFile (buffer,slice,(unsigned char *)imageData,img_height,img_width)) != 0 ) {
		char err_msg[IM_STATIC_STRINGS_SIZE];
        sprintf(err_msg,"VirtualVolume::saveImage_from_UINT8_to_Vaa3DRaw: error in saving slice %d (%d x %d) in file %s (writeSlice2RawFile: %s)", slice,img_height,img_width,buffer,err_rawfmt);
		throw MyException(err_msg);
	};

	delete imageData;
}


/*************************************************************************************************************
* Performs downsampling at a halved frequency on the given 3D image.  The given image is overwritten in order
* to store its halvesampled version without allocating any additional resources.
**************************************************************************************************************/
void VirtualVolume::halveSample ( REAL_T* img, int height, int width, int depth, int method )
{
	#ifdef S_TIME_CALC
	double proc_time = -TIME(0);
	#endif

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
		char buffer[IM_STATIC_STRINGS_SIZE];
		sprintf(buffer,"in halveSample(...): invalid halving method\n");
        throw MyException(buffer);
	}
	
}


void VirtualVolume::halveSample_UINT8 ( uint8** img, int height, int width, int depth, int channels, int method, int bytes_chan ) {
	#ifdef S_TIME_CALC
	double proc_time = -TIME(0);
	#endif

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
							img[c][z*(width/2)*(height/2) + i*(width/2) + j] = (uint8) ROUND((A+B+C+D+E+F+G+H)/(float)8);
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
						img[c][z*(width/2)*(height/2) + i*(width/2) + j] = (uint8) ROUND(A);
					}
				}
			}
		}

		}
		else {
			char buffer[IM_STATIC_STRINGS_SIZE];
			sprintf(buffer,"in VirtualVolume::halveSample_UINT8(...): invalid halving method\n");
			throw MyException(buffer);
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
							img16[c][z*(width/2)*(height/2) + i*(width/2) + j] = (uint16) ROUND((A+B+C+D+E+F+G+H)/(float)8);
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
						img[c][z*(width/2)*(height/2) + i*(width/2) + j] = (uint8) ROUND(A);
					}
				}
			}
		}

		}
		else {
			char buffer[IM_STATIC_STRINGS_SIZE];
			sprintf(buffer,"in VirtualVolume::halveSample_UINT8(...): invalid halving method\n");
			throw MyException(buffer);
		}

	}
	else {
		char buffer[IM_STATIC_STRINGS_SIZE];
		sprintf(buffer,"VirtualVolume::in halveSample_UINT8(...): invalid number of bytes per channel (%d)\n", bytes_chan);
        throw MyException(buffer);
	}

	#ifdef S_TIME_CALC
	proc_time += TIME(0);
	VolumeConverter::time_multiresolution+=proc_time;
	#endif
}
