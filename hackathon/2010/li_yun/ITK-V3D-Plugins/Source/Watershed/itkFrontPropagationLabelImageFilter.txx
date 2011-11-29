/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkFrontPropagationLabelImageFilter.txx,v $
  Language:  C++
  Date:      $Date: 2009-07-08 02:32:35 $
  Version:   $Revision: 1.11 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkFrontPropagationLabelImageFilter_txx
#define __itkFrontPropagationLabelImageFilter_txx

#include "itkFrontPropagationLabelImageFilter.h"
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
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::FrontPropagationLabelImageFilter()
{
  this->m_MaximumNumberOfIterations = 10;
  this->m_CurrentIterationNumber = 0;

  this->m_TotalNumberOfPixelsChanged = NumericTraits<SizeValueType>::Zero;

  this->m_SeedArray1 = NULL;
  this->m_SeedArray2 = NULL;

  this->m_OutputImage = NULL;

  this->SetNumberOfRequiredInputs(1);

  this->SetBackgroundValue( 0 );
}

/**
 * Destructor
 */
template <class TInputImage, class TOutputImage>
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::~FrontPropagationLabelImageFilter()
{
  SeedArrayMapIterator sitr1 = this->m_SeedArrayMap1.begin();

  while( sitr1 != this->m_SeedArrayMap1.end() )
    {
    delete sitr1->second;
    ++sitr1;
    }

  SeedArrayMapIterator sitr2 = this->m_SeedArrayMap2.begin();

  while( sitr2 != this->m_SeedArrayMap2.end() )
    {
    delete sitr2->second;
    ++sitr2;
    }
}


/**
 * Standard PrintSelf method.
 */
template <class TInputImage, class TOutputImage>
void
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);
}


/**
 * Get Output Image
 */
template <class TInputImage, class TOutputImage>
const typename FrontPropagationLabelImageFilter<TInputImage, TOutputImage>::OutputImageType *
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::GetOutputImage() const
{
  return this->m_OutputImage;
}


/**
 * Get Output Image
 */
template <class TInputImage, class TOutputImage>
typename FrontPropagationLabelImageFilter<TInputImage, TOutputImage>::OutputImageType *
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::GetOutputImage()
{
  return this->m_OutputImage;
}


template <class TInputImage, class TOutputImage>
void 
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::GenerateData()
{
  this->AllocateOutputImageWorkingMemory();
  this->InitializeNeighborhood();
  this->FindAllPixelsInTheBoundaryAndAddThemAsSeeds();
  this->IterateFrontPropagations();
}


template <class TInputImage, class TOutputImage>
void 
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::IterateFrontPropagations()
{
  this->m_CurrentIterationNumber = 0;
  this->m_TotalNumberOfPixelsChanged = NumericTraits<SizeValueType>::Zero;

  // Progress reporting
  ProgressReporter progress(this, 0, m_MaximumNumberOfIterations);
  
  while( this->m_CurrentIterationNumber < this->m_MaximumNumberOfIterations ) 
    {
    this->m_TotalNumberOfPixelsChangedInLastIteration = NumericTraits<SizeValueType>::Zero;

    this->VisitAllSeedsAndTransitionTheirState();
    this->m_CurrentIterationNumber++;

    progress.CompletedPixel();   // not really a pixel but an iteration

    this->InvokeEvent( IterationEvent() );
    
    if( this->m_TotalNumberOfPixelsChangedInLastIteration ==  NumericTraits<SizeValueType>::Zero )
      {
      break;
      }
    }
}


template <class TInputImage, class TOutputImage>
void 
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::AllocateOutputImageWorkingMemory()
{
  this->m_OutputImage  = this->GetOutput();
  OutputImageRegionType region =  this->m_OutputImage->GetRequestedRegion();

  // Allocate memory for the output image itself.
  this->m_OutputImage->SetBufferedRegion( region );
  this->m_OutputImage->Allocate();
  this->m_OutputImage->FillBuffer( 0 );

  this->m_SeedsMask = SeedMaskImageType::New();
  this->m_SeedsMask->SetRegions( region );
  this->m_SeedsMask->Allocate();
  this->m_SeedsMask->FillBuffer( 0 );
}

 
template <class TInputImage, class TOutputImage>
void 
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::InitializeNeighborhood()
{
  this->m_Neighborhood.SetRadius( this->GetRadius() );

  this->ComputeArrayOfNeighborhoodBufferOffsets();
}


template <class TInputImage, class TOutputImage>
void 
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::FindAllPixelsInTheBoundaryAndAddThemAsSeeds()
{
  const InputImageType * inputImage = this->GetInput();

  OutputImageRegionType region =  inputImage->GetRequestedRegion();

  ConstNeighborhoodIterator< InputImageType >   bit;
  ImageRegionIterator< OutputImageType >        itr;
  ImageRegionIterator< SeedMaskImageType >      mtr;
  
  const InputSizeType & radius = this->GetRadius();

  // Find the data-set boundary "faces"
  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType faceList;
  NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType> bC;
  faceList = bC(inputImage, region, radius);

  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType::iterator fit;

  // Process only the internal face
  fit = faceList.begin();
  
  this->m_InternalRegion = *fit;

  // Mark all the pixels in the boundary of the seed image as visited
  typedef itk::ImageRegionExclusionIteratorWithIndex< SeedMaskImageType > ExclusionIteratorType;
  ExclusionIteratorType exIt( this->m_SeedsMask, region );

  exIt.SetExclusionRegion( this->m_InternalRegion );
  exIt.GoToBegin();

  while( !exIt.IsAtEnd() )
    {
    exIt.Set( 255 );
    ++exIt;
    }

  bit = ConstNeighborhoodIterator<InputImageType>( radius, inputImage, this->m_InternalRegion );
  itr  = ImageRegionIterator<OutputImageType>(    this->m_OutputImage, this->m_InternalRegion );
  mtr  = ImageRegionIterator<SeedMaskImageType>(  this->m_SeedsMask,   this->m_InternalRegion );

  bit.GoToBegin();
  itr.GoToBegin();
  mtr.GoToBegin();
  
  unsigned int neighborhoodSize = bit.Size();

  const InputImagePixelType backgroundValue = this->GetBackgroundValue();
  
  this->m_SeedArrayMap1.clear();
  this->m_SeedArrayMap2.clear();
  this->m_SeedsNewValuesMap.clear();

  while ( ! bit.IsAtEnd() )
    {
    const InputImagePixelType value = bit.GetCenterPixel();

    if( value != backgroundValue )
      {
      itr.Set( value );
      mtr.Set( 255 );

      if( this->m_NumberOfPixels.find(value) == this->m_NumberOfPixels.end() )
        {
        this->m_NumberOfPixels[value] = NumericTraits<SizeValueType>::One;
        }
      this->m_NumberOfPixels[value]++;  
      }
    else
      {
      itr.Set( backgroundValue );
      mtr.Set( 0 );
      
      // Search for foreground pixels in the neighborhood
      for (unsigned int i = 0; i < neighborhoodSize; ++i)
        {
        const InputImagePixelType value = bit.GetPixel(i);

        if( value != backgroundValue )
          {
          // If we don't have a container for this label, then create one.
          if( this->m_SeedArrayMap1.find(value) == this->m_SeedArrayMap1.end() )
            {
            this->m_SeedArrayMap1[value] = new SeedArrayType;
            this->m_SeedArrayMap2[value] = new SeedArrayType;
            // Initialized to non-zero in order to force at least one cycle to happen
            this->m_NumberOfPixelsChangedInLastIteration[value] = NumericTraits<SizeValueType>::One;
            }

          this->m_SeedArrayMap1[value]->push_back( bit.GetIndex() );
          break;
          }
        }
      }   
    ++bit;
    ++itr;
    ++mtr;
    }


  SeedArrayMapIterator sitr = this->m_SeedArrayMap1.begin();

  while( sitr != this->m_SeedArrayMap1.end() )
    {
    const LabelType label = sitr->first;
    this->m_SeedsNewValuesMap[label] = SeedNewValuesArrayType();
    this->m_SeedsNewValuesMap[label].reserve( this->m_SeedArrayMap1[label]->size() ); 
    ++sitr;
    }
}


template <class TInputImage, class TOutputImage>
void 
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::FindAllPixelsInTheBoundaryAndAddThemAsSeeds2()
{
  const InputImageType * inputImage = this->GetInput();

  OutputImageRegionType region =  inputImage->GetRequestedRegion();

  ConstNeighborhoodIterator< InputImageType >   bit;
  ImageRegionIterator< OutputImageType >        itr;
  ImageRegionIterator< SeedMaskImageType >      mtr;
  
  const InputSizeType & radius = this->GetRadius();

  // Find the data-set boundary "faces"
  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType faceList;
  NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType> bC;
  faceList = bC(inputImage, region, radius);

  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType::iterator fit;

  // Process only the internal face
  fit = faceList.begin();
  
  this->m_InternalRegion = *fit;

  // Mark all the pixels in the boundary of the seed image as visited
  typedef itk::ImageRegionExclusionIteratorWithIndex< SeedMaskImageType > ExclusionIteratorType;
  ExclusionIteratorType exIt( this->m_SeedsMask, region );

  exIt.SetExclusionRegion( this->m_InternalRegion );
  exIt.GoToBegin();

  while( !exIt.IsAtEnd() )
    {
    exIt.Set( 255 );
    ++exIt;
    }

  bit = ConstNeighborhoodIterator<InputImageType>( radius, inputImage, this->m_InternalRegion );
  itr  = ImageRegionIterator<OutputImageType>(    this->m_OutputImage, this->m_InternalRegion );
  mtr  = ImageRegionIterator<SeedMaskImageType>(  this->m_SeedsMask,   this->m_InternalRegion );

  bit.GoToBegin();
  itr.GoToBegin();
  mtr.GoToBegin();
  
  unsigned int neighborhoodSize = bit.Size();

  const InputImagePixelType backgroundValue = this->GetBackgroundValue();
  
  this->m_SeedArrayMap1.clear();
  this->m_SeedArrayMap2.clear();
  this->m_SeedsNewValuesMap.clear();

  while ( ! bit.IsAtEnd() )
    {
    const InputImagePixelType value = bit.GetCenterPixel();

    if( value != backgroundValue )
      {
      itr.Set( value );
      mtr.Set( 255 );

      if( this->m_NumberOfPixels.find(value) == this->m_NumberOfPixels.end() )
        {
        this->m_NumberOfPixels[value] = NumericTraits<SizeValueType>::One;
        }
      this->m_NumberOfPixels[value]++;  

      // Search for foreground pixels in the neighborhood
      for (unsigned int i = 0; i < neighborhoodSize; ++i)
        {
        const InputImagePixelType neighborValue = bit.GetPixel(i);

        if( neighborValue != backgroundValue )
          {
          if( neighborValue != value )
            {
            // If we don't have a container for that label, then create one.
            if( this->m_SeedArrayMap1.find(neighborValue) == this->m_SeedArrayMap1.end() )
              {
              this->m_SeedArrayMap1[neighborValue] = new SeedArrayType;
              this->m_SeedArrayMap2[neighborValue] = new SeedArrayType;
              }
            this->m_SeedArrayMap1[neighborValue]->push_back( bit.GetIndex() );
            }
          }
        }
      }   
    else
      {
      itr.Set( backgroundValue );
      mtr.Set( 0 );
      }
      
    ++bit;
    ++itr;
    ++mtr;
    }


  SeedArrayMapIterator sitr = this->m_SeedArrayMap1.begin();

  while( sitr != this->m_SeedArrayMap1.end() )
    {
    const LabelType label = sitr->first;
    this->m_SeedsNewValuesMap[label] = SeedNewValuesArrayType();
    this->m_SeedsNewValuesMap[label].reserve( this->m_SeedArrayMap1[label]->size() ); 
    ++sitr;
    }
}
template <class TInputImage, class TOutputImage>
void 
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::VisitAllSeedsAndTransitionTheirState()
{

   SeedArrayMapIterator sitr1 = this->m_SeedArrayMap1.begin();
   SeedArrayMapIterator sitr2 = this->m_SeedArrayMap2.begin();

   while( sitr1 != this->m_SeedArrayMap1.end() )
    {
    const LabelType label = sitr1->first;

    if( this->m_NumberOfPixelsChangedInLastIteration[label] > NumericTraits<SizeValueType>::Zero )
      {
      this->m_SeedArray1 = sitr1->second;
      this->m_SeedArray2 = sitr2->second;

      this->VisitAllSeedsAndTransitionTheirState( label );

      sitr1->second = this->m_SeedArray1;
      sitr2->second = this->m_SeedArray2;
      }

    ++sitr1;
    ++sitr2;
    }
}


template <class TInputImage, class TOutputImage>
void 
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::VisitAllSeedsAndTransitionTheirState(LabelType label)
{
  this->SetForegroundValue( label );

  this->m_NumberOfPixelsChangedInLastIteration[label] = NumericTraits<SizeValueType>::Zero;

  typedef typename SeedArrayType::const_iterator   SeedIterator;

  SeedIterator seedItr = this->m_SeedArray1->begin();

  // Clear the array of new values
  this->m_SeedsNewValuesMap[label].clear();

  while( seedItr != this->m_SeedArray1->end() )
    {
    this->m_CurrentPixelIndex = *seedItr;

    if( this->TestForAcceptingCurrentPixel() )
      {
      this->m_SeedsNewValuesMap[label].push_back( label );
      this->PutCurrentPixelNeighborsIntoSeedArray();
      this->m_NumberOfPixels[label]++;
      this->m_NumberOfPixelsChangedInLastIteration[label]++;
      }
    else
      {
      this->m_SeedsNewValuesMap[label].push_back( this->GetBackgroundValue() );
      // Keep the seed to try again in the next iteration.
      this->m_SeedArray2->push_back( this->GetCurrentPixelIndex() );
      }

    ++seedItr;
    }

  this->PasteNewSeedValuesToOutputImage(label);
   
  this->m_TotalNumberOfPixelsChanged += this->m_NumberOfPixelsChangedInLastIteration[label];
  this->m_TotalNumberOfPixelsChangedInLastIteration += this->m_NumberOfPixelsChangedInLastIteration[label];

  // Now that the values have been copied to the output image, we can empty the
  // array in preparation for the next iteration
  this->m_SeedsNewValuesMap[label].clear();

  this->SwapSeedArrays();
  this->ClearSecondSeedArray();
}


template <class TInputImage, class TOutputImage>
void 
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::PasteNewSeedValuesToOutputImage( LabelType label )
{
  //
  //  Paste new values into the output image
  //
  typedef typename SeedArrayType::const_iterator   SeedIterator;

  SeedIterator seedItr = this->m_SeedArray1->begin();

  typedef typename SeedNewValuesArrayType::const_iterator   SeedsNewValuesIterator;

  SeedsNewValuesIterator newValueItr = this->m_SeedsNewValuesMap[label].begin();

  while (seedItr != this->m_SeedArray1->end() )
    {
    this->m_OutputImage->SetPixel( *seedItr, *newValueItr );
    ++seedItr;
    ++newValueItr;
    }
}

template <class TInputImage, class TOutputImage>
void 
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::SwapSeedArrays()
{
  SeedArrayType * temporary = this->m_SeedArray1;
  this->m_SeedArray1 = this->m_SeedArray2;
  this->m_SeedArray2 = temporary;
}


template <class TInputImage, class TOutputImage>
void 
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::ClearSecondSeedArray()
{
  delete this->m_SeedArray2;
  this->m_SeedArray2 = new SeedArrayType;
}


template <class TInputImage, class TOutputImage>
bool 
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::TestForAcceptingCurrentPixel() const
{
  // This method is intended to be overloaded in a derived class.
  return false;
}


template <class TInputImage, class TOutputImage>
const typename  FrontPropagationLabelImageFilter<TInputImage, TOutputImage>::NeighborOffsetArrayType & 
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::GetNeighborBufferOffset() const
{
  return this->m_NeighborBufferOffset;
}


template <class TInputImage, class TOutputImage>
void 
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::PutCurrentPixelNeighborsIntoSeedArray()
{
  //
  // Find the location of the current pixel in the image memory buffer
  //
  const OffsetValueType offset = this->m_OutputImage->ComputeOffset( this->GetCurrentPixelIndex() );

  const InputImagePixelType * buffer = this->m_OutputImage->GetBufferPointer();

  const InputImagePixelType * currentPixelPointer = buffer + offset;


  const unsigned int neighborhoodSize = this->m_Neighborhood.Size();

  //
  // Visit the offset of each neighbor in Index as well as buffer space
  // and if they are backgroundValue then insert them as new seeds
  //
  typedef typename NeighborhoodType::OffsetType    NeighborOffsetType;

  typedef typename NeighborOffsetArrayType::const_iterator   NeigborOffsetIterator;

  NeigborOffsetIterator neighborItr = this->m_NeighborBufferOffset.begin();

  const InputImagePixelType backgroundValue = this->GetBackgroundValue();

  for( unsigned int i = 0; i < neighborhoodSize; ++i, ++neighborItr )
    {
    const InputImagePixelType * neighborPixelPointer = currentPixelPointer + *neighborItr;

    if( *neighborPixelPointer == backgroundValue )
      {
      NeighborOffsetType neighborOffset = this->m_Neighborhood.GetOffset(i);
      IndexType neighborIndex = this->GetCurrentPixelIndex() + neighborOffset;

      if( this->m_InternalRegion.IsInside( neighborIndex ) )
        {
        if( this->m_SeedsMask->GetPixel( neighborIndex ) == 0 )
          {
          this->m_SeedArray2->push_back( neighborIndex );
          this->m_SeedsMask->SetPixel( neighborIndex, 255 );
          }
        }
      }
    }

}


template <class TInputImage, class TOutputImage>
unsigned int
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::GetNeighborhoodSize() const
{
  return this->m_Neighborhood.Size();
}


template <class TInputImage, class TOutputImage>
void 
FrontPropagationLabelImageFilter<TInputImage, TOutputImage>
::ComputeArrayOfNeighborhoodBufferOffsets()
{
  //
  // Copy the offsets from the Input image.
  // We assume that they are the same for the output image.
  //
  const size_t sizeOfOffsetTableInBytes = (InputImageDimension+1)*sizeof(unsigned long);

  memcpy( this->m_OffsetTable, this->m_OutputImage->GetOffsetTable(), sizeOfOffsetTableInBytes );


  //
  // Allocate the proper amount of buffer offsets.
  //
  const unsigned int neighborhoodSize = this->m_Neighborhood.Size();

  this->m_NeighborBufferOffset.resize( neighborhoodSize );


  //
  // Visit the offset of each neighbor in Index space and convert it to linear
  // buffer offsets that can be used for pixel access
  //
  typedef typename NeighborhoodType::OffsetType    NeighborOffsetType;

  for( unsigned int i = 0; i < neighborhoodSize; i++ )
    {
    NeighborOffsetType offset = this->m_Neighborhood.GetOffset(i);

    signed int bufferOffset = 0; // must be a signed number

    for( unsigned int d = 0; d < InputImageDimension; d++ )
      {
      bufferOffset += offset[d] * this->m_OffsetTable[d];
      }
    this->m_NeighborBufferOffset[i] = bufferOffset;
    }
}

} // end namespace itk

#endif
