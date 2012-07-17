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

#include "IOManager.h"
#include <iostream>
#include <string>
#include <sstream>
#include <time.h>
#include <cxcore.h>
#include <cv.h>
#include <highgui.h>
#include <exception>

double IOManager::time_IO = 0;
double IOManager::time_IO_conversions = 0;


using namespace std;

/*************************************************************************************************************
* Save image method. <> parameters are mandatory, while [] are optional.
* <img_path>				: absolute path of image to be saved. It DOES NOT include its extension, which ac-
*							  tually is a preprocessor variable (see IOManager_defs.h).
* <raw_img>					: image to be saved. Raw data is in [0,1] and it is stored row-wise in a 1D array.
* <raw_img_height/width>	: dimensions of raw_img.
* [start/end_height/width]	: optional ROI (region of interest) to be set on the given image.
**************************************************************************************************************/
void IOManager::saveImage(std::string img_path,   real_t* raw_img,     int raw_img_height,  int  raw_img_width, 
						  int start_height, int end_height, int start_width, int end_width, const char* img_format,
						  int img_depth) throw (MyException)
																							
{
	#if IO_M_VERBOSE > 4
	printf("\t\t\t\tin IOManager::saveImage(img_path=%s, raw_img_height=%d, raw_img_width=%d, start_height=%d, end_height=%d, start_width=%d, end_width=%d)\n",
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
		char err_msg[IO_STATIC_STRINGS_SIZE];
		sprintf(err_msg,"in saveImage(..., raw_img_height=%d, raw_img_width=%d, start_height=%d, end_height%d, start_width=%d, end_width=%d): invalid image portion\n",
			raw_img_height, raw_img_width, start_height, end_height, start_width, end_width);
		throw MyException(err_msg);
	}
	if(img_depth != 8 && img_depth != 16)
	{
		char err_msg[IO_STATIC_STRINGS_SIZE];		
		sprintf(err_msg,"in saveImage(..., img_depth=%d, ...): unsupported bit depth\n",img_depth);
		throw MyException(err_msg);
	}

	//generating complete path for image to be saved
	sprintf(img_filepath, "%s.%s", img_path.c_str(), img_format);

	#ifdef IO_TIME_CALC
	double proctime = -TIME(0);
	#endif

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
				row_data_8bit[j] = raw_img[(i+start_height)*raw_img_width+j+start_width] * scale_factor_8b;
		}
	}
	else
	{
		img_data_step = img->widthStep / sizeof(uint16);
		for(i = 0; i <img_height; i++)
		{
			row_data_16bit = ((uint16*)(img->imageData)) + i*img_data_step;
			for(j = 0; j < img_width; j++)
				row_data_16bit[j] = raw_img[(i+start_height)*raw_img_width+j+start_width] * scale_factor_16b;
		}
	}

	#ifdef IO_TIME_CALC
	proctime += TIME(0);
	IOManager::time_IO_conversions+=proctime;
	proctime = -TIME(0);
	#endif

	//saving image
	try{cvSaveImage(img_filepath, img);}
	catch(std::exception ex)
	{
		char err_msg[IO_STATIC_STRINGS_SIZE];		
		sprintf(err_msg,"in saveImage(...): unable to save image at \"%s\". Unsupported format or wrong path.\n",img_filepath);
		throw MyException(err_msg);
	}

	#ifdef IO_TIME_CALC
	proctime += TIME(0);
	IOManager::time_IO+=proctime;
	#endif

	//releasing memory of img
	cvReleaseImage(&img);
}

/*************************************************************************************************************
* Load image stack method.	  <> parameters are mandatory, while [] are optional.
* <image_filenames>			: array of C-string storing image filenames
* <image_filepaths_size>	: size of <image_filenames>
* [base_path]				: if given, it is concatenated to obtain [base_path]/<image_filenames>[i] which is
*							  the absolute path of each image.
* [first/last_file]			: if given, only images in the range [first_file,last_file] will be loaded.
**************************************************************************************************************/
real_t* IOManager::loadImageStack(char **image_filenames,	     int image_filepaths_size, char    *base_path, 
												 int first_file, int last_file)			   throw (MyException)
{
	#if IO_M_VERBOSE > 4
	printf("\t\t\t\tin IOManager::loadImageStack(..., first_file=%d, last_file=%d)\n",first_file, last_file);
	#endif

	//LOCAL VARIABLES
	char slice_fullpath[1000];
	IplImage *slice_img_i;
	uint8  *slice_row_data_8bit;
	uint16 *slice_row_data_16bit;
	uint32 slice_data_step;
	float scale_factor_16b, scale_factor_8b;
	real_t *image_stack = NULL;
	real_t *raw_data_ptr;
	int image_stack_width, image_stack_height;
	int file_i, i, j, k;

	//initializations
	first_file = (first_file == -1 ? 0						  : first_file);
	last_file  = (last_file  == -1 ? image_filepaths_size - 1 : last_file );
	scale_factor_16b = 65535.0F;
	scale_factor_8b  = 255.0F;
	
	for(file_i = first_file, k = 0; file_i <= last_file; file_i++, k++)
	{
		//building absolute image path
		if(base_path)
			sprintf(slice_fullpath, "%s/%s", base_path, image_filenames[file_i]);
		else
			strcpy(slice_fullpath, base_path);
			
		#ifdef IO_TIME_CALC
		double proctime = -TIME(0);
		#endif

		//loading image
		slice_img_i = cvLoadImage(slice_fullpath, CV_LOAD_IMAGE_GRAYSCALE | CV_LOAD_IMAGE_ANYDEPTH);
		if(!slice_img_i)
		{
			char msg[1000];
			sprintf(msg,"in IOManager::loadImageStack(...): unable to open image \"%s\". Possible unsupported format or it isn't an image.\nSupported formats are BMP, DIB, JPEG, JPG, JPE, PNG, PBM, PGM, PPM, SR, RAS, TIFF, TIF", slice_fullpath);
			throw MyException(msg);
		}		

		#ifdef IO_TIME_CALC
		proctime += TIME(0);
		IOManager::time_IO+=proctime;
		proctime = -TIME(0);
		#endif

		//if this is the first image in the stack, storing <image_stack> dimensions and allocating memory for <image_stack>
		if(!image_stack)
		{
			image_stack_height = slice_img_i->height;
			image_stack_width  = slice_img_i->width;
			image_stack = new real_t[image_stack_width * image_stack_height * (last_file-first_file+1)];
		}
		else
			if(image_stack_width != slice_img_i->width || image_stack_height != slice_img_i->height)
				throw MyException("in IOManager::loadImageStack(...): images in stack have not the same dimensions");

		//storing data into <image_stack>
		if(slice_img_i->depth == IPL_DEPTH_8U)
		{
			slice_data_step = slice_img_i->widthStep / sizeof(uint8);
			raw_data_ptr = &image_stack[k*image_stack_height*image_stack_width];
			for(i = 0; i <image_stack_height; i++)
			{
				slice_row_data_8bit = ((uint8*)(slice_img_i->imageData)) + i*slice_data_step;
				for(j = 0; j < image_stack_width; j++, raw_data_ptr++)
					*raw_data_ptr = slice_row_data_8bit[j] / scale_factor_8b;
			}
		}
		else if(slice_img_i->depth == IPL_DEPTH_16U)
		{
			slice_data_step = slice_img_i->widthStep / sizeof(uint16);
			raw_data_ptr = &image_stack[k*image_stack_height*image_stack_width];
			for(i = 0; i <image_stack_height; i++)
			{
				slice_row_data_16bit = ((uint16*)(slice_img_i->imageData)) + i*slice_data_step;
				for(j = 0; j < image_stack_width; j++, raw_data_ptr++)
					*raw_data_ptr = slice_row_data_16bit[j] / scale_factor_16b;
			}
		}
		else
			throw MyException("in IOManager::loadImageStack(...): unsupported image depth");
		
		//releasing image
		cvReleaseImage(&slice_img_i);

		#ifdef IO_TIME_CALC
		proctime += TIME(0);
		IOManager::time_IO_conversions+=proctime;
		#endif
	}

	return image_stack;
}
