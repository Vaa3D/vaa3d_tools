/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkConsolidationLabelImageFilter.txx,v $
  Language:  C++
  Date:      $Date: 2009-07-08 02:32:35 $
  Version:   $Revision: 1.11 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkConsolidationLabelImageFilter_txx
#define __itkConsolidationLabelImageFilter_txx

#include "itkConsolidationLabelImageFilter.h"

namespace itk
{

/**
 * Constructor
 */
template <class TInputImage, class TOutputImage>
ConsolidationLabelImageFilter<TInputImage, TOutputImage>
::ConsolidationLabelImageFilter()
{
}

/**
 * Destructor
 */
template <class TInputImage, class TOutputImage>
ConsolidationLabelImageFilter<TInputImage, TOutputImage>
::~ConsolidationLabelImageFilter()
{
}


/**
 * Standard PrintSelf method.
 */
template <class TInputImage, class TOutputImage>
void
ConsolidationLabelImageFilter<TInputImage, TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);
}


template <class TInputImage, class TOutputImage>
void 
ConsolidationLabelImageFilter<TInputImage, TOutputImage>
::GenerateData()
{
  this->AllocateOutputImageWorkingMemory();
  this->InitializeNeighborhood();
  this->FindAllPixelsInTheBoundaryAndAddThemAsSeeds2();
  this->ComputeLabelAffinities();
  this->ConsolidateRegionsWithAffinitiesOverThreshold();
}


template <class TInputImage, class TOutputImage>
void 
ConsolidationLabelImageFilter<TInputImage, TOutputImage>
::ComputeLabelAffinities()
{
 const SeedArrayMapType & seedArrayMap = this->GetSeedArrayMap1();
 SeedArrayMapConstIterator sitr1 = seedArrayMap.begin();

 while( sitr1 != seedArrayMap.end() )
  {
  this->ComputeLabelAffinities( sitr1->first, sitr1->second );
  this->ComputeLargestAffinity( sitr1->first );
  ++sitr1;
  }
}


template <class TInputImage, class TOutputImage>
void 
ConsolidationLabelImageFilter<TInputImage, TOutputImage>
::ComputeLabelAffinities( InputImagePixelType label, const SeedArrayType * seedArray )
{
  typedef typename NumericTraits<InputImagePixelType>::PrintType PrintType;

  //
  //  If there is no histogram for this label yet, create one.
  //
  if( this->m_NeigborLabelsHistogram.find(label) == this->m_NeigborLabelsHistogram.end() )
    {
    this->m_NeigborLabelsHistogram[label] = NumberOfPixelsArrayMapType();
    }

  NumberOfPixelsArrayMapType & currentLabelHistogram = this->m_NeigborLabelsHistogram[label];

  const OutputImageType * outputImage = this->GetOutputImage();
  const OutputImagePixelType backgroundValue = this->GetBackgroundValue();

  typedef typename SeedArrayType::const_iterator   SeedIterator;

  SeedIterator seedItr = seedArray->begin();

  while( seedItr != seedArray->end() )
    {
    const OutputImagePixelType pixelValue = outputImage->GetPixel( *seedItr );

    if( pixelValue != backgroundValue )
      {
      //
      // If the current label doesn't exist, create one
      //
      if( currentLabelHistogram.find(pixelValue) == currentLabelHistogram.end() )
        {
        currentLabelHistogram[pixelValue] = NumericTraits<SizeValueType>::Zero;
        }

      currentLabelHistogram[pixelValue]++;
      }

    ++seedItr;
    }
}

template <class TInputImage, class TOutputImage>
void 
ConsolidationLabelImageFilter<TInputImage, TOutputImage>
::ComputeLargestAffinity( InputImagePixelType label )
{
  typedef typename NumericTraits<InputImagePixelType>::PrintType PrintType;
  const NumberOfPixelsArrayMapType & currentLabelHistogram = this->m_NeigborLabelsHistogram[label];

  typename NumberOfPixelsArrayMapType::const_iterator currentLabelHistogramItr = currentLabelHistogram.begin();

  SizeValueType totalCount = NumericTraits< SizeValueType >::Zero;
  SizeValueType maximumCount = currentLabelHistogramItr->second;
  LabelType     labelWithMaximumCount = currentLabelHistogramItr->first;

  while( currentLabelHistogramItr != currentLabelHistogram.end() )
    {
    const SizeValueType numberOfNeighborsAtThisLabel = currentLabelHistogramItr->second;
    totalCount += numberOfNeighborsAtThisLabel;
    if( maximumCount < numberOfNeighborsAtThisLabel )
      {
      maximumCount = numberOfNeighborsAtThisLabel;
      labelWithMaximumCount = currentLabelHistogramItr->first;
      }
    ++currentLabelHistogramItr;
    }

  const double affinityValue = static_cast< double >( maximumCount ) / static_cast< double >( totalCount );

  std::cout << "Labels " << static_cast< typename NumericTraits< InputImagePixelType >::PrintType >( label );
  std::cout << " : " << labelWithMaximumCount << " Affinity = " << affinityValue << std::endl;

  this->m_AffinityValue[label] = affinityValue;
  this->m_LabelWithHigestAffinity[label] = labelWithMaximumCount;

  if( affinityValue > this->m_AffinityThreshold )
    {
    this->m_LabelChangeTable[label] = labelWithMaximumCount;
    }
}


template <class TInputImage, class TOutputImage>
void 
ConsolidationLabelImageFilter<TInputImage, TOutputImage>
::ConsolidateRegionsWithAffinitiesOverThreshold()
{
  OutputImageType * outputImage = this->GetOutputImage();

  typedef ImageRegionIterator< OutputImageType >  OutputImageIterator;
  OutputImageIterator otr( outputImage, outputImage->GetBufferedRegion() );

  otr.GoToBegin();

  typedef typename AffinityLabelMapType::const_iterator  AffinityLabelMapIterator;
  AffinityLabelMapIterator labelTableEnd = this->m_LabelChangeTable.end();

  while( !otr.IsAtEnd() )
    {
    const OutputImagePixelType label = otr.Get();
    AffinityLabelMapIterator labelTableItr = this->m_LabelChangeTable.find( label );

    if( labelTableItr != labelTableEnd )
      {
      otr.Set( labelTableItr->second );
      }

    ++otr;
    }
}


} // end namespace itk

#endif
