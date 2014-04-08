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

#ifndef _IO_MANAGER_H
#define _IO_MANAGER_H

#include <iostream>
#include <string>
#include <vector>
#include "IOManager_defs.h"
#include "MyException.h"

//STATIC class
class iomanager::IOManager
{
	private:

        IOManager(void){}
        ~IOManager(void){}

	public:

		//******CLASS MEMBERS******
		static double time_IO;					//time employed in Input/Output 
		static double time_IO_conversions;		//time employed in converting raw data to/from image data

		/*************************************************************************************************************
		* Save image method.		  <> parameters are mandatory, while [] are optional.
		* <img_path>				: absolute path of image to be saved. It DOES NOT include its extension, which ac-
		*							  tually is a preprocessor variable (see IOManager_defs.h).
		* <raw_img>					: image to be saved. Raw data is in [0,1] and it is stored row-wise in a 1D array.
		* <raw_img_height/width>	: dimensions of raw_img.
		* [start/end_height/width]	: optional ROI (region of interest) to be set on the given image.
		**************************************************************************************************************/
        static void saveImage(std::string img_path,   real_t* raw_img,       int raw_img_height,   int   raw_img_width,
							  int start_height = 0,   int end_height = - 1,  int start_width = 0,  int end_width = - 1,
                              const char* img_format = iom::DEF_IMG_FORMAT.c_str(),	 int img_depth = iom::DEF_IMG_DEPTH)
																								   throw (MyException);

		/*************************************************************************************************************
		* Load image stack method.	  <> parameters are mandatory, while [] are optional.
		* <image_filenames>			: array of C-string storing image filenames
		* <image_filepaths_size>	: size of <image_filenames>
		* [base_path]				: if given, it is concatenated to obtain [base_path]/<image_filenames>[i] which is
		*							  the absolute path of each image.
		* [first/last_file]			: if given, only images in the range [first_file,last_file] will be loaded.
		**************************************************************************************************************/
        static real_t* loadImageStack(char **image_filenames,	int image_filepaths_size, char *base_path = NULL,
									  int first_file = -1, int last_file = -1)				   throw    (MyException);
};

#endif
