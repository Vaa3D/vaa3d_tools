/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkConsolidationLabelImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2009-07-02 05:15:36 $
  Version:   $Revision: 1.7 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkConsolidationLabelImageFilter_h
#define __itkConsolidationLabelImageFilter_h

#include "itkImage.h"
#include "itkFrontPropagationLabelImageFilter.h"

#include <vector>

namespace itk
{

/** \class ConsolidationLabelImageFilter 
 *
 * \brief Consolidate small label regions into larger ones based on their
 * common boundaries.
 * 
 * The algorithm visits all pixels in the boundary of a label, counts how many
 * of them belong to another label and according to that number decides whether
 * this labels should be consolidated with one of its neighbors.
 *
 * \ingroup RegionGrowingSegmentation 
 */
template <class TInputImage, class TOutputImage>
class ITK_EXPORT ConsolidationLabelImageFilter:
    public FrontPropagationLabelImageFilter<TInputImage,TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef ConsolidationLabelImageFilter                                 Self;
  typedef FrontPropagationLabelImageFilter<TInputImage,TOutputImage>    Superclass;
  typedef SmartPointer<Self>                                            Pointer;
  typedef SmartPointer<const Self>                                      ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods).  */
  itkTypeMacro(ConsolidationLabelImageFilter, FrontPropagationLabelImageFilter);

  typedef typename Superclass::InputImageType             InputImageType;
  typedef typename InputImageType::Pointer                InputImagePointer;
  typedef typename InputImageType::ConstPointer           InputImageConstPointer;
  typedef typename InputImageType::RegionType             InputImageRegionType; 
  typedef typename InputImageType::PixelType              InputImagePixelType; 
  typedef typename InputImageType::IndexType              IndexType;
  typedef typename InputImageType::SizeType               InputSizeType;
  typedef typename InputImageType::OffsetValueType        OffsetValueType;
  typedef typename InputImageType::SizeValueType          SizeValueType;
  
  typedef typename Superclass::OutputImageType            OutputImageType;
  typedef typename OutputImageType::Pointer               OutputImagePointer;
  typedef typename OutputImageType::RegionType            OutputImageRegionType; 
  typedef typename OutputImageType::PixelType             OutputImagePixelType; 
  

  /** Image dimension constants */
  itkStaticConstMacro(InputImageDimension,  unsigned int, TInputImage::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int, TOutputImage::ImageDimension);

  /** Set/Get Affinity Threshold value. This is the value that will define
   * whether a region should consolidate with another one. If region A affinity
   * for region B is larger than AffinityThreshold then region A will be
   * consolidated with region B.
   */
  itkSetMacro( AffinityThreshold, double );
  itkGetMacro( AffinityThreshold, double );

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro(OutputEqualityComparableCheck, (Concept::EqualityComparable<OutputImagePixelType>));
  itkConceptMacro(InputEqualityComparableCheck, (Concept::EqualityComparable<InputImagePixelType>));
  itkConceptMacro(InputConvertibleToOutputCheck, (Concept::Convertible<InputImagePixelType, OutputImagePixelType>));
  itkConceptMacro(SameDimensionCheck, (Concept::SameDimension<InputImageDimension, OutputImageDimension>));
  itkConceptMacro(IntConvertibleToInputCheck, (Concept::Convertible<int, InputImagePixelType>));
  itkConceptMacro(OutputOStreamWritableCheck, (Concept::OStreamWritable<OutputImagePixelType>));
  /** End concept checking */
#endif

protected:
  ConsolidationLabelImageFilter();
  ~ConsolidationLabelImageFilter();

  virtual void GenerateData();
  
  virtual void PrintSelf ( std::ostream& os, Indent indent ) const;

private:
  ConsolidationLabelImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  typedef typename Superclass::LabelType                    LabelType; 
  typedef typename Superclass::SeedArrayType                SeedArrayType;
  typedef typename Superclass::SeedArrayMapType             SeedArrayMapType;
  typedef typename Superclass::SeedArrayMapIterator         SeedArrayMapIterator;
  typedef typename Superclass::SeedArrayMapConstIterator    SeedArrayMapConstIterator; 
  typedef typename Superclass::NumberOfPixelsArrayMapType   NumberOfPixelsArrayMapType; 

  virtual void ComputeLabelAffinities();
  virtual void ComputeLabelAffinities( InputImagePixelType label, const SeedArrayType * seedArray );
  virtual void ComputeLargestAffinity( InputImagePixelType label );
  virtual void ConsolidateRegionsWithAffinitiesOverThreshold();

  typedef std::map<LabelType, NumberOfPixelsArrayMapType >  NeighborLabelsDistributionType;
  typedef std::map<LabelType, double >                      AffinityMapType;
  typedef std::map<LabelType, SizeValueType >               AffinityLabelMapType;

  NeighborLabelsDistributionType      m_NeigborLabelsHistogram;
  AffinityMapType                     m_AffinityValue;
  AffinityLabelMapType                m_LabelWithHigestAffinity;
  AffinityLabelMapType                m_LabelChangeTable;
  double                              m_AffinityThreshold;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkConsolidationLabelImageFilter.txx"
#endif

#endif
