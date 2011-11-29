/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkHoleFillingFrontPropagationLabelImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2009-07-02 05:15:36 $
  Version:   $Revision: 1.7 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkHoleFillingFrontPropagationLabelImageFilter_h
#define __itkHoleFillingFrontPropagationLabelImageFilter_h

#include "itkImage.h"
#include "itkFrontPropagationLabelImageFilter.h"

namespace itk
{

/** \class HoleFillingFrontPropagationLabelImageFilter 
 *
 * \brief Perform front-propagation under a voting algorithm to close
 * holes inside labels.
 * 
 * The algorithm visits background pixels and promote them to
 * foreground when their number of foreground neighbors is larger than
 * the Birth Threshold parameter. This is done on a Label by label
 * basis.
 *
 * \ingroup RegionGrowingSegmentation 
 */
template <class TInputImage, class TOutputImage>
class ITK_EXPORT HoleFillingFrontPropagationLabelImageFilter:
    public FrontPropagationLabelImageFilter<TInputImage,TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef HoleFillingFrontPropagationLabelImageFilter                 Self;
  typedef FrontPropagationLabelImageFilter<TInputImage,TOutputImage>  Superclass;
  typedef SmartPointer<Self>                                          Pointer;
  typedef SmartPointer<const Self>                                    ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods).  */
  itkTypeMacro(HoleFillingFrontPropagationLabelImageFilter, FrontPropagationLabelImageFilter);

  typedef typename Superclass::InputImageType             InputImageType;
  typedef typename InputImageType::Pointer                InputImagePointer;
  typedef typename InputImageType::ConstPointer           InputImageConstPointer;
  typedef typename InputImageType::RegionType             InputImageRegionType; 
  typedef typename InputImageType::PixelType              InputImagePixelType; 
  typedef typename InputImageType::IndexType              IndexType;
  typedef typename InputImageType::OffsetValueType        OffsetValueType;
  
  typedef typename Superclass::OutputImageType            OutputImageType;
  typedef typename OutputImageType::Pointer               OutputImagePointer;
  typedef typename OutputImageType::RegionType            OutputImageRegionType; 
  typedef typename OutputImageType::PixelType             OutputImagePixelType; 
  
  typedef typename Superclass::InputSizeType              InputSizeType;
  typedef typename Superclass::NeighborOffsetArrayType    NeighborOffsetArrayType;



  /** Majority threshold. It is the number of pixels over 50% that will decide
   * whether an OFF pixel will become ON or not. For example, if the
   * neighborhood of a pixel has 124 pixels (excluding itself), the 50% will be
   * 62, and if you set upd a Majority threshold of 5, that means that the
   * filter will require 67 or more neighbor pixels to be ON in order to switch
   * the current OFF pixel to ON. The default value is 1. */ 
  itkGetConstReferenceMacro( MajorityThreshold, unsigned int );
  itkSetMacro( MajorityThreshold, unsigned int );

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro(OutputEqualityComparableCheck, (Concept::EqualityComparable<OutputImagePixelType>));
  itkConceptMacro(InputEqualityComparableCheck, (Concept::EqualityComparable<InputImagePixelType>));
  itkConceptMacro(InputConvertibleToOutputCheck, (Concept::Convertible<InputImagePixelType, OutputImagePixelType>));
  itkConceptMacro(IntConvertibleToInputCheck, (Concept::Convertible<int, InputImagePixelType>));
  itkConceptMacro(OutputOStreamWritableCheck, (Concept::OStreamWritable<OutputImagePixelType>));
  /** End concept checking */
#endif

protected:
  HoleFillingFrontPropagationLabelImageFilter();
  ~HoleFillingFrontPropagationLabelImageFilter();

  virtual void GenerateData();
  virtual void ComputeBirthThreshold();

  void PrintSelf ( std::ostream& os, Indent indent ) const;

private:
  HoleFillingFrontPropagationLabelImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  virtual bool TestForAcceptingCurrentPixel() const;

  unsigned int              m_MajorityThreshold; 
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkHoleFillingFrontPropagationLabelImageFilter.txx"
#endif

#endif
