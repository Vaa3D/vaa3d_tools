/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkFrontPropagationLabelImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2009-07-02 05:15:36 $
  Version:   $Revision: 1.7 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkFrontPropagationLabelImageFilter_h
#define __itkFrontPropagationLabelImageFilter_h

#include "itkImage.h"
#include "itkInPlaceImageFilter.h"

#include <vector>

namespace itk
{

/** \class FrontPropagationLabelImageFilter 
 *
 * \brief Perform front-propagation of labels based on user-provided rules.
 * 
 * The algorithm visits all pixels, identifies label boundaries and propagate
 * them according to a user-provided rule.
 *
 * \ingroup RegionGrowingSegmentation 
 */
template <class TInputImage, class TOutputImage>
class ITK_EXPORT FrontPropagationLabelImageFilter:
    public InPlaceImageFilter<TInputImage,TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef FrontPropagationLabelImageFilter                Self;
  typedef InPlaceImageFilter<TInputImage,TOutputImage>    Superclass;
  typedef SmartPointer<Self>                              Pointer;
  typedef SmartPointer<const Self>                        ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods).  */
  itkTypeMacro(FrontPropagationLabelImageFilter, InPlaceImageFilter);

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

  /** Set/Get the radius of the neighborhood used to compute the median. */
  itkSetMacro(Radius, InputSizeType);
  itkGetConstReferenceMacro(Radius, InputSizeType);
 
  /** Set/Get the maximum number of iterations that will be applied to the
   * propagating front */
  itkSetMacro( MaximumNumberOfIterations, unsigned int );
  itkGetMacro( MaximumNumberOfIterations, unsigned int );

  /** Returned the number of iterations used so far. */
  itkGetMacro( CurrentIterationNumber, unsigned int );

  /** Returned the number of pixels changed in total. */
  itkGetMacro( TotalNumberOfPixelsChanged, unsigned int );

  /** Set/Get the value associated with the Background. */
  itkSetMacro(BackgroundValue, InputImagePixelType);
  itkGetConstReferenceMacro(BackgroundValue, InputImagePixelType);

  /** Set/Get the value associated with the Foreground. */
  itkSetMacro(ForegroundValue, InputImagePixelType);
  itkGetConstReferenceMacro(ForegroundValue, InputImagePixelType);

  /** Majority threshold. It is the number of pixels over 50% that will decide
   * whether an OFF pixel will become ON or not. For example, if the
   * neighborhood of a pixel has 124 pixels (excluding itself), the 50% will be
   * 62, and if you set upd a Majority threshold of 5, that means that the
   * filter will require 67 or more neighbor pixels to be ON in order to switch
   * the current OFF pixel to ON. The default value is 1. */ 
  itkGetConstReferenceMacro( MajorityThreshold, unsigned int );
  itkSetMacro( MajorityThreshold, unsigned int );

  /** Birth threshold. Pixels that are OFF will turn ON when the number of
   * neighbors ON is larger than the value defined in this threshold. */
  itkGetConstReferenceMacro(BirthThreshold, unsigned int);
  itkSetMacro(BirthThreshold, unsigned int);

  /** Survival threshold. Pixels that are ON will turn OFF when the number of
   * neighbors ON is smaller than the value defined in this survival threshold. */
  itkGetConstReferenceMacro(SurvivalThreshold, unsigned int);
  itkSetMacro(SurvivalThreshold, unsigned int);

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
  FrontPropagationLabelImageFilter();
  ~FrontPropagationLabelImageFilter();

  void GenerateData();
  
  void PrintSelf ( std::ostream& os, Indent indent ) const;

  virtual void AllocateOutputImageWorkingMemory();

  void InitializeNeighborhood();

  void ComputeArrayOfNeighborhoodBufferOffsets();

  void FindAllPixelsInTheBoundaryAndAddThemAsSeeds();
  void FindAllPixelsInTheBoundaryAndAddThemAsSeeds2();

  void IterateFrontPropagations();

  unsigned int GetNeighborhoodSize() const;

  typedef std::vector< OffsetValueType >   NeighborOffsetArrayType;

  OutputImageType * GetOutputImage();
  const OutputImageType * GetOutputImage() const;

  itkGetConstReferenceMacro( CurrentPixelIndex, IndexType );

  const NeighborOffsetArrayType & GetNeighborBufferOffset() const;

  typedef unsigned int              LabelType;
  typedef std::vector<IndexType>    SeedArrayType;

  typedef std::map<LabelType,SeedArrayType *>        SeedArrayMapType;
  typedef typename SeedArrayMapType::iterator        SeedArrayMapIterator;
  typedef typename SeedArrayMapType::const_iterator  SeedArrayMapConstIterator;
  typedef std::map<LabelType, SizeValueType >        NumberOfPixelsArrayMapType;


  itkGetConstReferenceMacro( SeedArrayMap1, SeedArrayMapType );

private:
  FrontPropagationLabelImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  void VisitAllSeedsAndTransitionTheirState();
  void VisitAllSeedsAndTransitionTheirState(LabelType label);

  void PasteNewSeedValuesToOutputImage( LabelType label );

  void SwapSeedArrays();

  void ClearSecondSeedArray();

  virtual bool TestForAcceptingCurrentPixel() const;

  void PutCurrentPixelNeighborsIntoSeedArray();

  itkSetMacro( CurrentPixelIndex, IndexType );

  SeedArrayType *                   m_SeedArray1;
  SeedArrayType *                   m_SeedArray2;

  SeedArrayMapType                  m_SeedArrayMap1;
  SeedArrayMapType                  m_SeedArrayMap2;

  InputImageRegionType              m_InternalRegion;
  
  typedef std::vector<OutputImagePixelType>   SeedNewValuesArrayType;

  SeedNewValuesArrayType            m_SeedsNewValues;

  typedef std::map<LabelType,SeedNewValuesArrayType >  SeedNewValuesArrayMapType;

  SeedNewValuesArrayMapType         m_SeedsNewValuesMap;

  unsigned int                      m_CurrentIterationNumber;
  unsigned int                      m_MaximumNumberOfIterations;

  SizeValueType                     m_TotalNumberOfPixelsChanged;
  SizeValueType                     m_TotalNumberOfPixelsChangedInLastIteration;
  
  NumberOfPixelsArrayMapType        m_NumberOfPixels;
  NumberOfPixelsArrayMapType        m_NumberOfPixelsChangedInLastIteration;

  IndexType                         m_CurrentPixelIndex;

  //
  // Variables used for addressing the Neighbors.
  // This could be factorized into a helper class.
  //
  OffsetValueType                   m_OffsetTable[ InputImageDimension + 1 ]; 
  
  NeighborOffsetArrayType           m_NeighborBufferOffset;


  //
  // Helper cache variables 
  //
  const InputImageType *            m_InputImage;
  OutputImageType *                 m_OutputImage;

  typedef itk::Image< unsigned char, InputImageDimension >  SeedMaskImageType;
  typedef typename SeedMaskImageType::Pointer               SeedMaskImagePointer;

  SeedMaskImagePointer              m_SeedsMask;

  typedef itk::Neighborhood< InputImagePixelType, InputImageDimension >  NeighborhoodType;

  InputSizeType             m_Radius;

  InputImagePixelType       m_ForegroundValue;

  InputImagePixelType       m_BackgroundValue;

  NeighborhoodType          m_Neighborhood;

  unsigned int              m_MajorityThreshold; 
  unsigned int              m_BirthThreshold;
  unsigned int              m_SurvivalThreshold;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFrontPropagationLabelImageFilter.txx"
#endif

#endif
