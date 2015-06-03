/*
* Copyright (C) 2014 by  Fernando Amat
* See license.txt for full license and copyright notice.
*
* Authors: Fernando Amat
*  klb_ROI.cpp
*
*  Created on: October 2nd, 2014
*      Author: Fernando Amat
*
* \brief Region of interest. Basic class to handle sub-regions (slices, blocks, etc) within images
*/

#include <limits>
#include <assert.h>

#include "klb_ROI.h"

using namespace std;

void klb_ROI::defineSlice(int val, int dim, const std::uint32_t xyzct[KLB_DATA_DIMS])
{
	defineFullImage(xyzct);

	assert(dim < KLB_DATA_DIMS); 	
	//set plane
	xyzctLB[dim] = val;
	xyzctUB[dim] = val;
}

//===============================================
void klb_ROI::defineFullImage(const std::uint32_t xyzct[KLB_DATA_DIMS])
{
	memset(xyzctLB, 0, sizeof(uint32_t)* KLB_DATA_DIMS);
	for (int ii = 0; ii < KLB_DATA_DIMS; ii++)
	{
		xyzctUB[ii] = xyzct[ii] - 1;//UB is inlcuded in ROI
	}
}

