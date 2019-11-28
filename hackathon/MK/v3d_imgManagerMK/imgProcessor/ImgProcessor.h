//------------------------------------------------------------------------------
// Copyright (c) 2018 Hsienchi Kuo (Allen Institute, Hanchuan Peng's team)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  This library intends to fascilitate Vaa3D image operations in the lowest level.
*  ImgProcessor class only takes array pointers as the input and as well outputs array pointers. 
*  It does not involve in any image I/O and only operates arrays in the memory.
*  
*  Most ImgProcessor class methods are implemented as static template functions.   
*  A typical function call would need at least three input arguments:
*
*		ImgProcessor::func(unsigned char[] inputImgArray, unsigned char[] outputImgArray, int[] inputImgDimensions, other input arguments);
*
********************************************************************************/

#ifndef IMGPROCESSOR_H
#define IMGPROCESSOR_H

#include <string>
#include <cmath>
#include <algorithm>

#include "integratedDataStructures.h"

using namespace std;
using namespace integratedDataStructures;

#define getMax(a, b) ((a) >= (b)? (a):(b))
#define getMin(a, b) ((a) <= (b)? (a):(b))

class ImgProcessor 
{
public:
	virtual ~ImgProcessor() = default;

	/***************** Basic Image Operations *****************/
	template<class T>
	static inline T getPixValue(const T inputImgPtr[], const int imgDims[], const int x, const int y, const int z = 1);

	template<class T> // Between 2 input images, pixel-wisely pick the one with greater value. 
	static inline void imgMax(const T inputPtr1[], const T inputPtr2[], T outputPtr[], const int imgDims[]);

	template<class T> // Same as above, vector version overload.
	static inline void imgMax(const vector<T>* inputImgPtr1, const T inputImgPtr2[], T outputImgPtr[], const int imgDims[]);

	template<class T> // Subtracts the input image with a constant.
	static inline void imgSubtraction_const(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], const int subFactor);

	static inline void imgDotMultiply(const unsigned char inputImgPtr1[], const unsigned char inputImgPtr2[], unsigned char outputImgPtr[], const int imgDims[]);
	

	template<class T> // Cops image with specified ROI coordinates. (xlb = x lower bound; yhb = y higher bound, and so on)
	static inline void cropImg(const T InputImagePtr[], T OutputImagePtr[], 
		const int xlb, const int xhb, const int ylb, const int yhb, const int zlb, const int zhb, const int imgDims[]);

	template<class T> // Inverts 8-bit images, i.e., dynamic range = [0, 255].
	static inline void invert8bit(T input1D[], T output1D[]);

	template<class T1, class T2> // -> to be revised into 3D general form
	static inline void flipY2D(T1 input1D[], T1 output1D[], T2 xLength, T2 yLength);

	template<class T> // Slices input image stack into a series of 2D slices.
	static inline void imgStackSlicer(const T inputImgPtr[], vector<vector<T>>& outputSlices, const int imgDims[]);


	template<class T> // Ordinary downsampling method for 2D images with given downsampling factor for both x and y dimensions.
	static inline void imgDownSample2D(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], const int downSampFactor);

	template<class T> // Ordinary downsampling method for 3D images with given downsampling factor set for x, y, and z dimensions.
	static inline void imgDownSample(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], const int downSampFactors[]);

	template<class T> // Maximum downsampling method for 2D images with given downsampling factor for both x and y dimensions.
	static inline void imgDownSample2DMax(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], const int downSampFactor);

	template<class T> // Maximum downsampling method for 3D images with given dowmsampling factor set for x, y, and z dimensions.
	static inline void imgDownSampleMax(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], const int downSampFactors[]);

	template<class T> // Produces MIP image for the input image slice series.
	static inline void maxIPSeries(const vector<vector<T>> inputSlicePtrs, T outputImgPtr[], const int imgDims[]);


	template<class T> // Converts 1D vector of a 2D image into 2D array.
	static inline void slice1Dvector2_2Darray(const vector<T>& inputSliceVec, T* outputSlice2Dptr[], const int imgDims[]);

	template<class T> // Converts 2D image aray into 1D array.
	static inline void slice2Dto1D(T* inputImgPtr[], T outputImgPtr[], const int imgDims[]); // inputImgPtr[x][] cannot be guaranteed a constant here.
	/**********************************************************/



	/***************** Image Statistics *****************/
	template<class T> // Gets basic stats (sum, mean, std, var, median) out of the input image.
	static inline map<string, float> getBasicStats(const T inputImgPtr[], const int imgDims[]);

	template<class T> // Gets basic stats (sum, mean, std, var, median) out of the input image with all 0 value pixel/voxel excluded.
	static inline map<string, float> getBasicStats_no0(const T inputImgPtr[], const int imgDims[]);

	template<class T> // Outputs a map representing the input image histogram with bins as the keys and frequencies as the values.
	static inline map<int, size_t> histQuickList(const T inputImgPtr[], const int imgDims[]);

	// Gets basic stats (sum, mean, std, var, median) out of the histogram of the input image.
	static map<string, float> getBasicStats_fromHist(const map<int, size_t>& inputHistList);

	// Gets basic stats (sum, mean, std, var, median) out of the histogram of the input image with all 0 value pixel/voxel excluded.
	static map<string, float> getBasicStats_no0_fromHist(const map<int, size_t>& inputHistList);
	/****************************************************/



	/***************** Image Processing/Filtering *****************/
	template<class T> // Thresholds the input image with specified thresholding value.
	static inline void simpleThresh(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], const int threshold);

	template<class T> // Thresholds out all values that are HIGHER than the specified thresholding value.
	static inline void simpleThresh_reverse(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], const int threshold);

	template<class T> // > threshold => 255; <= threshold => 0
	static inline void imgMasking(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], const int threshold);

	template<class T> // Adaptive thresholding with specified stepsize and samping rate.
	static inline void simpleAdaThre(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], const int stepSize, const int sampRate);
	

	template<class T> // Gamma correction with cut off intensity using stepped multiplying factor, i.e., 4 => (4 - cutoff) * 4; 10 => (10 - cutoff) * 10, etc.
	static inline void stepped_gammaCorrection(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], int cutoffIntensity = 0);

	template<class T> // NOT IN USE; MAY BE DEPRECATED LATER.
	static inline void gammaCorrect_old(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], int starting_intensity = 0);

	template<class T> // Reversed gamma correction; will be re-implemented.
	static inline void reversed_gammaCorrect_eqSeriesFactor(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], int starting_intensity = 255);

	// Histogram equalization on 8 bit input images.
	// nonZero = true => only use non zero values to equalize; nonZero = false => use all values to equalize.
	template<class T>  
	static inline void histEqual_unit8(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], bool noZero = true);
	/**************************************************************/



	/********* Morphological Operations *********/
	// Skeletonizes the input 2D image and ouputs its corresponding skeleton image.
	static void skeleton2D(const unsigned char inputImgPtr[], unsigned char outputImgPtr[], const int imgDims[]);

	static void erode2D(const unsigned char inputImgPtr[], unsigned char outputImgPtr[], const int imgDims[], const morphStructElement2D& structEle);	
	// 2D erosion with specified structing element (morphStractElement2D) only for intensity <= threshold.
	static void conditionalErode2D_imgStats(const unsigned char inputImgPtr[], unsigned char outputImgPtr[], const int imgDims[], const morphStructElement2D& structEle, const int threshold);
	
	static void dilate2D(const unsigned char inputImgPtr[], unsigned char outputImgPtr[], const int imgDims[], const morphStructElement2D& structEle);

	static void imgClose2D(const unsigned char inputImgPtr[], unsigned char outputImgPtr[], const int imgDims[], const morphStructElement2D& structEle);
	static void imgOpen2D(const unsigned char inputImgPtr[], unsigned char outputImgPtr[], const int imgDims[], const morphStructElement2D& structEle);
	
	static void imgClose3D_sliceBySlice(const unsigned char inputImgPtr[], unsigned char outputImgPtr[], const int imgDims[], const morphStructElement2D& structEle);
	static void imgOpen3D_sliceBySlice(const unsigned char inputImgPtr[], unsigned char outputImgPtr[], const int imgDims[], const morphStructElement2D& structEle);
	/********************************************/



	/***************** Other utilities *****************/
	static void shapeMask2D(int imgDims[2], unsigned char outputMask1D[], int coords[2], int regionDims[2], string shape = "square");
	/***************************************************/
};

// ========================================= BASIC IMAGE OPERATION =========================================
template<class T>
inline T ImgProcessor::getPixValue(const T inputImgPtr[], const int imgDims[], const int x, const int y, const int z)
{
	size_t pix1Dindex = size_t((z - 1) * imgDims[0] * imgDims[1] + (y - 1) * imgDims[0] + x);
	return inputImgPtr[pix1Dindex];
}

template<class T>
inline void ImgProcessor::imgSubtraction_const(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], const int subFactor)
{
	size_t totalPixNum = size_t(imgDims[0] * imgDims[1] * imgDims[2]);
	for (size_t i = 0; i < totalPixNum; ++i)
	{
		if (int(inputImgPtr[i]) - subFactor <= 0) outputImgPtr[i] = 0;
		else outputImgPtr[i] = T(int(inputImgPtr[i] - subFactor));
	}
}

inline void ImgProcessor::imgDotMultiply(const unsigned char inputImgPtr1[], const unsigned char inputImgPtr2[], unsigned char outputImgPtr[], const int imgDims[])
{
	size_t totalPixNum = imgDims[0] * imgDims[1] * imgDims[2];
	for (size_t i = 0; i < totalPixNum; ++i)
	{
		int value = int(inputImgPtr1[i]) * int(inputImgPtr2[i]);
		if (value > 255) value = 255;
		outputImgPtr[i] = (unsigned char)(value);
	}
}

template<class T>
inline void ImgProcessor::cropImg(const T InputImagePtr[], T OutputImagePtr[], 
	const int xlb, const int xhb, const int ylb, const int yhb, const int zlb, const int zhb, const int imgDims[])
{
	long int OutputArrayi = 0;
	for (int zi = zlb; zi <= zhb; ++zi)
	{
		for (int yi = ylb; yi <= yhb; ++yi)
		{
			for (int xi = xlb; xi <= xhb; ++xi)
			{
				OutputImagePtr[OutputArrayi] = InputImagePtr[imgDims[0] * imgDims[1] * (zi - 1) + imgDims[0] * (yi - 1) + (xi - 1)];
				++OutputArrayi;
			}
		}
	}
}

template<class T>
inline void ImgProcessor::imgMax(const T inputPtr1[], const T inputPtr2[], T outputPtr[], const int imgDims[])
{
	size_t totalPixNum = imgDims[0] * imgDims[1] * imgDims[2];
	for (size_t i = 0; i < totalPixNum; ++i)
	{
		T updatedVal = getMax(inputPtr1[i], inputPtr2[i]);
		outputPtr[i] = updatedVal;
	}
}

template<class T>
inline void ImgProcessor::imgMax(const vector<T>* inputImgPtr1, const T inputImgPtr2[], T outputImgPtr[], const int imgDims[])
{
	size_t totalPixNum = imgDims[0] * imgDims[1] * imgDims[2];
	for (size_t i = 0; i < totalPixNum; ++i)
	{
		T updatedVal = getMax(inputImgPtr1->at(i), inputImgPtr2[i]);
		outputImgPtr[i] = updatedVal;
	}
}

template<class T>
inline void ImgProcessor::invert8bit(T input[], T output[])
{
	int length = sizeof(input) / sizeof(input[0]);
	for (int i = 0; i < length; ++i)
	{
		output[i] = 255 - input[i];
	}
}

template<class T1, class T2>
inline void ImgProcessor::flipY2D(T1 input[], T1 output[], T2 xLength, T2 yLength)
{
	for (int j = 0; j < yLength; ++j)
	{
		for (int i = 0; i < xLength; ++i)
		{
			output[xLength*j + i] = input[xLength*(yLength - j - 1) + i];
		}
	}
}

template<class T>
inline void ImgProcessor::maxIPSeries(const vector<vector<T>> inputSlicePtrs, T outputImgPtr[], const int imgDims[])
{
	for (int i = 0; i < imgDims[0] * imgDims[1]; ++i) outputImgPtr[i] = 0;
	for (typename vector<vector<T>>::iterator it = inputSlicePtrs.begin(); it != inputSlicePtrs.end(); ++it)
	{
		for (int i = 0; i < imgDims[0] * imgDims[1]; ++i) outputImgPtr[i] = ImgProcessor::imgMax(it, outputImgPtr, outputImgPtr, imgDims);
	}
}

template<class T>
inline void ImgProcessor::imgStackSlicer(const T inputImgPtr[], vector<vector<T>>& outputSlices, const int imgDims[])
{
	for (int k = 0; k < imgDims[2]; ++k)
	{
		vector<T> thisSlice;
		int outi = 0;
		for (int j = 0; j < imgDims[1]; ++j)
		{
			for (int i = 0; i < imgDims[0]; ++i)
			{
				thisSlice.push_back(inputImgPtr[(imgDims[0] * imgDims[1] * k) + (imgDims[0] * j) + i]);
			}
		}
		outputSlices.push_back(thisSlice);
		thisSlice.clear();
	}
}

template<class T>
inline void ImgProcessor::slice1Dvector2_2Darray(const vector<T>& inputSliceVec, T* outputSlice2Dptr[], const int imgDims[])
{
	for (int j = 0; j < imgDims[1]; ++j)
	{
		for (int i = 0; i < imgDims[0]; ++i)
			outputSlice2Dptr[j][i] = inputSliceVec.at((imgDims[0] * j) + i);
	}
}

template<class T>
inline void ImgProcessor::slice2Dto1D(T* inputImgPtr[], T outputImgPtr[], const int imgDims[])
{
	size_t outi = 0;
	for (size_t j = 0; j < imgDims[1]; ++j)
	{
		for (size_t i = 0; i < imgDims[0]; ++i)
		{
			outputImgPtr[outi] = inputImgPtr[j][i];
			++outi;
		}
	}
}

template<class T>
inline void ImgProcessor::imgDownSample2D(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], const int downSampFactor)
{
	long int outi = 0;
	int newXDim = imgDims[0] / downSampFactor;
	int newYDim = imgDims[1] / downSampFactor;
	cout << "output x Dim: " << newXDim << "  " << "output y Dim: " << newYDim << endl;
	for (int y = 0; y < newYDim; ++y)
	{
		for (int x = 0; x < newXDim; ++x)
		{
			outputImgPtr[outi] = inputImgPtr[(imgDims[0] * downSampFactor) * y + (x * downSampFactor)];
			++outi;
		}
	}
}

template<class T>
inline void ImgProcessor::imgDownSample(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], const int downSampFactors[])
{
	long int outi = 0;
	int newXDim = imgDims[0] / downSampFactors[0];
	int newYDim = imgDims[1] / downSampFactors[1];
	int newZDim = imgDims[2] / downSampFactors[2];
	cout << "output x Dim: " << newXDim << "  output y Dim: " << newYDim << "  output z Dim: " << newZDim << endl;
	for (int z = 0; z < newZDim; ++z)
	{
		for (int y = 0; y < newYDim; ++y)
		{
			for (int x = 0; x < newXDim; ++x)
			{
				outputImgPtr[outi] = inputImgPtr[(imgDims[0] * imgDims[1] * downSampFactors[2]) * z + (imgDims[0] * downSampFactors[1]) * y + x * downSampFactors[0]];
				++outi;
			}
		}
	}
}

template<class T>
inline void ImgProcessor::imgDownSample2DMax(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], const int downSampFactor)
{
	long int outi = 0;
	int newXDim = imgDims[0] / downSampFactor;
	int newYDim = imgDims[1] / downSampFactor;
	cout << "output x Dim: " << newXDim << "  " << "output y Dim: " << newYDim << endl;

	vector<int> dnSampBlock;
	T maxValue = 0;
	for (int y = 0; y < newYDim; ++y)
	{
		for (int x = 0; x < newXDim; ++x)
		{
			dnSampBlock.clear();
			maxValue = 0;
			for (int j = 0; j < downSampFactor; ++j)
			{
				for (int i = 0; i < downSampFactor; ++i)
				{
					T value = inputImgPtr[(imgDims[0] * downSampFactor * y + j) + (x * downSampFactor + i)];
					if (value > maxValue) maxValue = value;
				}
			}
			outputImgPtr[outi] = maxValue;
			++outi;
		}
	}
}

template<class T>
inline void ImgProcessor::imgDownSampleMax(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], const int downSampFactors[])
{
	long int outi = 0;
	int newXDim = imgDims[0] / downSampFactors[0];
	int newYDim = imgDims[1] / downSampFactors[1];
	int newZDim = imgDims[2] / downSampFactors[2];
	cout << "output x Dim: " << newXDim << "  output y Dim: " << newYDim << "  output z Dim: " << newZDim << endl;

	vector<int> dnSampBlock;
	T maxValue = 0;
	for (int z = 0; z < newZDim; ++z)
	{
		for (int y = 0; y < newYDim; ++y)
		{
			for (int x = 0; x < newXDim; ++x)
			{
				dnSampBlock.clear();
				maxValue = 0;
				for (int k = 0; k < downSampFactors[2]; ++k)
				{
					for (int j = 0; j < downSampFactors[1]; ++j)
					{
						for (int i = 0; i < downSampFactors[0]; ++i)
						{
							T value = inputImgPtr[(imgDims[0] * imgDims[1] * downSampFactors[2] * z + k) + (imgDims[0] * downSampFactors[1] * y + j) + (x * downSampFactors[0] + i)];
							if (value > maxValue) maxValue = value;
						}
					}
				}
				outputImgPtr[outi] = maxValue;
				++outi;
			}
		}
	}
}
// ====================================== END of [BASIC IMAGE OPERATION] ======================================


// ==================================== IMAGE STATISTICS ====================================
template<class T>
inline map<string, float> ImgProcessor::getBasicStats(const T inputImgPtr[], const int imgDims[])
{
	map<string, float> basicStats;
	size_t totalPixNum = imgDims[0] * imgDims[1] * imgDims[2];
	vector<T> img1DVec;

	float sum = 0;
	float mean = 0;
	float var = 0;
	float std = 0;
	float median = 0;

	for (size_t i = 0; i < totalPixNum; ++i)
	{
		img1DVec.push_back(inputImgPtr[i]);
		sum = sum + inputImgPtr[i];
	}
	basicStats.insert(pair<string, float>("sum", sum));

	mean = sum / float(totalPixNum);
	basicStats.insert(pair<string, float>("mean", mean));

	float varSum = 0;
	for (size_t i = 0; i < totalPixNum; ++i) varSum = varSum + (inputImgPtr[i] - mean) * (inputImgPtr[i] - mean);
	var = varSum / float(totalPixNum);
	std = sqrtf(var);
	basicStats.insert(pair<string, float>("var", var));
	basicStats.insert(pair<string, float>("std", std));

	sort(img1DVec.begin(), img1DVec.end());
	median = float(img1DVec[floor(totalPixNum / 2)]);
	basicStats.insert(pair<string, float>("median", median));

	return basicStats;
}

template<class T>
inline map<string, float> ImgProcessor::getBasicStats_no0(const T inputImgPtr[], const int imgDims[])
{
	map<string, float> basicStats;
	vector<T> img1DVec;
	float validPixCount = 0;

	float sum = 0;
	float mean = 0;
	float var = 0;
	float std = 0;
	float median = 0;

	for (size_t i = 0; i < imgDims[0] * imgDims[1] * imgDims[2]; ++i)
	{
		if (inputImgPtr[i] == 0) continue;

		++validPixCount;
		img1DVec.push_back(inputImgPtr[i]);
		sum = sum + inputImgPtr[i];
	}
	if (img1DVec.size() == 0)
	{
		cerr << "This is a blank image. No stats will be computed." << endl;
		basicStats.clear();
		return basicStats;
	}
	basicStats.insert(pair<string, float>("sum", sum));

	mean = sum / float(validPixCount);
	basicStats.insert(pair<string, float>("mean", mean));

	float varSum = 0;
	for (size_t i = 0; i < validPixCount; ++i) varSum = varSum + (img1DVec.at(i) - mean) * (img1DVec.at(i) - mean);
	var = varSum / float(validPixCount);
	std = sqrtf(var);
	basicStats.insert(pair<string, float>("var", var));
	basicStats.insert(pair<string, float>("std", std));

	sort(img1DVec.begin(), img1DVec.end());
	median = float(img1DVec[floor(validPixCount / 2)]);
	basicStats.insert(pair<string, float>("median", median));

	return basicStats;
}

template<class T>
inline map<int, size_t> ImgProcessor::histQuickList(const T inputImgPtr[], const int imgDims[])
{
	size_t totalPixNum = imgDims[0] * imgDims[1] * imgDims[2];
	map<int, size_t> histMap;
	for (size_t i = 0; i < totalPixNum; ++i)
	{
		int value = int(inputImgPtr[i]);
		++histMap.insert(pair<int, size_t>(value, 0)).first->second;
	}

	return histMap;
}
// ================================== END of [IMAGE STATISTICS] ==================================


// ================================== IMAGE PROCESSING/FILTERING ==================================
template<class T>
inline void ImgProcessor::simpleThresh(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], const int threshold)
{
	size_t totalPixNum = imgDims[0] * imgDims[1] * imgDims[2];
	for (size_t i = 0; i < totalPixNum; ++i)
	{
		if (inputImgPtr[i] < threshold)
		{
			outputImgPtr[i] = 0;
			continue;
		}
		else outputImgPtr[i] = inputImgPtr[i];
	}
}

template<class T>
inline void ImgProcessor::simpleThresh_reverse(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], const int threshold)
{
	size_t totalPixNum = imgDims[0] * imgDims[1] * imgDims[2];
	for (size_t i = 0; i < totalPixNum; ++i)
	{
		if (inputImgPtr[i] > threshold)
		{
			outputImgPtr[i] = 0;
			continue;
		}
		else outputImgPtr[i] = inputImgPtr[i];
	}
}

template<class T>
inline void ImgProcessor::imgMasking(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], const int threshold)
{
	size_t totalPixNum = imgDims[0] * imgDims[1] * imgDims[2];
	for (size_t i = 0; i < totalPixNum; ++i)
	{
		if (inputImgPtr[i] > threshold)
		{
			outputImgPtr[i] = 255;
			continue;
		}
		else outputImgPtr[i] = inputImgPtr[i];
	}
}

template<class T>
inline void ImgProcessor::stepped_gammaCorrection(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], int cutoffIntensity)
{
	size_t totalPixNum = imgDims[0] * imgDims[1] * imgDims[2];
	for (size_t i = 0; i < totalPixNum; ++i)
	{
		if (inputImgPtr[i] == 0)
		{
			outputImgPtr[i] = 0;
			continue;
		}

		int transformedValue = int(inputImgPtr[i]);
		if (transformedValue < cutoffIntensity) outputImgPtr[i] = 0;
		else if (transformedValue * (transformedValue - cutoffIntensity) >= 255) outputImgPtr[i] = 255;
		else outputImgPtr[i] = transformedValue * (transformedValue - cutoffIntensity);
	}
}

template<class T>
inline void ImgProcessor::gammaCorrect_old(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], int starting_intensity)
{
	size_t totalPixNum = imgDims[0] * imgDims[1] * imgDims[2];
	for (size_t i = 0; i < totalPixNum; ++i)
	{
		if (inputImgPtr[i] == 0)
		{
			outputImgPtr[i] = 0;
			continue;
		}

		int transformedValue = int(inputImgPtr[i]);
		if (transformedValue < starting_intensity) outputImgPtr[i] = 0;
		else if (transformedValue * (transformedValue - starting_intensity + 2) >= 255) outputImgPtr[i] = 255;
		else outputImgPtr[i] = transformedValue * (transformedValue - starting_intensity + 2);
	}
}

template<class T>
inline void ImgProcessor::reversed_gammaCorrect_eqSeriesFactor(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], int starting_intensity)
{
	size_t totalPixNum = imgDims[0] * imgDims[1] * imgDims[2];
	for (size_t i = 0; i < totalPixNum; ++i)
	{
		if (inputImgPtr[i] == 0)
		{
			outputImgPtr[i] = 0;
			continue;
		}

		int transformedValue = int(inputImgPtr[i]);
		if (transformedValue > starting_intensity) outputImgPtr[i] = transformedValue;
		else if (transformedValue / (starting_intensity - transformedValue + 1) < 1) outputImgPtr[i] = 0;
		else outputImgPtr[i] = transformedValue / (starting_intensity - transformedValue + 1);
	}
}

template<class T>
inline void ImgProcessor::histEqual_unit8(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], bool noZero)
{
	map<int, size_t> histMap = ImgProcessor::histQuickList(inputImgPtr, imgDims);
	if (noZero) histMap.erase(histMap.find(0)); // exclude 0 from the histogram profile
	
	map<int, double> histCumul; // generate CDF
	double totalNum = 0;
	int maxIntensity = 0;
	for (map<int, size_t>::iterator it = histMap.begin(); it != histMap.end(); ++it)
	{
		totalNum = totalNum + it->second;
		histCumul.insert(pair<int, double>(it->first, double(totalNum)));
		maxIntensity = it->first;
	}
	
	map<int, int> binMap; // mapping intensity
	for (map<int, double>::iterator it = histCumul.begin(); it != histCumul.end(); ++it)
		binMap.insert(pair<int, int>(it->first, int(round((it->second / histCumul.at(maxIntensity)) * 255))));
	
	for (size_t i = 0; i < imgDims[0] * imgDims[1]; ++i)
	{
		if (inputImgPtr[i] == 0) outputImgPtr[i] = 0;
		else outputImgPtr[i] = T(binMap.at(int(inputImgPtr[i])));
	}
}

template<class T>
inline void ImgProcessor::simpleAdaThre(const T inputImgPtr[], T outputImgPtr[], const int imgDims[], const int stepSize, const int sampRate)
{
	int sampCount = 0, sampSum = 0; 
	float residue = 0;
	for (int k = 0; k < imgDims[2]; ++k)
	{
		for (int j = 0; j < imgDims[1]; ++j)
		{
			for (int i = 0; i < imgDims[0]; ++i)
			{
				sampCount = 0;
				sampSum = 0;
				for (int rate = 1; rate <= sampRate; ++rate)
				{
					if (k - stepSize * rate >= 0)		  { ++sampCount; sampSum += int(inputImgPtr[imgDims[0] * imgDims[1] * (k - stepSize * rate) + imgDims[0] * j + i]); }
					if (k + stepSize * rate < imgDims[2]) { ++sampCount; sampSum += int(inputImgPtr[imgDims[0] * imgDims[1] * (k + stepSize * rate) + imgDims[0] * j + i]); }
					if (j - stepSize * rate >= 0)		  { ++sampCount; sampSum += int(inputImgPtr[imgDims[0] * imgDims[1] * k + imgDims[0] * (j - stepSize * rate) + i]); }
					if (j + stepSize * rate < imgDims[1]) { ++sampCount; sampSum += int(inputImgPtr[imgDims[0] * imgDims[1] * k + imgDims[0] * (j + stepSize * rate) + i]); }
					if (i - stepSize * rate >= 0)		  { ++sampCount; sampSum += int(inputImgPtr[imgDims[0] * imgDims[1] * k + imgDims[0] * j + (i - stepSize * rate)]); }
					if (i + stepSize * rate < imgDims[0]) { ++sampCount; sampSum += int(inputImgPtr[imgDims[0] * imgDims[1] * k + imgDims[0] * j + (i + stepSize * rate)]); }
				}

				residue = float(inputImgPtr[imgDims[0] * imgDims[1] * k + imgDims[0] * j + i]) - float(sampSum) / float(sampCount);
				outputImgPtr[imgDims[0] * imgDims[1] * k + imgDims[0] * j + i] = (residue > 0) ? residue : 0;
			}
		}
	}
}
// ================================ END of [IMAGE PROCESSING/FILTERING] ================================

#endif