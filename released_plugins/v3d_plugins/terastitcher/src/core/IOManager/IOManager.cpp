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
*       Bria, A., Iannello, G., "TeraStitcher - A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Microscopy Images", (2012) BMC Bioinformatics, 13 (1), art. no. 316.
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
#include <stdio.h>

using namespace std;
using namespace iomanager;


// initialize static variables
double IOManager::time_IO = 0;
double IOManager::time_IO_conversions = 0;


// initialize namespace parameters
namespace iomanager
{
    /*******************
    *    PARAMETERS    *
    ********************
    ---------------------------------------------------------------------------------------------------------------------------*/
    int DEBUG = NO_DEBUG;           // debug level
    bool TIME_CALC = true;          // whether to enable time measurements
    int CHANNEL_SELECTION = ALL;    // channel to be loaded (default is ALL)
    /*-------------------------------------------------------------------------------------------------------------------------*/
}

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
    /**/iom::debug(iom::LEV3, strprintf("img_path=%s, raw_img_height=%d, raw_img_width=%d, start_height=%d, end_height=%d, start_width=%d, end_width=%d, img_format = %s, img_depth =%d",
                                        img_path.c_str(), raw_img_height, raw_img_width, start_height, end_height, start_width, end_width, img_format, img_depth).c_str(), __iom__current__function__);


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
        throw MyException(strprintf("in saveImage(..., raw_img_height=%d, raw_img_width=%d, start_height=%d, end_height%d, start_width=%d, end_width=%d): invalid image portion\n",
                                  raw_img_height, raw_img_width, start_height, end_height, start_width, end_width).c_str());
	if(img_depth != 8 && img_depth != 16)
        throw MyException(strprintf("in saveImage(..., img_depth=%d, ...): unsupported bit depth\n",img_depth).c_str());

	//generating complete path for image to be saved
	sprintf(img_filepath, "%s.%s", img_path.c_str(), img_format);

    // convert raw data in image data
    double proctime = -TIME(0);
    cv::Mat image(img_height, img_width, img_depth == 8 ? CV_8U : CV_16U, cv::Scalar(0));
    if(img_depth == 8)
    {
		for(i = 0; i <img_height; i++)
		{
            uint8* img_data = image.ptr<uint8>(i);
			for(j = 0; j < img_width; j++)
                img_data[j] = raw_img[(i+start_height)*raw_img_width+j+start_width] * 255.0f + 0.5f;
		}
	}
	else
	{
		for(i = 0; i <img_height; i++)
		{
            uint16* img_data = image.ptr<uint16>(i);
			for(j = 0; j < img_width; j++)
                img_data[j] = raw_img[(i+start_height)*raw_img_width+j+start_width] * 65535.0f + 0.5f;
		}
	}

    if(TIME_CALC)
    {
        proctime += TIME(0);
        IOManager::time_IO_conversions+=proctime;
        proctime = -TIME(0);
    }

    // save image
    try{cv::imwrite(img_filepath, image);}
	catch(std::exception ex)
	{
        throw MyException(strprintf("in saveImage(...): unable to save image at \"%s\". Unsupported format or wrong path.\n",img_filepath).c_str());
	}

    if(TIME_CALC)
    {
        proctime += TIME(0);
        IOManager::time_IO+=proctime;
    }
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
    /**/iom::debug(iom::LEV3, strprintf("image_filepaths_size = %d, base_path = %s, first_file=%d, last_file=%d",
                                        image_filepaths_size, base_path ? base_path : "null", first_file, last_file).c_str(), __iom__current__function__);

	//LOCAL VARIABLES
	char slice_fullpath[1000];
	real_t *image_stack = NULL;
	int image_stack_width, image_stack_height;
	int file_i, i, j, k;

	//initializations
	first_file = (first_file == -1 ? 0						  : first_file);
	last_file  = (last_file  == -1 ? image_filepaths_size - 1 : last_file );
	first_file = std::min(first_file, image_filepaths_size-1);
	last_file = std::min(last_file, image_filepaths_size-1);
	
	for(file_i = first_file, k = 0; file_i <= last_file; file_i++, k++)
	{
		//building absolute image path
		if(base_path)
			sprintf(slice_fullpath, "%s/%s", base_path, image_filenames[file_i]);
		else
			strcpy(slice_fullpath, base_path);
			

        // load image
        double proctime = -TIME(0);
        cv::Mat image;
        if(CHANNEL_SELECTION == ALL)
            image = cv::imread(slice_fullpath, CV_LOAD_IMAGE_GRAYSCALE | CV_LOAD_IMAGE_ANYDEPTH);   // pack all channels into grayscale
        else
            image = cv::imread(slice_fullpath, CV_LOAD_IMAGE_ANYCOLOR);                             // load individual channels
        if(!image.data)
            throw MyException(strprintf("in IOManager::loadImageStack(...): unable to open image \"%s\". Possible unsupported format or it isn't an image.\nSupported formats are BMP, DIB, JPEG, JPG, JPE, PNG, PBM, PGM, PPM, SR, RAS, TIFF, TIF", slice_fullpath).c_str());

        // select channel (if requested)
        if(CHANNEL_SELECTION != ALL)
        {
            cv::Mat imageChannels[3];
            cv::split(image, imageChannels);
            image = imageChannels[3-CHANNEL_SELECTION]; // OpenCV uses BGR !
        }

        if(TIME_CALC)
        {
            proctime += TIME(0);
            IOManager::time_IO+=proctime;
            proctime = -TIME(0);
        }

        // if this is the first image in the stack, store <image_stack> dimensions and allocate memory for <image_stack>
		if(!image_stack)
		{
            image_stack_height = image.rows;
            image_stack_width  = image.cols;
			image_stack = new real_t[image_stack_width * image_stack_height * (last_file-first_file+1)];
		}
        else if(image_stack_width != image.cols || image_stack_height != image.rows)
            throw MyException("in IOManager::loadImageStack(...): images in stack have not the same dimensions");

        // scan image with pointers
        real_t *raw_data = &image_stack[k*image.rows*image.cols];
        if(image.depth() == CV_8U)
        {
            for(int i=0; i<image.rows; i++)
            {
                uint8* img_data = image.ptr<uint8>(i);
                for(int j=0; j<image.cols; j++, raw_data++)
                    *raw_data = img_data[j]/255.0f;
            }
        }
        else if (image.depth() == CV_16U)
        {
            for(int i=0; i<image.rows; i++)
            {
                uint16* img_data = image.ptr<uint16>(i);
                for(int j=0; j<image.cols; j++, raw_data++)
                    *raw_data = img_data[j]/65535.0f;
            }
        }
		else
			throw MyException("in IOManager::loadImageStack(...): unsupported image depth");
		
        if(TIME_CALC)
        {
            proctime += TIME(0);
            IOManager::time_IO_conversions+=proctime;
        }
	}

	return image_stack;
}
