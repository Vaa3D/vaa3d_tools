#include <stdio.h>
#include <string.h>

#include "waveletTransform.h"

// Compute the maximum feasible scale for a given 3D image size
// for as scale i, the minimum size of the image is 5 +4*(2^(i-1)-1) along each direction
int WaveletTransform::computeMaximumScale(int width, int height, int depth)
{
	//compute the smallest size along the 3 dimensions
	int minSize = width;
	if (height < minSize)
		minSize = height;
	if (depth < minSize)
		minSize = depth;
	if (minSize < 5) //b3 kernel size
		return 0;
	int maxScale = 1;
	while (5+(pow(2, maxScale+1-1)-1)*4 < minSize) //increase the scale until the bound exceeds the smallest size
			maxScale++;
	return maxScale;
}

// Compute the maximum feasible scale for a given 2D image size
// for as scale i, the minimum size of the image is 5 +4*(2^(i-1)-1) along each direction
int WaveletTransform::computeMaximumScale2D(int width, int height)
{
	//compute the smallest size along the 2 dimensions
	int minSize = width;
	if (height < minSize)
		minSize = height;
	if (minSize < 5) //b3 kernel size
		return 0;
	int maxScale = 1;
	while (5+(pow(2, maxScale+1-1))*4 < minSize) //increase the scale until the bound exceeds the smallest size
			maxScale++;
	return maxScale;
}


//Check that the 3D image dimensions complies with the number of chosen scales
void WaveletTransform::checkImageDimensions(int width, int height, int depth, int numScales) throw (WaveletConfigException)
{
 	int minSize = 5+(pow(2, numScales-1))*4;//compute the minimum size for numScales scales
 	if (width < minSize || height < minSize || depth < minSize)//image size is too small, return an exception
  	{
  		char* buffer = new char[150];
  		sprintf(buffer, "Number of scales too large for the size of the image. These settings require: width>%d, height >%d and depth >%d", minSize-1, minSize-1, minSize-1);
  		throw WaveletConfigException(buffer);
  	}
}
//Check that the 2D image dimensions complies with the wavelet scale
void WaveletTransform::checkImageDimensions2D(int width, int height, int numScales) throw (WaveletConfigException)
{
	int minSize = 5+(pow(2, numScales-1)-1)*4;//compute the minimum size for numScales scales
 	if (width < minSize || height < minSize)//image size is too small, return an exception
  	{
  		char* buffer = new char[150];
  		sprintf(buffer, "Number of scales too large for the size of the image. These settings require: width>%d, height >%d", minSize-1, minSize-1);
  		throw WaveletConfigException(buffer);
  	}
}

//Compute the wavelet coefficients from wavelet scales
double** WaveletTransform::b3WaveletCoefficients(double** scaleCoefficients, double* originalImage, int numScales, int numVoxels)
{
	//numScales wavelet images to store, + one image for the low pass residual
	double** waveletCoefficients = new double*[numScales+1];

	//compute wavelet coefficients as the difference between scale coefficients of subsequent scales
	double* iterPrev = originalImage;//the finest scale coefficient is the difference between the original image and the first scale.
	double* iterCurrent;
 	double* currentImgEnd;
 	
 	int j = 0;
	while (j<numScales)
 	{
 		iterCurrent =  scaleCoefficients[j];
 		currentImgEnd = iterCurrent + numVoxels;
 		waveletCoefficients[j] = new double[numVoxels];//wavelet coefficients at a given scale, the wavelet image is the same size as the original image
		double* iterResult = waveletCoefficients[j];
		while(iterCurrent<currentImgEnd)//iterate over the coefficients of two given scales
 		{
 			(*iterResult) = (*iterPrev)-(*iterCurrent);//compute the difference between the coefficients
 			iterCurrent++;
 			iterPrev++;
 			iterResult++;
 		}
 		iterPrev = iterCurrent;
 		j++;
 	}	
	//residual low pass image is the last wavelet Scale
	waveletCoefficients[numScales] = new double[numVoxels];
	memcpy(waveletCoefficients[numScales], scaleCoefficients[numScales-1], numVoxels*sizeof(double));
	return waveletCoefficients;
}

//Compute the wavelet coefficients and override wavelet scale coefficients
void WaveletTransform::b3WaveletCoefficientsInplace(double** coefficients, double* originalImage, double* lowPass, int numScales, int numVoxels)
{
	//residual low pass image is the last wavelet Scale
	memcpy(lowPass, coefficients[numScales-1], numVoxels*sizeof(double));
	
	//then subtract wavelet scales to get wavelet coefficients 
	double* prevImg;
 	double* prevImgEnd;
 	double* iterPrev;
 	double* currentImg;
 	double* currentImgEnd;
 	double* iterCurrent;
 	
 	for (int j = numScales-1; j >0; j--)//iterate from the coarsest scale to the finest scale
 	{
 		currentImg = coefficients[j];
 		currentImgEnd = currentImg+numVoxels;
 		iterCurrent = currentImg;
 		
 		prevImg = coefficients[j-1];
 		prevImgEnd = prevImg+numVoxels;
 		iterPrev = prevImg;
 		
 		while(iterCurrent<currentImgEnd)//iterate over the coefficients of two given scales
 		{
 			(*iterCurrent) = (*iterPrev)-(*iterCurrent);//replace the wavelet scale coefficient by the difference between two subsequent scales
 			iterCurrent++;
 			iterPrev++;
 		}
 	}
 	//the finest scale coefficient is the difference between the original image and the first scale.
 	currentImg = coefficients[0];
 	currentImgEnd = currentImg+numVoxels;
 	iterCurrent = currentImg;
 		
 	prevImg = originalImage;
 	prevImgEnd = prevImg+numVoxels;
 	iterPrev = prevImg;
 	
 	while(iterCurrent<currentImgEnd)
 	{
 		(*iterCurrent) = (*iterPrev)-(*iterCurrent);
 		iterCurrent++;
 		iterPrev++;
 	}
}

//Reconstruct an image from the wavelet coefficients and a low pass residual image
void WaveletTransform::b3WaveletReconstruction(double** inputCoefficients, double* lowPassResidual, double* output, int numScales, int numVoxels)
{
	double** iter = new double*[numScales];
	for (int i =0; i < numScales; i++)
		iter[i] = inputCoefficients[i];
	double* iterLowPass = lowPassResidual;
	double* iterOut = output;
	double* endIter = output+numVoxels;
	double tmp;
	while(iterOut<endIter)//loop over the voxels
	{
		tmp = (*iterLowPass);//initialize the voxel as the lowpass value
		iterLowPass++;
		for  (int i =0; i < numScales; i++)//loop over the scales
		{
			tmp += (*(iter[i]));//add the wavelet coefficient
			iter[i]++;
		}
		*iterOut = tmp;
		iterOut++;
	}
}

//filter a 2D image with the B3 spline wavelet scale kernel in the x direction when using the a trous algorithm, and swap dimensions
void WaveletTransform::filterAndSwap2D(double* arrayIn, double* arrayOut, int width, int height, int stepS)
{
	//B3 spline wavelet configuration
	//the convolution kernel is {1/16, 1/4, 3/8, 1/4, 1/16}
	//with some zeros values inserted between the coefficients, depending on the scale
	double w2 =  ((double)1)/16;
	double w1 = ((double)1)/4;
	double w0 = ((double)3)/8;
	
	double* w0idx;
 	double* w1idx1;
 	double* w2idx1;
 	double* w1idx2;
 	double* w2idx2;	
 	double* arrayOutiter;
 	
 	int cntX;
 	w0idx = arrayIn;
 	
 	for (int y=0; y<height; y++)//loop over the second dimension
 	{
 		//manage the left border with mirror symmetry
 		arrayOutiter = arrayOut + y;//output pointer initialization, we swap dimensions at this point
 		//w0idx = arrayIn + y*width;
 		w1idx1 = w0idx + stepS-1;
 		w2idx1 = w1idx1 + stepS;
 		w1idx2 = w0idx + stepS;
 		w2idx2 = w1idx2 + stepS;
 						
 		cntX = 0;
 		while(cntX < stepS)
 		{
 			*arrayOutiter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 			w1idx1--;
 			w2idx1--;
 			w1idx2++;
 			w2idx2++;
 			w0idx++;
 			arrayOutiter+=height;
 			cntX++;
 		}
 		w1idx1++;
 		while(cntX < 2*stepS)
 		{
 			*arrayOutiter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);							
 			w1idx1++;
 			w2idx1--;
 			w1idx2++;
 			w2idx2++;
 			w0idx++;
 			arrayOutiter+=height;
 			cntX++;
 		}
 		w2idx1++;
 		//filter the center area of the image (no border issue)
 		while(cntX < width - 2*stepS)
 		{	
 			*arrayOutiter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);	
 			w1idx1++;
 			w2idx1++;
 			w1idx2++;
 			w2idx2++;
 			w0idx++;
 			arrayOutiter+=height;
 			cntX++;
 		}
 		w2idx2--;
 		//manage the right border with mirror symmetry
 		while (cntX < width - stepS)
 		{
 			*arrayOutiter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 			w1idx1++;
 			w2idx1++;
 			w1idx2++;
 			w2idx2--;
 			w0idx++;
 			arrayOutiter+=height;
 			cntX++;
 		}
 		w1idx2--;
 		while (cntX < width)
 		{
 			*arrayOutiter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
 			w1idx1++;
 			w2idx1++;
 			w1idx2--;
 			w2idx2--;
 			w0idx++;
 			arrayOutiter+=height;
 			cntX++;
 		}
 	}
}

//Compute the wavelet scale images for a 2D image
double**  WaveletTransform::b3WaveletScales2D(double* dataIn, int width, int height, int numScales) throw (WaveletConfigException)
{
	if (numScales < 1)//at least on scale is required
 	{
			throw WaveletConfigException("Invalid number of wavelet scales. Number of scales should be an integer >=1");
 	}
 	//check that image dimensions complies with the number of chosen scales
 	try{checkImageDimensions2D(width, height, numScales);}
 	catch(WaveletConfigException e)
 	{
 		throw(e);
 	}
	
	int s;//scale
	int stepS;//step between non zero coefficients of the convolution kernel, depends on the scale
	int wh = width*height;
	double** resArray = new double*[numScales];//store wavelet scales in a new 2d double array
 	double* prevArray = dataIn; //array to filter, original data for the first scale
 	double* currentArray = new double [wh]; //filtered array
 
 	for (s = 1; s <= numScales; s++)//for each scale
 	{
 		 stepS = pow(2, s-1);//compute the step between non zero coefficients of the convolution kernel = 2^(scale-1)
 		//convolve along the x direction and swap dimensions
 		filterAndSwap2D(prevArray, currentArray, width, height, stepS);
 		//swap current and previous array pointers
 		if (s==1)
 		{
 			prevArray = currentArray;//the filtered array becomes the array to filter
 			currentArray = new double[wh];//allocate memory for the next dimension filtering (preserve original data)
 		}
 		else
 		{
 			double* tmp = currentArray;
 			currentArray = prevArray;//the filtered array becomes the array to filter
 			prevArray = tmp;//the filtered array becomes the array to filter
 		}		
 		//convolve along the y direction and swap dimensions
		filterAndSwap2D(prevArray, currentArray, height, width, stepS);//swap size of dimensions
 		//swap current and previous array pointers
 		double* tmp = currentArray;
 		currentArray = prevArray;
 		prevArray = tmp;
 				
 		resArray[s-1] = new double[wh]; //allocate memory to store the filtered array
 		memcpy(resArray[s-1], prevArray, wh*sizeof(double)); //copy the filtered array to the result table
 	}
 	delete[] currentArray;
 	delete[] prevArray;
	return resArray;
}

// Compute the scale images for a 3D image
double**  WaveletTransform::b3WaveletScales(double* dataIn, int width, int height, int depth, int numScales) throw (WaveletConfigException)
{
	if (numScales < 1)//at least on scale is required
 	{
			throw WaveletConfigException("Invalid number of wavelet scales. Number of scales should be an integer >=1");
 	}
 	//check that image dimensions complies with the number of chosen scales
 	try{checkImageDimensions(width, height, depth, numScales);}
 	catch(WaveletConfigException e)
 	{
 		throw(e);
 	}
		
	int s;//scale
	int stepS;//step between non zero coefficients of the convolution kernel, depends on the scale
	int whd = width*height*depth;
	
	double** resArray = new double*[numScales];//store wavelet scales in a new 2d double array
 	double* prevArray = dataIn; //array to filter, original data for the first scale
 	double* currentArray = new double [whd];//filtered array
 
 	for (s = 1; s <= numScales; s++)//for each scale
 	{
 		 stepS = pow(2, s-1);//compute the step between non zero coefficients of the convolution kernel = 2^(scale-1)
 		//convolve along the x direction and swap dimensions
 		filterAndSwap(prevArray, currentArray, width, height, depth, stepS);
 		//swap current and previous array pointers
 		if (s==1)
 		{
 			prevArray = currentArray; //the filtered array becomes the array to filter
 			currentArray = new double[whd]; //allocate memory for the next dimension filtering (preserve original data)
 		}
 		else
 		{
 			double* tmp = currentArray;
 			currentArray = prevArray; //the filtered array becomes the array to filter
 			prevArray = tmp; //the filtered array becomes the array to filter
 		}		
 		//convolve along the y direction and swap dimensions
		filterAndSwap(prevArray, currentArray, height, depth, width, stepS);//swap size of dimensions
 		//swap current and previous array pointers
 		double* tmp = currentArray;
 		currentArray = prevArray;
 		prevArray = tmp;
 		
 		//convolve along the z direction and swap dimensions
		filterAndSwap(prevArray, currentArray, depth, width, height, stepS);//swap size of dimensions
 		//swap current and previous array pointers
 		tmp = currentArray;
 		currentArray = prevArray;
 		prevArray = tmp;
 				
 		resArray[s-1] = new double[whd]; //allocate memory to store the filtered array
 		memcpy(resArray[s-1], prevArray, whd*sizeof(double)); //copy the filtered array to the result table
 	}
 	delete[] currentArray;
 	delete[] prevArray;
	return resArray;
}

//filter a 3D image with the B3 spline wavelet scale kernel in the x direction when using the a trous algorithm, and swap dimensions
void WaveletTransform::filterAndSwap(double* arrayIn, double* arrayOut, int width, int height, int depth, int stepS)
{
	//B3 spline wavelet configuration
	//the convolution kernel is {1/16, 1/4, 3/8, 1/4, 1/16}
	//with some zeros values inserted between the coefficients, depending on the scale
	double w2 =  ((double)1)/16;
	double w1 = ((double)1)/4;
	double w0 = ((double)3)/8;
	
	int hd = height*depth;
	
	double* w0idx;
 	double* w1idx1;
 	double* w2idx1;
 	double* w1idx2;
 	double* w2idx2;	
 	double* arrayOutIter;
 	
// 	int cntX;
 	w0idx = arrayIn;
 	
	for (int z=0; z<depth; z++)//loop over the third dimension
 	{
 		for (int y=0; y<height; y++)//loop over the second dimension
 		{
 			//manage the left border with mirror symmetry
			arrayOutIter = arrayOut + y + z*height; //output pointer initialization, we swap dimensions at this point
			//w0idx = arrayIn + (y*width + z*wh);
			w1idx1 = w0idx + stepS-1;
			w2idx1 = w1idx1 + stepS;
			w1idx2 = w0idx + stepS;
			w2idx2 = w1idx2 + stepS;
			
		//	cntX = 0;
			double* end0 = w0idx + stepS;
			while (w0idx<end0)
	//		while(cntX < stepS)
			{
				*arrayOutIter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
				w1idx1--;
				w2idx1--;
				w1idx2++;
				w2idx2++;
				w0idx++;
				arrayOutIter+=hd;
				//cntX++;
			}
			w1idx1++;
			end0 = w0idx + stepS;
			while (w0idx<end0)
			//while(cntX < 2*stepS)
			{
				*arrayOutIter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);							
				w1idx1++;
				w2idx1--;
				w1idx2++;
				w2idx2++;
				w0idx++;
				arrayOutIter+=hd;
				//cntX++;
			}
			w2idx1++;
			//filter the center area of the image (no border issue)
			end0 = w0idx + width - 4*stepS;
			while (w0idx<end0)
			//while(cntX < width - 2*stepS)
			{	
				*arrayOutIter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);	
				w1idx1++;
				w2idx1++;
				w1idx2++;
				w2idx2++;
				w0idx++;
				arrayOutIter+=hd;
				//cntX++;
			}
			w2idx2--;
			//manage the right border with mirror symmetry
			end0 = w0idx + stepS;
			while (w0idx<end0)
			//while (cntX < width - stepS)
			{
				*arrayOutIter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
				w1idx1++;
				w2idx1++;
				w1idx2++;
				w2idx2--;
				w0idx++;
				arrayOutIter+=hd;
				//cntX++;
			}
			w1idx2--;
			end0 = w0idx + stepS;
			while (w0idx<end0)
//			while (cntX < width)
			{
				*arrayOutIter = w2*((*w2idx1) + (*w2idx2)) + w1*((*w1idx1) + (*w1idx2)) + w0*(*w0idx);						
				w1idx1++;
				w2idx1++;
				w1idx2--;
				w2idx2--;
				w0idx++;
				arrayOutIter+=hd;
				//cntX++;
			}
		}
	}
}
