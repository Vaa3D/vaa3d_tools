#ifndef __WAVELETTRANSFORM_H__
#define __WAVELETTRANSFORM_H__

/*! \mainpage A trous wavelet transform utility
 * 
 * Useful functions for computing undecimated B3 spline wavelet transform on 2D and 3D images.
 *
 * \section intro_sec Introduction
 *
 * The algorithm implemented is the a trous algorithm for the B3 spline wavelet. See:
 * <UL>
 * <li> S. Mallat, "A WAVELET TOUR OF SIGNAL PROCESSING", Academic Press, 1988)
 * <li> Image and Data Analysis: The Multiscale Approach", J.-L. Starck, F. Murtagh and A. Bijaoui, Cambridge University Press, 1998
 * </UL>
 * The provided functions are primarily designed for 2D and 3D image denoising and target detection. For a description of the wavelet transform application to bio-medical image denoising and particle detection please read:<BR>
 * <UL>
 * <li> I. Smal, M. Loog, W. Niessen, E. Meijering. "Quantitative Comparison of Spot Detection Methods in Fluorescence Microscopy", IEEE Transactions on Medical Imaging, 29(2):282-301, February 2010 
 * <li> Fabrice de Chaumont, Nicolas Chenouard, Aurelie Mouret, Pierre Marie Lledo, Jean-Christophe Olivo-Marin: A multi-threaded program architecture for an asynchronous and highly responsive GUI for automatic neuronal survival quantification, 1007-1010, ISBI 2008.
 * <li> Olivo-Marin, J.-C. (2002) Extraction of spots in biological images using multiscale products, Pattern Recognition, 35, 9, pp. 1989-1996
 * </UL>
 * \section Example
 * In the wavelet transform library all the 2D and 3D images are represented by 1d double arrays with the xyz convention: the x index is varying first, then the y index, and the z index for 3D images.<BR>
 * Here is a basic example of 3D image denoising with the wavelet transform library:
 * \code
 * #include "waveletConfigException.h"
 * #include "waveletTransform.h"
 *
 * double* data1d; //double array containing the 3d image of size szx, szy and szz.
 * ...
 * int numScales =  WaveletTransform::computeMaximumScale(szx, szy, szz); //maximum number of scales allowed for the image size
 * double** resTab;
 * try{
 *  resTab = WaveletTransform::b3WaveletScales(data1d, szx, szy, szz, numScales); //compute the wavelet scale coefficients for numScales scales
 * }
 * catch(WaveletConfigException)//catch an exception indicating an invalid configuration of the wavelet transform
 * {
 * printf(e.what());
 * return;
 * }
 * //compute waveletCoefficients from the scale images
 * double* lowPassResidual = new double[szx*szy*szz]; //allocate memory for the low pass residual
 * WaveletTransform::b3WaveletCoefficientsInplace(resTab, data1d, lowPassResidual, numScales, szx*szy*szz);//compute the wavelet coefficients from the wavelet scales and override wavelet scales
 * //nb: overriding scale images save memory usage. Allocating new memory for wavelet coefficient can be done using the b3WaveletCoefficients function:
 * //double** waveletCoefficients = b3WaveletCoefficients(resTab, data1d, numScales, szx*szy*szz);
 *
 * //Denoise wavelet coefficients: set to 0 coefficients which absolute value is lower than a threshold.
 * double threshold = 5; //set the treshold
 * for (int scale = 0; scale < numScales; scale++) // loop over the scale
 * {
 *		double* coefficientsAtScale = resTab[scale]; // get the wavelet coefficients at scale scale
 *		for (int i = 0; i < szx*szy*szz; i++) //loop over the wavelet coefficients
 *		{
 *			if (abs(coefficientsAtScale[i])<threshold)
 *				coefficientsAtScale[i] = 0;
 *		}
 * }
 *
 * //reconstruct now an image from denoise coefficients
 * double* denoisedImage = new double[szx*szy*szz]; //allocate memory for the denoised image
 * WaveletTransform::b3WaveletReconstruction(resTab, lowPassResidual, denoisedImage, numScales, szx*szy*szz); //call reconstruction function with the modified wavelet coefficients and the residual image
 *
 * \endcode
 *
 * \section Authors
 *
 *  Authors are Nicolas Chenouard, Fabrice de Chaumont and Ihor Smal.
 *
 *  \section Licence
 *
 * The wavelet transform library is distributed under the GPL v2.0 licence.
 * 
 */

///////////////////////////// CLASS DEFINITION //////////////////////////////////////

//! Useful functions for computing undecimated B3 spline wavelet transform on 2D and 3D images.
/*! B3 spline wavelet transform on 2D and 3D images.
 * The algorithm implemented is the a trous algorithm (S. Mallat, "A WAVELET TOUR OF SIGNAL PROCESSING", Academic Press, 1988).
 * The input image is passed as a 1d double table with xyz convention (x coordinate varying first, then y, and z).
 *
 * @author Nicolas Chenouard nicolas.chenouard@epfl.ch
 * @author Fabrice de Chaumont
 * @author Ihor Smal
 *
 * @version 1.0
 * @date 8/5/2010
 */

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "waveletConfigException.h"

class WaveletTransform
{

public:

//! Compute the maximum feasible scale for a given 3D image size
/*!
	For a scale i, the minimum size of the image is 5 +4*(2^(i-1)-1) along each direction.
	\param width the image width
	\param height the image height
	\param depth the image depth
	\return throw a WaveletConfigException if the image size is invalid
*/
static int computeMaximumScale(int width, int height, int depth);

//! Compute the maximum feasible scale for a given 2D image size
/*!
	For a scale i, the minimum size of the image is 5 +4*(2^(i-1)-1) along each direction.
	\param width the image width
	\param height the image height
	\return throw a WaveletConfigException if the image size is invalid
*/
static int computeMaximumScale2D(int width, int height);

//! Check that 3D image dimensions complies with the wavelet scale
/*!
	\param width the image width
	\param height the image height
	\param depth the image depth
	\param numScales the number of wavelet scales
	\return throw a WaveletConfigException if the image size is invalid
*/
static void checkImageDimensions(int width, int height, int depth, int numScales) throw (WaveletConfigException);

//! Check that 2D image dimensions complies with the wavelet scale
/*!
	The minimum size is 5+(2^(number of scales -1))*4 for each dimension
	
	\param width the image width
	\param height the image height
	\param numScales the number of wavelet scales
	\return throw a WaveletConfigException if the image size is invalid
*/
static void checkImageDimensions2D(int width, int height, int numScales) throw (WaveletConfigException);

//! Compute the scale images for a 3D image
/*!
	The wavelet coefficients are computed later on as the difference of the subsequent wavelet scale images.
	
	\param dataIn the input origininal as a 1d double array in xyz convention 
	\param width the image width
	\param height the image height
	\param depth the image depth
	\param numScales the number of wavelet scales
	\return Wavelet scales in an array. Each scale is stored as a 1d double array in the xyz convention.
*/
static double**  b3WaveletScales(double* dataIn, int width, int height, int depth, int numScales) throw (WaveletConfigException);

//! 	Compute the wavelet scale images for a 2D image
/*!
	The wavelet coefficients are computed later on as the difference of the subsequent wavelet scale images.
	
	\param dataIn the input origininal as a 1d double array in xy convention 
	\param width the image width
	\param height the image height
	\param numScales the number of wavelet scales
	\return Wavelet scales in an array. Each scale is stored as a 1d double array in the xy convention.
*/
static double**  b3WaveletScales2D(double* dataIn, int width, int height, int numScales) throw (WaveletConfigException);

//! filter a 3D image with the B3 spline wavelet scale kernel in the x direction when using the a trous algorithm
/*!
	Convolve an image with the B3 spline wavelet scale kernel: {1/16, 1/4, 3/8, 1/4, 1/16}.
	When using the a trous algorithm the kernel has to be upsampled with zero values.
	The distance between each non zero coefficient is thus inputted as stepS.
	For optimized performance the output image has swapped dimensions. For instance, for a xyz convention input, the output has yzx convention.
	The image borders are handled with mirror symmetry. :TODO: handle zero paddding too.
	
	\param arrayIn 3D input coefficients to filter in a 1d array.
	\param arrayOut 3D filtered image stored in a 1d array. Dimensions have been swapped with respect to the input array dimension convention.
	\param width the image width
	\param height the image height
	\param depth the image depth
	\param stepS the distance between each non zero coefficient of the convolution kernel
	\return void
*/
static void filterAndSwap(double* arrayIn, double* arrayOut, int width, int height, int depth, int stepS);

//! filter a 2D image with the B3 spline wavelet scale kernel in the x direction when using the a trous algorithm
/*!
	Convolve an image with the B3 spline wavelet scale kernel: {1/16, 1/4, 3/8, 1/4, 1/16}.
	When using the a trous algorithm the kernel has to be upsampled with zero values.
	The distance between each non zero coefficient is thus inputted as stepS.
	For optimized performance the output image has swapped dimensions. For instance, for a xy convention input, the output has yz convention.
	The image borders are handled with mirror symmetry. :TODO: handle zero paddding too.
	
	\param arrayIn 2D input coefficients to filter in a 1d array.
	\param arrayOut 2D filtered image stored in a 1d array. Dimensions have been swapped with respect to the input array dimension convention.
	\param width the image width
	\param height the image height
	\param stepS the distance between each non zero coefficient of the convolution kernel
	\return void
*/
static void filterAndSwap2D(double* arrayIn, double* arrayOut, int width, int height, int stepS);

//! Compute the wavelet coefficients from wavelet scales
/*!
	Compute the wavelet coefficients for a 3D or 2D image.
	The input is the wavelet scale coefficients as outputed by the b3WaveletScales function.
	The wavelet coefficients are returned in 1d double arrays of the same size as the input image.
	The low pass residual image is the last array of the output array.
	
	\param coefficients wavelet scale coefficients
	\param originalImage the original image from which the wavelet scales have been computed
	\param numScales the number of wavelet scales
	\param numVoxels the number of voxels of the 3D image
	\return Wavelet coefficients in an array. Each scale is stored as a 1d double array in the xy convention.
*/
static double** b3WaveletCoefficients(double** coefficients, double* originalImage, int numScales, int numVoxels);

//! Compute the wavelet coefficients and override wavelet scale coefficients
/*!
	Compute the wavelet coefficients for a 3D or 2D image and store them in the same arrays as the wavelet scale coefficients.
	The input is the wavelet scale coefficients as outputed by the b3WaveletScales function.
	This function save memory space as compared to the b3WaveletCoefficients function.
	
	\param coefficients wavelet scale coefficients. Will be overriden to output wavelet coefficients.
	\param originalImage the original image from which the wavelet scales have been computed
	\param lowPass array where to store the low pass residual image
	\param numScales the number of wavelet scales
	\param numVoxels the number of voxels of the 3D image
	\return void
*/
static void b3WaveletCoefficientsInplace(double** coefficients, double* originalImage, double* lowPass, int numScales, int numVoxels);

//! Reconstruct an image from the wavelet coefficients and a low pass residual image
/*!
	Reconstruct a 2D or 3d image from wavelet coefficients and a low pass residual image.
	The reconctruction procedure consists in summing the wavelet coefficients and the residual pointwise.
	
	\param inputCoefficients wavelet coefficients in an array. Each wavelet scale is a 1d image following the xyz convention.
	\param lowPassResidual the low frequency image as 1d image following the xyz convention.
	\param output the reconstructed image as a 1d image following the xyz convention.
	\param numScales the number of wavelet scales
	\param numVoxels the number of voxels of the 3D image
	\return void
*/
static void b3WaveletReconstruction(double** inputCoefficients, double* lowPassResidual, double* output, int numScales, int numVoxels);

};

#endif
