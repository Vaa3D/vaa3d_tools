/*
* Copyright (C) 2014 by  Fernando Amat
* See license.txt for full license and copyright notice.
*
* Authors: Fernando Amat
*  klb_ROI.h
*
*  Created on: October 2nd, 2014
*      Author: Fernando Amat
*
* \brief Region of interest. Basic class to handle sub-regions (slices, blocks, etc) within images
*/

#ifndef __KLB_ROI_H__
#define __KLB_ROI_H__

#include <cstdint>

#include "klb_imageHeader.h"

#if defined(COMPILE_SHARED_LIBRARY) && defined(_MSC_VER)
class __declspec(dllexport) klb_ROI
#else
class klb_ROI
#endif
{
public:
	std::uint32_t xyzctLB[KLB_DATA_DIMS];//lower bound (upper left corner) of the box (including this limit)
	std::uint32_t xyzctUB[KLB_DATA_DIMS];//upper bound (bottom right corner) of the box (including this limit)


	void defineSlice(int val, int dim, const std::uint32_t xyzct[KLB_DATA_DIMS]);//generates a slice along dimension dim
	void defineFullImage(const std::uint32_t xyzct[KLB_DATA_DIMS]);//ROI = image
	std::uint32_t getSizePixels(int dim){ return (xyzctUB[dim] - xyzctLB[dim] + 1); };
	std::uint32_t getSizePixels(int dim) const { return (xyzctUB[dim] - xyzctLB[dim] + 1); };
	std::uint64_t getSizePixels() const
	{
		std::uint64_t size = xyzctUB[0] - xyzctLB[0] + 1;
		for (int ii = 0; ii < KLB_DATA_DIMS; ii++)
			size *= (xyzctUB[ii] - xyzctLB[ii] + 1);
		return size;
	};
	
protected:

private:	
	
};


#endif //end of __KLB_ROI_H__