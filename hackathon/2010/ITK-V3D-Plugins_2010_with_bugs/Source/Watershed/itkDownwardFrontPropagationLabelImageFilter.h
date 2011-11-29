/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkDownwardFrontPropagationLabelImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2009-07-02 05:15:36 $
  Version:   $Revision: 1.7 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkDownwardFrontPropagationLabelImageFilter_h
#define __itkDownwardFrontPropagationLabelImageFilter_h

#include "itkImage.h"
#include "itkFrontPropagationLabelImageFilter.h"

namespace itk
{

/** \class DownwardFrontPropagationLabelImageFilter 
 *
 * \brief Perform front-propagation towards feature image pixels with descending values.
 * 
 * The algorithm identifies all pixels in the boundary of labeled areas and propagate
 * the fronts of the labels to pixels that in the feature image has lower values than 
 * pixels that are already in the label.
 *
 * \ingroup RegionGrowingSegmentation 
 */
template <class TInputImage, class TInputFeatureImage, class TOutputImage>
class ITK_EXPORT DownwardFrontPropagationLabelImageFilter:
    public FrontPropagationLabelImageFilter<TInputImage,TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef DownwardFrontPropagationLabelImageFilter                    Self;
  typedef FrontPropagationLabelImageFilter<TInputImage,TOutputImage>  Superclass;
  typedef SmartPointer<Self>                                          Pointer;
  typedef SmartPointer<const Self>                                    ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods).  */
  itkTypeMacro(DownwardFrontPropagationLabelImageFilter, FrontPropagationLabelImageFilter);

  typedef typename Superclass::InputImageType             InputImageType;
  typedef typename InputImageType::OffsetValueType        OffsetValueType;
  
  typedef typename Superclass::OutputImageType            OutputImageType;
  typedef typename OutputImageType::PixelType             OutputImagePixelType; 
  
  typedef typename Superclass::InputSizeType              InputSizeType;
  typedef typename Superclass::NeighborOffsetArrayType    NeighborOffsetArrayType;

  typedef TInputFeatureImage                              FeatureImageType;
  typedef typename FeatureImageType::PixelType            InputFeaturePixelType;
  
  /** Set the image to be segmented */
  void SetFeatureImage( const FeatureImageType * imageToSegment );

  /** Set the lower threshold to reach in the feature image */
  itkSetMacro( LowerThreshold, InputFeaturePixelType );
  itkGetMacro( LowerThreshold, InputFeaturePixelType );

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro(FeatureLessThanComparableCheck, (Concept::LessThanComparable<InputFeaturePixelType>));
  /** End concept checking */
#endif

protected:
  DownwardFrontPropagationLabelImageFilter();
  ~DownwardFrontPropagationLabelImageFilter();

  void PrintSelf ( std::ostream& os, Indent indent ) const;

  void AllocateOutputImageWorkingMemory();

private:
  DownwardFrontPropagationLabelImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  bool TestForAcceptingCurrentPixel() const;

  //
  // Helper cache variables 
  //
  const FeatureImageType *    m_FeatureImage;

  InputFeaturePixelType       m_LowerThreshold;

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDownwardFrontPropagationLabelImageFilter.txx"
#endif

#endif
