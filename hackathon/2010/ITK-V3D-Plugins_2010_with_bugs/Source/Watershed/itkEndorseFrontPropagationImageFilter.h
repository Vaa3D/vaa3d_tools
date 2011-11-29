/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkVotingHoleFillingFloodingImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2006-03-24 16:03:17 $
  Version:   $Revision: 1.17 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkEndorseFrontPropagationImageFilter_h
#define __itkEndorseFrontPropagationImageFilter_h

#include "itkImage.h"
#include "itkVotingBinaryImageFilter.h"

#include <vector>

namespace itk{

/** /class EndorseFrontPropagationImageFilter 
 *
 * \brief Perform front-propagation under a quorum sensing (voting) algorithm
 * for filling holes in a binary mask.
 * 
 * This is an alternative implementation of the
 * VotingBinaryIterativeHoleFillingImageFilter.
 *
 * \ingroup RegionGrowingSegmentation 
 */
template <class TInputImage1, class TInputImage2, class TOutputImage>
class ITK_EXPORT EndorseFrontPropagationImageFilter:
    public VotingBinaryImageFilter<TInputImage1,TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef EndorseFrontPropagationImageFilter              Self;
  typedef VotingBinaryImageFilter<TInputImage1,TOutputImage>    Superclass;
  typedef SmartPointer<Self>                                   Pointer;
  typedef SmartPointer<const Self>                             ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods).  */
  itkTypeMacro(EndorseFrontPropagationImageFilter, VotingBinaryImageFilter);

  typedef typename Superclass::InputImageType             InputImageType;
  typedef typename InputImageType::Pointer                InputImagePointer;
  typedef typename InputImageType::ConstPointer           InputImageConstPointer;
  typedef typename InputImageType::RegionType             InputImageRegionType; 
  typedef typename InputImageType::PixelType              InputImagePixelType; 
  typedef typename InputImageType::IndexType              IndexType;
  typedef typename InputImageType::OffsetValueType        OffsetValueType;
  
  typedef TInputImage2                                    CreditInputImageType;
  typedef typename CreditInputImageType::PixelType        CreditInputImagePixelType; 

  typedef typename Superclass::OutputImageType            OutputImageType;
  typedef typename OutputImageType::Pointer               OutputImagePointer;
  typedef typename OutputImageType::RegionType            OutputImageRegionType; 
  typedef typename OutputImageType::PixelType             OutputImagePixelType; 
  
  typedef typename Superclass::InputSizeType              InputSizeType;
  
  /** Image dimension constants */
  itkStaticConstMacro(InputImageDimension,  unsigned int, TInputImage1::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int, TOutputImage::ImageDimension);

  /** Majority threshold. It is the number of pixels over 50% that will decide
   * whether an OFF pixel will become ON or not. For example, if the
   * neighborhood of a pixel has 124 pixels (excluding itself), the 50% will be
   * 62, and if you set upd a Majority threshold of 5, that means that the
   * filter will require 67 or more neighbor pixels to be ON in order to switch
   * the current OFF pixel to ON. The default value is 1. */ 
  itkGetConstReferenceMacro( MajorityThreshold, unsigned int );
  itkSetMacro( MajorityThreshold, unsigned int );

  /** Endorsement Threshold. At every candidate pixel an endorsement from its
   * neighbors is computed based on their difference in intensity values. The
   * endorsements from all neighbors are added, and when the sum is larger that
   * this EndorsementThreshold then the pixel is included in the propagating
   * front. */
  itkGetConstReferenceMacro( EndorsementThreshold, double );
  itkSetMacro( EndorsementThreshold, double );

  /** Set/Get the maximum number of iterations that will be applied to the
   * propagating front */
  itkSetMacro( MaximumNumberOfIterations, unsigned int );
  itkGetMacro( MaximumNumberOfIterations, unsigned int );

  /** Returned the number of iterations used so far. */
  itkGetMacro( CurrentIterationNumber, unsigned int );

  /** Returned the number of pixels changed in total. */
  itkGetMacro( TotalNumberOfPixelsChanged, unsigned int );

  /** Connect the image that provides similarities between pixels. This is
   * typically the image to be segmented. */
  void SetCreditInput( const TInputImage2 * image2);

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
  EndorseFrontPropagationImageFilter();
  ~EndorseFrontPropagationImageFilter();

  void GenerateData();
  
  void PrintSelf ( std::ostream& os, Indent indent ) const;

private:
  EndorseFrontPropagationImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented


  void AllocateOutputImageWorkingMemory();

  void InitializeNeighborhood();

  void FindAllPixelsInTheBoundaryAndAddThemAsSeeds();

  void IterateFrontPropagations();

  void VisitAllSeedsAndTransitionTheirState();

  void PasteNewSeedValuesToOutputImage();

  void SwapSeedArrays();

  void ClearSecondSeedArray();

  bool TestForEndorsementAtCurrentPixel() const;
 
  void PutCurrentPixelNeighborsIntoSeedArray();

  void ComputeArrayOfNeighborhoodBufferOffsets();

  void ComputeBirthThreshold();

  itkSetMacro( CurrentPixelIndex, IndexType );
  itkGetConstReferenceMacro( CurrentPixelIndex, IndexType );

  unsigned int                      m_MajorityThreshold;

  typedef std::vector<IndexType>    SeedArrayType;

  SeedArrayType *                   m_SeedArray1;
  SeedArrayType *                   m_SeedArray2;
  
  typedef std::vector<OutputImagePixelType>   SeedNewValuesArrayType;

  SeedNewValuesArrayType            m_SeedsNewValues;

  unsigned int                      m_CurrentIterationNumber;
  unsigned int                      m_MaximumNumberOfIterations;
  unsigned int                      m_NumberOfPixelsChangedInLastIteration;
  unsigned int                      m_TotalNumberOfPixelsChanged;
  
  IndexType                         m_CurrentPixelIndex;

  //
  // Variables used for addressing the Neighbors.
  // This could be factorized into a helper class.
  //
  OffsetValueType                   m_OffsetTable[ InputImageDimension + 1 ]; 
  
  typedef std::vector< OffsetValueType >   NeighborOffsetArrayType;

  NeighborOffsetArrayType           m_NeighborBufferOffset;


  //
  // Helper cache variables 
  //
  const InputImageType *            m_MaskInputImage;
  const CreditInputImageType *      m_CreditInputImage;
  OutputImageType *                 m_OutputImage;

  typedef itk::Image< unsigned char, InputImageDimension >  SeedMaskImageType;
  typedef typename SeedMaskImageType::Pointer               SeedMaskImagePointer;

  SeedMaskImagePointer              m_SeedsMask;

  typedef itk::Neighborhood< InputImagePixelType, InputImageDimension >  NeighborhoodType;

  NeighborhoodType                  m_Neighborhood;

  double                            m_EndorsementThreshold;

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkEndorseFrontPropagationImageFilter.txx"
#endif

#endif
