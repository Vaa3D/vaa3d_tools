/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkVotingHoleFillingFloodingImageFilter.txx,v $
  Language:  C++
  Date:      $Date: 2006-06-28 17:06:15 $
  Version:   $Revision: 1.26 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkVotingHoleFillingFloodingImageFilter_txx_
#define __itkVotingHoleFillingFloodingImageFilter_txx_

#include "itkVotingBinaryHoleFillFloodingImageFilter.h"
#include "itkConstNeighborhoodIterator.h"
#include "itkImageRegionIterator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkOffset.h"

namespace itk
{

/**
 * Constructor
 */
template <class TInputImage, class TOutputImage>
VotingBinaryHoleFillFloodingImageFilter<TInputImage, TOutputImage>
::VotingBinaryHoleFillFloodingImageFilter()
{
  this->m_MaximumNumberOfIterations = 10;
  this->m_CurrentIterationNumber = 0;

  this->m_NumberOfPixelsChangedInLastIteration = 0;
  this->m_TotalNumberOfPixelsChanged = 0;

  this->m_SeedArray1 = new SeedArrayType;
  this->m_SeedArray2 = new SeedArrayType;

  this->m_OutputImage = NULL;

  this->m_MajorityThreshold = 1;
}

/**
 * Destructor
 */
template <class TInputImage, class TOutputImage>
VotingBinaryHoleFillFloodingImageFilter<TInputImage, TOutputImage>
::~VotingBinaryHoleFillFloodingImageFilter()
{
  delete this->m_SeedArray1;
  delete this->m_SeedArray2;
}


/**
 * Standard PrintSelf method.
 */
template <class TInputImage, class TOutputImage>
void
VotingBinaryHoleFillFloodingImageFilter<TInputImage, TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);
}


template <class TInputImage, class TOutputImage>
void 
VotingBinaryHoleFillFloodingImageFilter<TInputImage,TOutputImage>
::GenerateData()
{
  this->AllocateOutputImageWorkingMemory();
  this->InitializeNeighborhood();
  this->ComputeBirthThreshold();
  this->ComputeArrayOfNeighborhoodBufferOffsets();
  this->FindAllPixelsInTheBoundaryAndAddThemAsSeeds();
  this->IterateFrontPropagations();
}


template <class TInputImage, class TOutputImage>
void 
VotingBinaryHoleFillFloodingImageFilter<TInputImage,TOutputImage>
::IterateFrontPropagations()
{
  this->m_CurrentIterationNumber = 0;
  this->m_TotalNumberOfPixelsChanged = 0;
  this->m_NumberOfPixelsChangedInLastIteration = 0;

  while( this->m_CurrentIterationNumber < this->m_MaximumNumberOfIterations ) 
    {
    this->VisitAllSeedsAndTransitionTheirState();
    this->m_CurrentIterationNumber++;
    if( this->m_NumberOfPixelsChangedInLastIteration ==  0 )
      {
      break;
      }
    }
}


template <class TInputImage, class TOutputImage>
void 
VotingBinaryHoleFillFloodingImageFilter<TInputImage,TOutputImage>
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
VotingBinaryHoleFillFloodingImageFilter<TInputImage,TOutputImage>
::InitializeNeighborhood()
{
  this->m_Neighborhood.SetRadius( this->GetRadius() );
}


template <class TInputImage, class TOutputImage>
void 
VotingBinaryHoleFillFloodingImageFilter<TInputImage,TOutputImage>
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

  bit = ConstNeighborhoodIterator<InputImageType>( radius, inputImage, this->m_InternalRegion );
  itr  = ImageRegionIterator<OutputImageType>(    this->m_OutputImage, this->m_InternalRegion );
  mtr  = ImageRegionIterator<SeedMaskImageType>(  this->m_SeedsMask,   this->m_InternalRegion );

  bit.GoToBegin();
  itr.GoToBegin();
  mtr.GoToBegin();
  
  unsigned int neighborhoodSize = bit.Size();

  const InputImagePixelType foregroundValue = this->GetForegroundValue();
  const InputImagePixelType backgroundValue = this->GetBackgroundValue();
  
  this->m_SeedArray1->clear();
  this->m_SeedsNewValues.clear();

  while ( ! bit.IsAtEnd() )
    {
    if( bit.GetCenterPixel() == foregroundValue )
      {
      itr.Set( foregroundValue );
      mtr.Set( 255 );
      }
    else
      {
      itr.Set( backgroundValue );
      mtr.Set( 0 );
      
      // Search for foreground pixels in the neighborhood
      for (unsigned int i = 0; i < neighborhoodSize; ++i)
        {
        InputImagePixelType value = bit.GetPixel(i);
        if( value == foregroundValue )
          {
          this->m_SeedArray1->push_back( bit.GetIndex() );
          break;
          }
        }
      }   
    ++bit;
    ++itr;
    ++mtr;
    }
  this->m_SeedsNewValues.reserve( this->m_SeedArray1->size() ); 
}


template <class TInputImage, class TOutputImage>
void 
VotingBinaryHoleFillFloodingImageFilter<TInputImage,TOutputImage>
::VisitAllSeedsAndTransitionTheirState()
{
  typedef typename SeedArrayType::const_iterator   SeedIterator;

  SeedIterator seedItr = this->m_SeedArray1->begin();

  this->m_NumberOfPixelsChangedInLastIteration = 0;

  // Clear the array of new values
  this->m_SeedsNewValues.clear();

  while( seedItr != this->m_SeedArray1->end() )
    {
    this->SetCurrentPixelIndex( *seedItr );

    if( this->TestForQuorumAtCurrentPixel() )
      {
      this->m_SeedsNewValues.push_back( this->GetForegroundValue() );
      this->PutCurrentPixelNeighborsIntoSeedArray();
      this->m_NumberOfPixelsChangedInLastIteration++;
      }
    else
      {
      this->m_SeedsNewValues.push_back( this->GetBackgroundValue() );
      // Keep the seed to try again in the next iteration.
      this->m_SeedArray2->push_back( this->GetCurrentPixelIndex() );
      }

    ++seedItr;
    }

  this->PasteNewSeedValuesToOutputImage();
   
  this->m_TotalNumberOfPixelsChanged += this->m_NumberOfPixelsChangedInLastIteration;

  // Now that the values have been copied to the output image, we can empty the
  // array in preparation for the next iteration
  this->m_SeedsNewValues.clear();

  this->SwapSeedArrays();
  this->ClearSecondSeedArray();
}


template <class TInputImage, class TOutputImage>
void 
VotingBinaryHoleFillFloodingImageFilter<TInputImage,TOutputImage>
::PasteNewSeedValuesToOutputImage()
{
  //
  //  Paste new values into the output image
  //
  typedef typename SeedArrayType::const_iterator   SeedIterator;

  SeedIterator seedItr = this->m_SeedArray1->begin();

  typedef typename SeedNewValuesArrayType::const_iterator   SeedsNewValuesIterator;

  SeedsNewValuesIterator newValueItr = this->m_SeedsNewValues.begin();

  while (seedItr != this->m_SeedArray1->end() )
    {
    this->m_OutputImage->SetPixel( *seedItr, *newValueItr );
    ++seedItr;
    ++newValueItr;
    }
}

template <class TInputImage, class TOutputImage>
void 
VotingBinaryHoleFillFloodingImageFilter<TInputImage,TOutputImage>
::SwapSeedArrays()
{
  SeedArrayType * temporary = this->m_SeedArray1;
  this->m_SeedArray1 = this->m_SeedArray2;
  this->m_SeedArray2 = temporary;
}


template <class TInputImage, class TOutputImage>
void 
VotingBinaryHoleFillFloodingImageFilter<TInputImage,TOutputImage>
::ClearSecondSeedArray()
{
  delete this->m_SeedArray2;
  this->m_SeedArray2 = new SeedArrayType;
}


template <class TInputImage, class TOutputImage>
bool 
VotingBinaryHoleFillFloodingImageFilter<TInputImage,TOutputImage>
::TestForQuorumAtCurrentPixel() const
{
  //
  // Find the location of the current pixel in the image memory buffer
  //
  const OffsetValueType offset = this->m_OutputImage->ComputeOffset( this->GetCurrentPixelIndex() );

  const InputImagePixelType * buffer = this->m_OutputImage->GetBufferPointer();

  const InputImagePixelType * currentPixelPointer = buffer + offset;

  unsigned int numberOfNeighborsAtForegroundValue = 0;

  //
  // From that buffer position, visit all other neighbor pixels 
  // and check if they are set to the foreground value.
  //
  typedef typename NeighborOffsetArrayType::const_iterator   NeigborOffsetIterator;

  NeigborOffsetIterator neighborItr = this->m_NeighborBufferOffset.begin();

  const InputImagePixelType foregroundValue = this->GetForegroundValue();

  while( neighborItr != this->m_NeighborBufferOffset.end() )
    {
    const InputImagePixelType * neighborPixelPointer = currentPixelPointer + *neighborItr;

    if( *neighborPixelPointer == foregroundValue )
      {
      numberOfNeighborsAtForegroundValue++;
      }

    ++neighborItr;
    }

  bool quorum = (numberOfNeighborsAtForegroundValue > this->GetBirthThreshold() );

  return quorum;
}


template <class TInputImage, class TOutputImage>
void 
VotingBinaryHoleFillFloodingImageFilter<TInputImage,TOutputImage>
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
      NeighborOffsetType offset = this->m_Neighborhood.GetOffset(i);
      IndexType neighborIndex = this->GetCurrentPixelIndex() + offset;

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
void 
VotingBinaryHoleFillFloodingImageFilter<TInputImage,TOutputImage>
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

template <class TInputImage, class TOutputImage>
void 
VotingBinaryHoleFillFloodingImageFilter<TInputImage,TOutputImage>
::ComputeBirthThreshold()
{
  const unsigned int neighborhoodSize = this->m_Neighborhood.Size();

  // Take the number of neighbors, discount the central pixel, and take half of them.
  unsigned int threshold = static_cast<unsigned int>( (neighborhoodSize - 1 ) / 2.0 ); 

  // add the majority threshold.
  threshold += this->GetMajorityThreshold();

  // Set that number as the Birth Threshold
  this->SetBirthThreshold( threshold );
}

} // end namespace itk

#endif
