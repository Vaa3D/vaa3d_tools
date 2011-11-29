/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkHoleFillingFrontPropagationLabelImageFilter.txx,v $
  Language:  C++
  Date:      $Date: 2009-07-08 02:32:35 $
  Version:   $Revision: 1.11 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkHoleFillingFrontPropagationLabelImageFilter_txx
#define __itkHoleFillingFrontPropagationLabelImageFilter_txx

#include "itkHoleFillingFrontPropagationLabelImageFilter.h"
#include "itkConstNeighborhoodIterator.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionExclusionIteratorWithIndex.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkOffset.h"
#include "itkProgressReporter.h"

namespace itk
{

/**
 * Constructor
 */
template <class TInputImage, class TOutputImage>
HoleFillingFrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::HoleFillingFrontPropagationLabelImageFilter()
{
  InputSizeType  ballManhattanRadius;
  ballManhattanRadius.Fill( 2 );
  this->SetRadius( ballManhattanRadius );

  this->SetMajorityThreshold( 1 );
}

/**
 * Destructor
 */
template <class TInputImage, class TOutputImage>
HoleFillingFrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::~HoleFillingFrontPropagationLabelImageFilter()
{
}


/**
 * Standard PrintSelf method.
 */
template <class TInputImage, class TOutputImage>
void
HoleFillingFrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);
}


template <class TInputImage, class TOutputImage>
void 
HoleFillingFrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::GenerateData()
{
  this->AllocateOutputImageWorkingMemory();
  this->ComputeBirthThreshold();
  this->FindAllPixelsInTheBoundaryAndAddThemAsSeeds();
  this->IterateFrontPropagations();
}


template <class TInputImage, class TOutputImage>
void 
HoleFillingFrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::ComputeBirthThreshold()
{
  this->InitializeNeighborhood();

  const unsigned int neighborhoodSize = this->GetNeighborhoodSize();

  // Take the number of neighbors, discount the central pixel, and take half of them.
  unsigned int threshold = static_cast<unsigned int>( (neighborhoodSize - 1 ) / 2.0 ); 

  // add the majority threshold.
  threshold += this->GetMajorityThreshold();

  // Set that number as the Birth Threshold
  this->SetBirthThreshold( threshold );
}


template <class TInputImage, class TOutputImage>
bool 
HoleFillingFrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::TestForAcceptingCurrentPixel() const
{
  //
  // Find the location of the current pixel in the image memory buffer
  //
  const OutputImageType * outputImage = this->GetOutputImage();

  const OffsetValueType offset = outputImage->ComputeOffset( this->GetCurrentPixelIndex() );

  const OutputImagePixelType  * maskBuffer = outputImage->GetBufferPointer();

  const OutputImagePixelType  * currentMaskPixelPointer = maskBuffer + offset;

  const OutputImagePixelType foregroundValue = this->GetForegroundValue();
  const OutputImagePixelType backgroundValue = this->GetBackgroundValue();

  if( *currentMaskPixelPointer != backgroundValue )
    {
    return false;
    }

  unsigned int numberOfNeighborsAtForegroundValue = 0;

  //
  // From that buffer position, visit all other neighbor pixels 
  // and check if they are set to the foreground value.
  //
  typedef typename NeighborOffsetArrayType::const_iterator   NeigborOffsetIterator;

  const NeighborOffsetArrayType & neighborOffsetArray = this->GetNeighborBufferOffset();

  NeigborOffsetIterator neighborItr = neighborOffsetArray.begin();

  while( neighborItr != neighborOffsetArray.end() )
    {
    const OutputImagePixelType  * neighborMaskPixelPointer    = currentMaskPixelPointer    + *neighborItr;

    if( *neighborMaskPixelPointer == foregroundValue )
      {
      numberOfNeighborsAtForegroundValue++;
      }

    ++neighborItr;
    }

  bool quorum = ( numberOfNeighborsAtForegroundValue > this->GetBirthThreshold() );

  return quorum;
}

} // end namespace itk

#endif
