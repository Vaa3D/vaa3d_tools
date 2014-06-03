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

#include "VirtualStack.h"
#include <highgui.h>
#include <cv.h>


void VirtualStack::print()
{
	printf("\t |\t[%d,%d]\n", ROW_INDEX, COL_INDEX);
	printf("\t |\tDirectory:\t\t%s\n", DIR_NAME);
	printf("\t |\tDimensions:\t\t%d(V) x %d(H) x %d(D)\n", HEIGHT, WIDTH, DEPTH);
	printf("\t |\tAbsolute position:\t%d(V) , %d(H) , %d(D)\n", ABS_V, ABS_H, ABS_D);
	printf("\t |\n");
}

//show the selected slice with a simple GUI
void VirtualStack::show(int D_index, int window_HEIGHT, int window_WIDTH)
{
	#if VM_VERBOSE > 2
	printf("\t\t\tin Stack::show(D_index=%d, win_height=%d, win_width=%d)\n", D_index, window_HEIGHT, window_WIDTH);
	#endif

	//converting selected slice of vol (1-D array of REAL_T) into a CvMat
	CvMat *slice = cvCreateMat(HEIGHT, WIDTH, CV_32FC1);
	for(int i=0; i<slice->rows; i++)
	{
		float *row_ptr = (float*)(slice->data.ptr+slice->step*i);
		for(int j=0; j<slice->cols; j++)
			row_ptr[j] = STACKED_IMAGE[D_index*HEIGHT*WIDTH + i*WIDTH +j];
	}

	//showing slice
	CvSize window_dims;
	window_dims.height = window_HEIGHT ? window_HEIGHT : HEIGHT;
	window_dims.width  = window_WIDTH  ? window_WIDTH  : WIDTH;
	char buffer[200];
	CvMat* mat_rescaled = cvCreateMat(HEIGHT, WIDTH, CV_16UC1);
	IplImage* image_resized = cvCreateImage(window_dims, IPL_DEPTH_16U, 1);
	IplImage* img_buffer = cvCreateImageHeader(window_dims, IPL_DEPTH_16U, 1);

	cvConvertScale(slice,mat_rescaled, 65535);
	cvResize(cvGetImage(mat_rescaled,img_buffer), image_resized, CV_INTER_CUBIC);

	sprintf(buffer,"SLICE %d of volume %d x %d",D_index, HEIGHT, WIDTH);
	cvNamedWindow(buffer,1);
	cvShowImage(buffer,image_resized);
	cvMoveWindow(buffer, 10,10);
	while(1)
	{
		if(cvWaitKey(100)==27) break;
	}

	cvDestroyWindow(buffer);

	cvReleaseMat(&slice);
	cvReleaseMat(&mat_rescaled);
	cvReleaseImage(&image_resized);
	cvReleaseImageHeader(&img_buffer);
}