/*=========================================================================
Copyright 2009 Rensselaer Polytechnic Institute
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. 
=========================================================================*/

/*=========================================================================
  Program:   Open Snake Tracing System
  Autohr:    Yu Wang
  Email: wangy15@rpi.edu
  Language:  C++
  Date:      $Date:  $
  Version:   $Revision: 0.00 $
=========================================================================*/

#ifndef IMAGEOPERATION_H
#define IMAGEOPERATION_H

#include "itkBinaryThresholdImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
//#include "itkImageSeriesReader.h"
#include "itkSubtractImageFilter.h"
#include "itkNeighborhoodIterator.h"
#include "itkConstNeighborhoodIterator.h"
#include "itkImageIterator.h"
#include "itkImageIteratorWithIndex.h"

#include "itkListSample.h"
#include "itkKdTree.h"
#include "itkKdTreeGenerator.h"

#include "Filters/itkBinaryThinningImageFilter3D.h"
#include "itkMedianImageFilter.h"

#include "itkImageSliceConstIteratorWithIndex.h"
#include "itkImageSliceIteratorWithIndex.h"
#include "itkImageLinearIteratorWithIndex.h"

#include "itkVotingBinaryHoleFillingImageFilter.h"

#include "itkConnectedComponentImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
#include "itkLabelGeometryImageFilter.h"

//#include "itkImageToVTKImageFilter.h" 

#include "itkLabelStatisticsImageFilter.h"

#include "itkImageDuplicator.h"
#include "itkShrinkImageFilter.h" 

#include "Vesselness/itkMultiScaleHessianSmoothed3DToVesselnessMeasureImageFilter.h"
#include "itkHessian3DToVesselnessMeasureImageFilter.h"
//#include "itkMultiScaleHessianBasedMeasureImageFilter.h"

#include "itkGradientVectorFlowImageFilter.h"
#include "itkGradientImageFilter.h"
#include "itkCovariantVector.h"
#include "itkRecursiveGaussianImageFilter.h"
#include "itkGradientRecursiveGaussianImageFilter.h"
#include "itkGradientAnisotropicDiffusionImageFilter.h"
#include "itkLaplacianRecursiveGaussianImageFilter.h"

#include "itkExtractImageFilter.h"
#include "itkVectorLinearInterpolateImageFunction.h"
#include "itkInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkImageFunction.h"

#include "itkPermuteAxesImageFilter.h"

#include "PointOperation.h"

#include "itkRescaleIntensityImageFilter.h"
#include "itkMaximumProjectionImageFilter.h"

#include "itkImageSeriesReader.h"
#include "itkTIFFImageIO.h"

#include "itkFastMarchingImageFilter.h"
#include "itkGeodesicActiveContourLevelSetImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"

#include "itkNumericTraits.h"
#include "itkPolyLineParametricPath.h"

#include "itkDanielssonDistanceMapImageFilter.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryBallStructuringElement.h"

#include "itkVectorContainer.h"

#include "itkVesselTubeSpatialObject.h"
#include "itkVesselTubeSpatialObjectPoint.h"

#include "itkRegionOfInterestImageFilter.h"
#include "itkGrayscaleErodeImageFilter.h"

#include "itkOtsuThresholdImageFilter.h"

#include <itkLaplacianRecursiveGaussianImageFilter.h>
#include <itkInvertIntensityImageFilter.h>

#include <vnl/vnl_matrix.h>
#include <vnl/vnl_vector.h>

#define PG_MAX(a,b)  (a>b? a: b)
#define PG_MIN(a,b)  (a<b? a: b)

typedef unsigned char  PGbyte;
typedef          char  PGchar;
typedef unsigned short PGushort;
typedef          short PGshort; 
typedef unsigned int   PGuint;
typedef          int   PGint;
typedef float          PGfloat;
typedef double         PGdouble;
typedef void           PGvoid;

typedef itk::Image<unsigned char, 3> IOImageType;
typedef IOImageType::Pointer IOImagePointer;

typedef itk::Image<unsigned short, 3> IOImageType1;
typedef IOImageType1::Pointer IOImagePointer1;

typedef itk::RGBPixel< unsigned char>  IORGBPixelType;
typedef itk::Image<IORGBPixelType, 3> IORGBImageType;
typedef IORGBImageType::Pointer IORGBImagePointer;

typedef itk::RGBPixel< float >  RGBPixelType;
typedef itk::Image<RGBPixelType, 3> RGBImageType;
typedef RGBImageType::Pointer RGBImagePointer;

typedef itk::Image< RGBPixelType, 2 > RGBImageType2D;
typedef RGBImageType2D::Pointer RGBImagePointer2D;

typedef itk::Image<signed int, 3> ImageType;
typedef ImageType::Pointer ImagePointer;

typedef itk::Image<unsigned int, 3> SomaImageLabelType;
typedef SomaImageLabelType::Pointer SomaImageLabelPointer;

typedef itk::Image<signed int, 2 > ImageType2D;
typedef ImageType2D::Pointer ImagePointer2D;

typedef itk::Image<float, 3> ProbImageType;
typedef ProbImageType::Pointer ProbImagePointer;

typedef itk::Image<float, 2> ProbImageType2D;
typedef ProbImageType2D::Pointer ProbImagePointer2D;

typedef itk::Image<unsigned short int, 3> LabelImageType;
typedef LabelImageType::Pointer LabelImagePointer;

typedef itk::Image<unsigned long int, 3> LabelImageTypeCCIF;//////////for connected component image filter in seed_centroid()
typedef LabelImageTypeCCIF::Pointer LabelImagePointerCCIF;

typedef itk::Image<unsigned short int, 2> LabelImageType2D;
typedef LabelImageType2D::Pointer LabelImagePointer2D;

typedef itk::CovariantVector< float, 3 >  GradientPixelType;
typedef itk::Image< GradientPixelType, 3 > GradientImageType;
typedef GradientImageType::Pointer GradientImagePointer;

typedef itk::Vector< float, 3 > SeedType;
typedef itk::Statistics::ListSample< SeedType > SampleType;
typedef SampleType::Pointer SamplePointer;


float norm_density(float x, float mu, float sigma);

//functions for eig3volume
static void tred2(double V[3][3], double d[3], double e[3]);
static void tql2(double V[3][3], double d[3], double e[3]);
void eigen_decomposition(double A[3][3], double V[3][3], double d[3]);
void eigen_decomposition2D(double A[2][2], double V[2][2], double d[2]);
double MAX_snake(double a, double b);
static double hypot2(double x, double y);
double absd(double val);

//functions for gvfc
PGfloat ***pgDmatrix(int M, int N, int K);
PGvoid pgFreeDmatrix(PGfloat ***Image, int M, int N, int K);

void GVFC(int XN, int YN, int ZN, float *f, float *ou, float *ov, float *oo, 
	  float mu, int ITER);


ProbImagePointer extract_one_component(int index, GradientImagePointer IG);
//PointList3D extract_local_maximum(ProbImagePointer V_saliency, int rad, double th, ImagePointer BW, bool local_maxima);
class VectorPixelAccessor;


class ImageOperation
{
	
public:

	ImageOperation(void);

	int SM;
	int SN;
	int SZ;
	bool mask_set;
	bool display_set; 
	float *u, *v, *o;

	int coding_method;

	float u1,u2,sigma1,sigma2; //intensity models
    float v_threshold; //automatically selected threshold value

    std::vector<float> I_cu;

	//IOImagePointer IIO;

	ImagePointer I;

	RGBImagePointer IRGB;
	ImagePointer IDisplay;

	//ImagePointer2D I2D;
	//ImagePointer DBW;//Depth Image
	ProbImagePointer IGMag;
	//ProbImagePointer IDist;
	ImagePointer ISeg;//Segmentation Image/Reconstruction
	
	LabelImagePointer IL;
    LabelImagePointer IL_Tracing;

	//soma
	ImagePointer IMask;//Soma Mask;
	LabelImagePointer ISoma;
	LabelImagePointer IVoronoi;
	int num_soma;
    PointList3D Centroid;
	std::vector<float> soma_size;
	std::vector<float> soma_radii;

	ProbImagePointer IVessel;

    GradientImagePointer IGVF;

    PointList3D SeedPt;
	//PointList3D SeedPt_mg;
	std::vector<PointList3D> SeedSets; //seed sets for different microglia cells

	int current_seed_idx;
    vnl_vector<int> visit_label;

	void SetCodingMethod(int in);  //set image coding method (centerline coding or vessel tube coding)
    //intensity models for 4-D region based open-curve snake
	void ImComputeInitBackgroundModel(double th);
	void ImComputeInitForegroundModel();
	void ImComputeForegroundModel(PointList3D Cu, std::vector<float> Ru);

    void ConvertReadImage();
    void ConvertWriteImage();
	//void ConvertWriteLabelImage();
	void ImRemoveSlice(int in);

	IOImagePointer ImBkSub(IOImagePointer In);
	void ImSeriesReadWrite(std::vector< std::string > filenames, const char *save_name, int shrink_factor, bool sixteen_bit);
	void ImRead(const char *filename);
    void Imcreate(unsigned char *datald, int *in_sz);
	void ImReadSoma(const char *filename);
	void ImRead_NoSmooth(const char *filename, int in);
	void ImDisplayRead(const char *filename, int shrink_factor);
	void ImWrite(const char *filename, ImagePointer ROI);
	void ImWrite(const char *filename, LabelImagePointer ROI);
    void ImWrite(const char *filename, int image_id);
	void ImWrite_Soma(const char *filename);
	void ImWrite_Segmentation(const char *filename);
	ImagePointer ImCopy();
	void ImMasking(const char *filename, int shrink_factor);
	void ImMasking(int shrink_factor);
	IOImagePointer ImInvert(IOImagePointer In);
	void ImShrink(int shrink_factor);
	IOImagePointer ImShrink(IOImagePointer In, int shrink_factor);

	void ImThinning(bool clean_skeleton, int min_length);

	void ImFastMarchingAnimation(int threshold);

	PointList3D ImSomaCentroidExtraction();

	void clear_IMask();
	void ImFastMarching_Spine(PointList3D spine_seeds); //segment the spines
	void ImFastMarching_Soma(PointList3D seg_seeds, int timeThreshold = 100, double curvatureScaling = 0.5, double rmsError = 0.02, const char *somaFileName = "Soma.tif");//segment single soma
	std::vector<float> ImFastMarchingI(PointList3D seg_seeds);
	void ImFastMarchingI_New(PointList3D seg_seeds); //automatic threshold selecting
    void ImFastMarchingII(PointList3D seg_seeds, int idx);

    void ImCoding(PointList3D Cu, std::vector<float> Ru, int id, bool tracing);
    void ImRemove_RedSeeds(PointList3D Cu, std::vector<float> Ru);
	void ImRefresh_TracingImage();
	void ImRefresh_LabelImage();
    void ImSmoothing(int smoothing_scale);
	void ImLaplacian_of_Gaussian();

    ProbImagePointer ImGaussian(ProbImagePointer I_In, int sigma);
	ImagePointer ImGaussian(ImagePointer I_In, int sigma);
	ImagePointer ImGaussian_XY(ImagePointer I_In, int sigma);
	IOImagePointer ImGaussian_XY(IOImagePointer I_In, int sigma);
	void seed_centroid();

	ImagePointer ImRescale(ProbImagePointer IR);
    ImagePointer ImRescale(ImagePointer IR);
	ImagePointer ImRescale(LabelImagePointer IR);

	ImagePointer2D ImMaxProjection(ImagePointer IInput);
	ProbImagePointer2D ImMaxProjection(ProbImagePointer IInput);
    LabelImagePointer2D ImMaxProjection(LabelImagePointer IInput);
	RGBImagePointer2D ImMaxProjection(RGBImagePointer IInput);
	RGBImagePointer2D ImMinProjection(RGBImagePointer IInput);
	RGBImagePointer2D ImMaxProjection1(ImagePointer IInput);

	void ComputeMultiVesselness(double sigma_min, double sigma_max, int sigma_step);
	void ComputeGVFVesselness();
	void ComputeGVFVesselness2D();
	//void ComputeGVFVesselness_CUDA();
	//void ComputeGVFVesselness_CUDA_New(float *vesselness, float *vx1, float *vy1, float *vz1);

    void normalizeGVF();
    //void computeGVF_C(int noise_level, int num_iteration, bool cuda, int smooth_scale);
	void computeGVF(int mu, int ITER, int smoothing_scale);

	void SeedDetection(float th, int detection_method, int radius);
	void SeedAdjustment(int iter_num);
	void OutputSeeds();
	bool SeedSparsify(SamplePointer seeds_candidate, SeedType query_point, int radius);

    ImagePointer2D extract_one_slice_yz(ImagePointer I_input, int index);
    ProbImagePointer2D extract_one_slice_yz(ProbImagePointer I_input, int index);

	ImagePointer2D extract_one_slice_xz(ImagePointer I_input, int index);
    ProbImagePointer2D extract_one_slice_xz(ProbImagePointer I_input, int index);

	ImagePointer2D extract_one_slice(ImagePointer I_input, int index);
    ProbImagePointer2D extract_one_slice(ProbImagePointer I_input, int index);

	
};

//accessor for extracting one component of the vector image
class VectorPixelAccessor  
{
public:
  typedef itk::CovariantVector<float,3>   InternalType;
  typedef                      float      ExternalType;

  void operator=( const VectorPixelAccessor & vpa )
    {
      m_Index = vpa.m_Index;
    }
  ExternalType Get( const InternalType & input ) const 
    {
    return static_cast<ExternalType>( input[ m_Index ] );
    }
  void SetIndex( unsigned int index )
    {
    m_Index = index;
    }
private:
  unsigned int m_Index;
};

#endif
