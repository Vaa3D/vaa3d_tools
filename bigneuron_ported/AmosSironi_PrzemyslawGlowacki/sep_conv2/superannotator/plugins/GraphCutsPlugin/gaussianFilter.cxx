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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

// ITK Header Files
#include "itkImage.h"
#include "itkRecursiveGaussianImageFilter.h"
#include "itkImportImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"

/**
 * @param direction : 0=X, 1=Y, 2=Z
 */
template <typename TInputPixelType, typename TOutputPixelType>
void gaussianGradientFilter(unsigned char* rawData, long nx, long ny, long nz, long nc,
                            int direction,
                            TOutputPixelType*& output1d)
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
  region.SetIndex( start );
  region.SetSize(  size  );
		
  importFilter->SetRegion( region );
		
  region.SetSize( size );
		
  typename InputImageType::PointType origin;
  origin.Fill( 0.0 );
		
  importFilter->SetOrigin( origin );
		
		
  typename ImportFilterType::SpacingType spacing;
  spacing.Fill( 1.0 );
		
  importFilter->SetSpacing( spacing );
		
  const bool importImageFilterWillOwnTheBuffer = false;
		
  typedef itk::CastImageFilter< InputImageType, OutputImageType> CastImageFilterType;
  typename CastImageFilterType::Pointer castImageFilter = CastImageFilterType::New();
		
  typedef itk::RecursiveGaussianImageFilter<OutputImageType, OutputImageType> GaussianImageFilterType;
  typename GaussianImageFilterType::Pointer gaussianImageFilter = GaussianImageFilterType::New();


  try
    {
      output1d = new TOutputPixelType [numberOfPixels];
    }
  catch(...)
    {
      std::cerr << "[GaussianFilter] Error while allocating memory." << std::endl;
      return;
    }
				
  const bool filterWillDeleteTheInputBuffer = false;
				
  for(long ch=0; ch<nc; ch++)
    {
      offsets = ch*pagesz;
					
      TOutputPixelType *p = output1d+offsets;
					
      importFilter->SetImportPointer( data1d+offsets, pagesz, importImageFilterWillOwnTheBuffer );
					
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
					
      gaussianImageFilter->SetInput( castImageFilter->GetOutput() );					
      gaussianImageFilter->SetDirection(direction);
      gaussianImageFilter->SetFirstOrder();
      gaussianImageFilter->SetSigma(3);
					
      gaussianImageFilter->GetOutput()->GetPixelContainer()->SetImportPointer( p, pagesz, filterWillDeleteTheInputBuffer);
					
      try
        {
          gaussianImageFilter->Update();
        }
      catch( itk::ExceptionObject & excp)
        {
          std::cerr << "Error while running this filter." << std::endl;
          std::cerr << excp << std::endl;
          return;
        }
					
    }
}


template <typename TInputPixelType, typename TOutputPixelType>
void gradientMagnitude(unsigned char* rawData, long nx, long ny, long nz, long nc,
                       float gaussianVariance, TOutputPixelType*& output1d)
{
  typedef TInputPixelType  PixelType;
	
  PixelType * data1d = reinterpret_cast< PixelType * >(rawData);
  long pagesz = nx*ny*nz;
  unsigned long int numberOfPixels = pagesz*nc;
  long offsets = 0;
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
  region.SetIndex( start );
  region.SetSize(  size  );
		
  importFilter->SetRegion( region );	
  region.SetSize( size );
		
  typename InputImageType::PointType origin;
  origin.Fill( 0.0 );
		
  importFilter->SetOrigin( origin );
		
  typename ImportFilterType::SpacingType spacing;
  spacing.Fill( 1.0 );	
  importFilter->SetSpacing( spacing );
		
  const bool importImageFilterWillOwnTheBuffer = false;
		
  typedef itk::GradientMagnitudeRecursiveGaussianImageFilter<InputImageType, OutputImageType> GradientType;
  typename GradientType::Pointer gradientFilter = GradientType::New();

  gradientFilter->SetSigma(gaussianVariance);

  try
    {
      output1d = new TOutputPixelType [numberOfPixels];
    }
  catch(...)
    {
      std::cerr << "[F_GradientStats] Error while allocating memory." << std::endl;
      return;
    }
				
  const bool filterWillDeleteTheInputBuffer = false;
				
  for(long ch=0; ch<nc; ch++)
    {
      offsets = ch*pagesz;
      TOutputPixelType *p = output1d+offsets;
					
      importFilter->SetImportPointer( data1d+offsets, pagesz, importImageFilterWillOwnTheBuffer );
      gradientFilter->SetInput( importFilter->GetOutput() );

      gradientFilter->GetOutput()->GetPixelContainer()->SetImportPointer( p, pagesz, filterWillDeleteTheInputBuffer);
					
      try
        {
          gradientFilter->Update();
        }
      catch( itk::ExceptionObject & excp)
        {
          std::cerr << "[F_GradientStats] Error ehile running this filter." << std::endl;
          std::cerr << excp << std::endl;
          return;
        }

    }
}


