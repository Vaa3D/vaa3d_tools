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

# include "VirtualVolume.h"

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

    //LOCAL VARIABLES
    char buffer[IM_STATIC_STRINGS_SIZE];
    IplImage* img = 0;
    int img_height, img_width;
    int nchannels = 0;

    //detecting the number of channels
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
    if(nchannels != 1 && nchannels != 3)
    {
        sprintf(buffer,"in saveImage_from_UINT8(): unsupported number of channels (= %d)\n",nchannels);
        throw MyException(buffer);
    }
    if(img_depth != 8 && img_depth != 16 && nchannels == 1)
    {
        sprintf(buffer,"in saveImage_from_UINT8(..., img_depth=%d, ...): unsupported bit depth for greyscale images\n",img_depth);
        throw MyException(buffer);
    }
    if(img_depth != 8 && nchannels == 3)
    {
        sprintf(buffer,"in saveImage_from_UINT8(..., img_depth=%d, ...): unsupported bit depth for 3-channels images\n",img_depth);
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
* Performs downsampling at a halved frequency on the given 3D image.  The given image is overwritten in order
* to store its halvesampled version without allocating any additional resources.
**************************************************************************************************************/
void VirtualVolume::halveSample ( REAL_T* img, int height, int width, int depth, int method )
{
	#ifdef S_TIME_CALC
	double proc_time = -TIME(0);
	#endif

	float A,B,C,D,E,F,G,H;

	if ( method == HALVE_BY_MEAN ) {

		for(int z=0; z<depth/2; z++)
		{
			for(int i=0; i<height/2; i++)
			{
				for(int j=0; j<width/2; j++)
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

		for(int z=0; z<depth/2; z++)
		{
			for(int i=0; i<height/2; i++)
			{
				for(int j=0; j<width/2; j++)
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


void VirtualVolume::halveSample_UINT8 ( uint8** img, int height, int width, int depth, int channels, int method ) {
	#ifdef S_TIME_CALC
	double proc_time = -TIME(0);
	#endif

	float A,B,C,D,E,F,G,H;

	if ( method == HALVE_BY_MEAN ) {

		for(int c=0; c<channels; c++)
		{
			for(int z=0; z<depth/2; z++)
			{
				for(int i=0; i<height/2; i++)
				{
					for(int j=0; j<width/2; j++)
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

	for(int c=0; c<channels; c++)
	{
		for(int z=0; z<depth/2; z++)
		{
			for(int i=0; i<height/2; i++)
			{
				for(int j=0; j<width/2; j++)
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
		sprintf(buffer,"in halveSample_UINT8(...): invalid halving method\n");
        throw MyException(buffer);
	}

	#ifdef S_TIME_CALC
	proc_time += TIME(0);
	VolumeConverter::time_multiresolution+=proc_time;
	#endif
}
