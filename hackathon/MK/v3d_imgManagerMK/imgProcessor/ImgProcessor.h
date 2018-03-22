#ifndef IMGPROCESSOR_H
#define IMGPROCESSOR_H

#include <iostream>

using namespace std;

#define getMax(a, b) ((a) >= (b)? (a):(b))

enum MIPOrientation {Mxy, Myz, Mxz};
enum mipOrientation {mxy, myz, mxz};

class ImgProcessor 
{

public:
	/******** Constructors *********/
	ImgProcessor() : MIPDirection(Mxy), mipDirection(mxy) {};
	ImgProcessor(MIPOrientation);
	/*******************************/

	// These 2 enums decide in which direction to make projection.
	MIPOrientation MIPDirection;
	mipOrientation mipDirection;

	/********* Basic Image Operations *********/
	template<class T1, class T2>
	static inline void cropImg2D(T1 InputImagePtr[], T1 OutputImagePtr[], T2 xlb, T2 xhb, T2 ylb, T2 yhb, T2 imgX, T2 imgY);

	template <class T1, class T2>
	static inline void imageMax(T1 inputPtr[], T1 outputPtr[], T2 pixelNum); // Between 2 input images, pixel-wisely pick the greater pixel. 

	template<class T>
	static inline void invert8bit(T input1D[], T output1D[]);

	template<class T1, class T2>
	static inline void flipY2D(T1 input1D[], T1 output1D[], T2 xLength, T2 yLength);

	template<class T>
	static inline void imgDownSample2D(T inputImgPtr[], T outputImgPtr[], int imgDims[], int downSampFactor);
	/******************************************/

	/********* Other utilities *********/
	void maxIPStack(unsigned char inputVOIPtr[], unsigned char OutputImage2DPtr[],
		long int MIPxDim, long int MIPyDim, long int MIPzDim); // make MIP out of an input 1D image data array	

	static void shapeMask2D(int imgDims[2], unsigned char outputMask1D[], int coords[2], int regionDims[2], string shape = "square");
};

template<class T1, class T2>
inline void ImgProcessor::cropImg2D(T1 InputImagePtr[], T1 OutputImagePtr[], T2 xlb, T2 xhb, T2 ylb, T2 yhb, T2 imgX, T2 imgY)
{
	int OutputArrayi = 0;
	for (T2 yi = ylb; yi <= yhb; ++yi)
	{
		for (T2 xi = xlb; xi <= xhb; ++xi)
		{
			OutputImagePtr[OutputArrayi] = InputImagePtr[imgX*(yi - 1) + (xi - 1)];
			++OutputArrayi;
		}
	}
}

template<class T1, class T2>
inline void ImgProcessor::imageMax(T1 inputPtr[], T1 outputPtr[], T2 pixelNum)
{
	for (size_t i = 0; i < pixelNum; ++i)
	{
		T1 updatedVal = getMax(inputPtr[i], outputPtr[i]);
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

#endif