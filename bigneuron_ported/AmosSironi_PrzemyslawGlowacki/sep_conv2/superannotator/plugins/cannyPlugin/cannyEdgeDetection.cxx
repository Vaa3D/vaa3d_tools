/////////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or       //
// modify it under the terms of the GNU General Public License         //
// version 2 as published by the Free Software Foundation.             //
//                                                                     //
// This program is distributed in the hope that it will be useful, but //
// WITHOUT ANY WARRANTY; without even the implied warranty of          //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   //
// General Public License for more details.                            //
//                                                                     //
// Written and (C) by Aurelien Lucchi and Kevin Smith                  //
// Contact <aurelien.lucchi@gmail.com> for comments & bug reports      //
// Contact <kevin.smith@epfl.ch> for comments & bug reports            //
/////////////////////////////////////////////////////////////////////////

/** \class CannyEdgeDetectionImageFilter
 *
 * This filter is an implementation of a Canny edge detector for scalar-valued
 * images.  Based on John Canny's paper "A Computational Approach to Edge 
 * Detection"(IEEE Transactions on Pattern Analysis and Machine Intelligence, 
 * Vol. PAMI-8, No.6, November 1986),  there are four major steps used in the 
 * edge-detection scheme:
 * (1) Smooth the input image with Gaussian filter.
 * (2) Calculate the second directional derivatives of the smoothed image. 
 * (3) Non-Maximum Suppression: the zero-crossings of 2nd derivative are found,
 *     and the sign of third derivative is used to find the correct extrema. 
 * (4) The hysteresis thresholding is applied to the gradient magnitude
 *      (multiplied with zero-crossings) of the smoothed image to find and 
 *      link edges.
 *
 * \par Inputs and Outputs
 * The input to this filter should be a scalar, real-valued Itk image of
 * arbitrary dimension.  The output should also be a scalar, real-value Itk
 * image of the same dimensionality.
 *
 * \par Parameters
 * There are four parameters for this filter that control the sub-filters used
 * by the algorithm.
 *
 * \par 
 * Variance and Maximum error are used in the Gaussian smoothing of the input
 * image.  See  itkDiscreteGaussianImageFilter for information on these
 * parameters.
 *
 * \par
 * Threshold is the lowest allowed value in the output image.  Its data type is 
 * the same as the data type of the output image. Any values below the
 * Threshold level will be replaced with the OutsideValue parameter value, whose
 * default is zero.
 * 
 * \todo Edge-linking will be added when an itk connected component labeling
 * algorithm is available.
 *
 * \sa DiscreteGaussianImageFilter
 * \sa ZeroCrossingImageFilter
 * \sa ThresholdImageFilter
 *
 * @param nc : Number of channels
 *
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

// ITK Header Files
#include "itkImage.h"
#include "itkCannyEdgeDetectionImageFilter.h"
#include "itkImportImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkRecursiveGaussianImageFilter.h"

template <typename TInputPixelType, typename TOutputPixelType>
void canny(unsigned char* rawData, long nx, long ny, long nz, long nc,
           unsigned char lowerTh, unsigned char upperTh,
           TOutputPixelType*& output1d,
           float gaussian_var = 10.0f,
           bool preSmoothing = false,
           float preSmoothing_var = 5.0f)
{
  long offsets = 0;

  typedef TInputPixelType  PixelType;

  PixelType * data1d = reinterpret_cast< PixelType * >(rawData);

  long pagesz = nx*ny*nz;		
  unsigned long int numberOfPixels = pagesz*nc;

  const unsigned int Dimension = 3;
		
  typedef itk::Image< TInputPixelType, Dimension > InputImageType;
  typedef itk::Image< TOutputPixelType, Dimension > OutputImageType;
  typedef itk::ImportImageFilter< TInputPixelType, Dimension > ImportFilterType;
		
  typename ImportFilterType::Pointer importFilter = ImportFilterType::New();
		
  typename ImportFilterType::SizeType size;
  size[0] = nx;
  size[1] = ny;
  size[2] = nz;
		
  typename ImportFilterType::IndexType start;
  start.Fill( 0 );
		
  typename ImportFilterType::RegionType region;
  region.SetIndex(start);
  region.SetSize(size);
  importFilter->SetRegion(region);
		
  typename InputImageType::PointType origin;
  origin.Fill(0.0);
  importFilter->SetOrigin( origin );
		
  typename ImportFilterType::SpacingType spacing;
  spacing.Fill(1.0);
  importFilter->SetSpacing(spacing);
		
  const bool importImageFilterWillOwnTheBuffer = false;
		
  typedef itk::CastImageFilter< InputImageType, OutputImageType> CastImageFilterType;
  typename CastImageFilterType::Pointer castImageFilter = CastImageFilterType::New();
		
  typedef itk::CannyEdgeDetectionImageFilter<OutputImageType, OutputImageType> CannyEdgeDetectionType;
  typename CannyEdgeDetectionType::Pointer cannyedgedetectionFilter = CannyEdgeDetectionType::New();	     

  float maxerr = 0.5; //0.5  //Maximum Error Must be in the range [ 0.0 , 1.0 ]

  try
    {
      output1d = new TOutputPixelType [numberOfPixels];
    }
  catch(...)
    {
      std::cerr << "[CannyEdgeDetection] Error while allocating memory." << std::endl;
      return;
    }

  typedef itk::RecursiveGaussianImageFilter<InputImageType, InputImageType> GaussianImageFilterType;
  typename GaussianImageFilterType::Pointer gaussianImageFilter = 0;
  if(preSmoothing)
    {
      // smooth the image
      typedef itk::RecursiveGaussianImageFilter<InputImageType, InputImageType> GaussianImageFilterType;
      gaussianImageFilter = GaussianImageFilterType::New();
      gaussianImageFilter->SetZeroOrder();
      gaussianImageFilter->SetSigma(preSmoothing_var);
    }
		
  const bool filterWillDeleteTheInputBuffer = false;
				
  for(long ch=0; ch<nc; ch++)
    {
      offsets = ch*pagesz;
					
      TOutputPixelType *p = output1d+offsets;
					
      importFilter->SetImportPointer( data1d+offsets, pagesz, importImageFilterWillOwnTheBuffer );


      if(preSmoothing)
        {
          gaussianImageFilter->SetInput( importFilter->GetOutput() );
          castImageFilter->SetInput( gaussianImageFilter->GetOutput() );
        }
      else
        castImageFilter->SetInput( importFilter->GetOutput() );

      try
        {
          castImageFilter->Update();
        }
      catch( itk::ExceptionObject & excp)
        {
          std::cerr << "Error run this filter." << std::endl;
          std::cerr << excp << std::endl;
          return;
        }
					
      cannyedgedetectionFilter->SetInput( castImageFilter->GetOutput() );
					
      cannyedgedetectionFilter->SetVariance(gaussian_var);
      cannyedgedetectionFilter->SetMaximumError(maxerr);
      cannyedgedetectionFilter->SetLowerThreshold(lowerTh);
      cannyedgedetectionFilter->SetUpperThreshold(upperTh);
					
      cannyedgedetectionFilter->GetOutput()->GetPixelContainer()->SetImportPointer( p, pagesz, filterWillDeleteTheInputBuffer);
					
      try
        {
          cannyedgedetectionFilter->Update();
        }
      catch( itk::ExceptionObject & excp)
        {
          std::cerr << "Error while running this filter." << std::endl;
          std::cerr << excp << std::endl;
          return;
        }
					
    }
}

//------------------------------------------------------------------------------


template <typename TInputPixelType, typename TOutputPixelType, typename OutputImageType>
void canny(unsigned char* rawData, long nx, long ny, long nz, long nc,
           unsigned char lowerTh, unsigned char upperTh,
           //OutputImageType*& outputImage,
           typename OutputImageType::Pointer& outputImage,
           float gaussian_var = 10.0f,
           bool preSmoothing = false,
           float preSmoothing_var = 5.0f)
{
  long offsets = 0;

  typedef TInputPixelType  PixelType;

  PixelType * data1d = reinterpret_cast< PixelType * >(rawData);

  long pagesz = nx*ny*nz;		
  unsigned long int numberOfPixels = pagesz*nc;

  const unsigned int Dimension = 3;
		
  typedef itk::Image< TInputPixelType, Dimension > InputImageType;
  //typedef itk::Image< TOutputPixelType, Dimension > OutputImageType;
  typedef itk::ImportImageFilter< TInputPixelType, Dimension > ImportFilterType;
		
  typename ImportFilterType::Pointer importFilter = ImportFilterType::New();
		
  typename ImportFilterType::SizeType size;
  size[0] = nx;
  size[1] = ny;
  size[2] = nz;
		
  typename ImportFilterType::IndexType start;
  start.Fill( 0 );
		
  typename ImportFilterType::RegionType region;
  region.SetIndex(start);
  region.SetSize(size);
  importFilter->SetRegion(region);
		
  typename InputImageType::PointType origin;
  origin.Fill(0.0);
  importFilter->SetOrigin( origin );
		
  typename ImportFilterType::SpacingType spacing;
  spacing.Fill(1.0);
  importFilter->SetSpacing(spacing);
		
  const bool importImageFilterWillOwnTheBuffer = false;
		
  typedef itk::CastImageFilter< InputImageType, OutputImageType> CastImageFilterType;
  typename CastImageFilterType::Pointer castImageFilter = CastImageFilterType::New();
		
  typedef itk::CannyEdgeDetectionImageFilter<OutputImageType, OutputImageType> CannyEdgeDetectionType;
  typename CannyEdgeDetectionType::Pointer cannyedgedetectionFilter = CannyEdgeDetectionType::New();	     

  float maxerr = 0.5; //0.5  //Maximum Error Must be in the range [ 0.0 , 1.0 ]

  TOutputPixelType* output1d = 0;
  try
    {
      output1d = new TOutputPixelType [numberOfPixels];
    }
  catch(...)
    {
      std::cerr << "[CannyEdgeDetection] Error while allocating memory." << std::endl;
      return;
    }

  typedef itk::RecursiveGaussianImageFilter<InputImageType, InputImageType> GaussianImageFilterType;
  typename GaussianImageFilterType::Pointer gaussianImageFilter = 0;
  if(preSmoothing)
    {
      // smooth the image
      typedef itk::RecursiveGaussianImageFilter<InputImageType, InputImageType> GaussianImageFilterType;
      gaussianImageFilter = GaussianImageFilterType::New();
      gaussianImageFilter->SetZeroOrder();
      gaussianImageFilter->SetSigma(preSmoothing_var);
    }
		
  const bool filterWillDeleteTheInputBuffer = false;
		
  outputImage = 0;
		
  for(long ch=0; ch<nc; ch++)
    {
      offsets = ch*pagesz;
					
      TOutputPixelType *p = output1d+offsets;
					
      importFilter->SetImportPointer( data1d+offsets, pagesz, importImageFilterWillOwnTheBuffer );


      if(preSmoothing)
        {
          gaussianImageFilter->SetInput( importFilter->GetOutput() );
          castImageFilter->SetInput( gaussianImageFilter->GetOutput() );
        }
      else
        castImageFilter->SetInput( importFilter->GetOutput() );

      try
        {
          castImageFilter->Update();
        }
      catch( itk::ExceptionObject & excp)
        {
          std::cerr << "Error while running this filter." << std::endl;
          std::cerr << excp << std::endl;
          return;
        }
					
      cannyedgedetectionFilter->SetInput( castImageFilter->GetOutput() );
					
      cannyedgedetectionFilter->SetVariance(gaussian_var);
      cannyedgedetectionFilter->SetMaximumError(maxerr);
      cannyedgedetectionFilter->SetLowerThreshold(lowerTh);
      cannyedgedetectionFilter->SetUpperThreshold(upperTh);
					
      cannyedgedetectionFilter->GetOutput()->GetPixelContainer()->SetImportPointer( p, pagesz, filterWillDeleteTheInputBuffer);
					
      try
        {
          cannyedgedetectionFilter->Update();
        }
      catch( itk::ExceptionObject & excp)
        {
          std::cerr << "Error run this filter." << std::endl;
          std::cerr << excp << std::endl;
          return;
        }					
    }

  outputImage = cannyedgedetectionFilter->GetOutput();
}
