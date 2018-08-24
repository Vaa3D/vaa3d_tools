//------------------------------------------------------------------------------
// Copyright (c) 2018 Hsienchi Kuo (Allen Institute, Hanchuan Peng's team)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
* This library intends to fascilitate basic image operations

*/

#ifndef IMGPROCESSOR_H
#define IMGPROCESSOR_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <algorithm>

using namespace std;

#define getMax(a, b) ((a) >= (b)? (a):(b))

enum MIPOrientation {Mxy, Myz, Mxz};
enum mipOrientation {mxy, myz, mxz};

class ImgProcessor 
{

public:
	/******** Constructors *********/
	ImgProcessor() {};
	ImgProcessor(unsigned char* inputImg1DPtr);
	ImgProcessor(unsigned char** inputImg2DPtr);
	ImgProcessor(unsigned char*** inputImg3DPtr);
	/*******************************/

	/***************** Data Members *****************/
	unsigned char*   inputImg1DPtr;
	unsigned char**  inputImg2DPtr;
	unsigned char*** inputImg3DPtr;
	/************************************************/

	// These 2 enums decide in which direction to make projection.
	MIPOrientation MIPDirection;
	mipOrientation mipDirection;


	/***************** Basic Image Operations *****************/
	template<class T>
	static inline void simpleThresh(T inputImgPtr[], T outputImgPtr[], int imgDims[], int threshold);

	template<class T>
	static inline void imgMasking(T inputImgPtr[], T outputImgPtr[], int imgDims[], int threshold);
	
	template<class T>
	static inline void cropImg2D(T InputImagePtr[], T OutputImagePtr[], int xlb, int xhb, int ylb, int yhb, int imgDims[]);

	template<class T>
	static inline void invert8bit(T input1D[], T output1D[]);

	template<class T1, class T2>
	static inline void flipY2D(T1 input1D[], T1 output1D[], T2 xLength, T2 yLength);

	template<class T>
	static inline void imageMax(T inputPtr1[], T inputPtr2[], T outputPtr[], int imgDims[]); // Between 2 input images, pixel-wisely pick the one with greater value. 

	template<class T>
	static inline void imgDownSample2D(T inputImgPtr[], T outputImgPtr[], int imgDims[], int downSampFactor);

	template<class T>
	static inline void imgDownSample2DMax(T inputImgPtr[], T outputImgPtr[], int imgDims[], int downSampFactor);

	template<class T>
	static vector<vector<T>> imgStackSlicer(T inputImgPtr[], int imgDims[]);

	void maxIPStack(unsigned char inputVOIPtr[], unsigned char OutputImage2DPtr[],
		int MIPxDim, int MIPyDim, int MIPzDim); // make MIP out of an input 1D image data array	
	/**********************************************************/


	/***************** Image Statistics *****************/
	template<class T>
	static inline map<string, float> getBasicStats(T inputImgPtr[], int imgDims[]);

	template<class T>
	static inline map<string, float> getBasicStats_no0(T inputImgPtr[], int imgDims[]);

	template<class T>
	static inline map<int, size_t> histQuickList(T inputImgPtr[], int imgDims[]);
	/****************************************************/


	/***************** Image Processing/Filtering *****************/
	template<class T>
	static inline void gammaCorrect_eqSeriesFactor(T inputImgPtr[], T outputImgPtr[], int imgDims[], int starting_intensity = 0);

	template<class T>
	static inline void reversed_gammaCorrect_eqSeriesFactor(T inputImgPtr[], T outputImgPtr[], int imgDims[], int starting_intensity = 255);
	/**************************************************************/


	/********* Morphological Operations *********/
	static void erode2D(unsigned char inputPtr[], unsigned char outputPtr[]);
	/********************************************/


	/***************** Other utilities *****************/
	static void shapeMask2D(int imgDims[2], unsigned char outputMask1D[], int coords[2], int regionDims[2], string shape = "square");
	/***************************************************/
};

// ========================================= BASIC IMAGE OPERATION =========================================
template<class T>
inline void ImgProcessor::simpleThresh(T inputImgPtr[], T outputImgPtr[], int imgDims[], int threshold)
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
inline void ImgProcessor::imgMasking(T inputImgPtr[], T outputImgPtr[], int imgDims[], int threshold)
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
inline void ImgProcessor::cropImg2D(T InputImagePtr[], T OutputImagePtr[], int xlb, int xhb, int ylb, int yhb, int imgDims[])
{
	long int OutputArrayi = 0;
	for (int yi = ylb; yi <= yhb; ++yi)
	{
		for (int xi = xlb; xi <= xhb; ++xi)
		{
			OutputImagePtr[OutputArrayi] = InputImagePtr[imgDims[0] * (yi - 1) + (xi - 1)];
			++OutputArrayi;
		}
	}
}

template<class T>
inline void ImgProcessor::imageMax(T inputPtr1[], T inputPtr2[], T outputPtr[], int imgDims[])
{
	size_t totalPixNum = imgDims[0] * imgDims[1] * imgDims[2];
	for (size_t i = 0; i < totalPixNum; ++i)
	{
		T updatedVal = getMax(inputPtr1[i], inputPtr2[i]);
		outputPtr[i] = updatedVal;
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
vector<vector<T>> ImgProcessor::imgStackSlicer(T inputImgPtr[], int imgDims[])
{
	vector<vector<T>> outputSlices;
	for (int k = 0; k < imgDims[2]; ++k)
	{
		vector<T> thisSlice;
		long int outi = 0;
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

	return outputSlices;
}

template<class T>
inline void ImgProcessor::imgDownSample2D(T inputImgPtr[], T outputImgPtr[], int imgDims[], int downSampFactor)
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
inline void ImgProcessor::imgDownSample2DMax(T inputImgPtr[], T outputImgPtr[], int imgDims[], int downSampFactor)
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
// ====================================== END of [BASIC IMAGE OPERATION] ======================================


// ==================================== IMAGE STATISTICS ====================================
template<class T>
inline map<string, float> ImgProcessor::getBasicStats(T inputImgPtr[], int imgDims[])
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
inline map<string, float> ImgProcessor::getBasicStats_no0(T inputImgPtr[], int imgDims[])
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
	basicStats.insert(pair<string, float>("sum", sum));

	mean = sum / float(validPixCount);
	basicStats.insert(pair<string, float>("mean", mean));

	float varSum = 0;
	for (size_t i = 0; i < validPixCount; ++i) varSum = varSum + (inputImgPtr[i] - mean) * (inputImgPtr[i] - mean);
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
inline map<int, size_t> ImgProcessor::histQuickList(T inputImgPtr[], int imgDims[])
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
inline void ImgProcessor::gammaCorrect_eqSeriesFactor(T inputImgPtr[], T outputImgPtr[], int imgDims[], int starting_intensity)
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
inline void ImgProcessor::reversed_gammaCorrect_eqSeriesFactor(T inputImgPtr[], T outputImgPtr[], int imgDims[], int starting_intensity)
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
// ================================ END of [IMAGE PROCESSING/FILTERING] ================================

#endif