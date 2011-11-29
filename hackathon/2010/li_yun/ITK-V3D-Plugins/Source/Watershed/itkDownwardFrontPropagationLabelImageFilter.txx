/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkDownwardFrontPropagationLabelImageFilter.txx,v $
  Language:  C++
  Date:      $Date: 2009-07-08 02:32:35 $
  Version:   $Revision: 1.11 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkDownwardFrontPropagationLabelImageFilter_txx
#define __itkDownwardFrontPropagationLabelImageFilter_txx

#include "itkDownwardFrontPropagationLabelImageFilter.h"
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
template <class TInputImage, class TFeatureImage, class TOutputImage>
DownwardFrontPropagationLabelImageFilter<TInputImage, TFeatureImage, TOutputImage>
::DownwardFrontPropagationLabelImageFilter()
{
  this->SetNumberOfRequiredInputs(2);

  this->SetBirthThreshold( 1 );

  InputSizeType  ballManhattanRadius;
  ballManhattanRadius.Fill( 1 );
  this->SetRadius( ballManhattanRadius );
}

/**
 * Destructor
 */
template <class TInputImage, class TFeatureImage, class TOutputImage>
DownwardFrontPropagationLabelImageFilter<TInputImage, TFeatureImage, TOutputImage>
::~DownwardFrontPropagationLabelImageFilter()
{
}


/**
 * Standard PrintSelf method.
 */
template <class TInputImage, class TFeatureImage, class TOutputImage>
void
DownwardFrontPropagationLabelImageFilter<TInputImage, TFeatureImage, TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);
}

template <class TInputImage, class TFeatureImage, class TOutputImage>
void 
DownwardFrontPropagationLabelImageFilter<TInputImage, TFeatureImage, TOutputImage>
::SetFeatureImage( const TFeatureImage * imageToSegment )
{
  this->ProcessObject::SetNthInput( 1, const_cast< TFeatureImage * >( imageToSegment ) );
}


template <class TInputImage, class TFeatureImage, class TOutputImage>
void 
DownwardFrontPropagationLabelImageFilter<TInputImage, TFeatureImage, TOutputImage>
::AllocateOutputImageWorkingMemory()
{
  this->Superclass::AllocateOutputImageWorkingMemory();
  this->m_FeatureImage = dynamic_cast< const FeatureImageType * >( itk::ProcessObject::GetInput(1) );
}


template <class TInputImage, class TFeatureImage, class TOutputImage>
bool 
DownwardFrontPropagationLabelImageFilter<TInputImage, TFeatureImage, TOutputImage>
::TestForAcceptingCurrentPixel() const
{
  //
  // Find the location of the current pixel in the image memory buffer
  //
  const OutputImageType * outputImage = this->GetOutputImage();

  const OffsetValueType offset = outputImage->ComputeOffset( this->GetCurrentPixelIndex() );

  const InputFeaturePixelType featureValue = this->m_FeatureImage->GetPixel( this->GetCurrentPixelIndex() );

  if( featureValue < this->m_LowerThreshold )
    {
    return false;
    }

  const OutputImagePixelType  * maskBuffer = outputImage->GetBufferPointer();
  const InputFeaturePixelType * featureBuffer = this->m_FeatureImage->GetBufferPointer();

  const OutputImagePixelType  * currentMaskPixelPointer = maskBuffer + offset;
  const InputFeaturePixelType * currentFeaturePixelPointer = featureBuffer + offset;

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

  const NeighborOffsetArrayType  & neighborBufferOffset = this->GetNeighborBufferOffset();

  NeigborOffsetIterator neighborItr = neighborBufferOffset.begin();

  while( neighborItr != neighborBufferOffset.end() )
    {
    const OutputImagePixelType  * neighborMaskPixelPointer    = currentMaskPixelPointer    + *neighborItr;
    const InputFeaturePixelType * neighborFeaturePixelPointer = currentFeaturePixelPointer + *neighborItr;

    if( *neighborMaskPixelPointer == foregroundValue  &&
        *neighborFeaturePixelPointer > featureValue )  //  Add an option for (>=) here
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
