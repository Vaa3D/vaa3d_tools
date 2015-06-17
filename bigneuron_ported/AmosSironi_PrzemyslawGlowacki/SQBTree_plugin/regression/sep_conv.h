#ifndef _SEP_CONV_H
#define _SEP_CONV_H

#include <iostream>
#include <fstream>
#include <string>
#include "sqb_0.1/include/Eigen/Dense"
#include "superannotator_sel/Matrix3D.h"
#include <SQB/Core/RegTree.h>
#include <SQB/Core/Utils.h>
#include <SQB/Core/Booster.h>
#include <SQB/Core/LineSearch.h>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkConvolutionImageFilter.h"
#include "itkImageRandomNonRepeatingConstIteratorWithIndex.h"
#include "itkRescaleIntensityImageFilter.h"

//#include <omp.h>

using namespace std;
using namespace Eigen;

#define MAXBUFSIZE  ((int) 1e6)

typedef Eigen::VectorXf VectorTypeFloat;
typedef Eigen::VectorXd VectorTypeDouble;
typedef Eigen::MatrixXd MatrixTypeDouble;
typedef Eigen::MatrixXf MatrixTypeFloat;

//typedef SQB::TreeBooster<
//            MatrixSampleIndexListType,
//            MatrixFeatureIndexListType,
//            MatrixFeatureValueObjectType,
//            MatrixClassifResponseValueObjectType >      TreeBoosterType;



////functions declarations
///
// read text file and store it in Eigen matrix
MatrixTypeDouble readMatrix(const char *filename);

template<typename MatrixType>
void writeMatrix(const char *filename,MatrixType matrix_to_write);

//prepare 1D kernel for itk separable convolution
template<typename ImageType, typename VectorType>
   void prepare1DKernel( typename ImageType::Pointer &kernel, const unsigned dir, const VectorType &origVec );

//convolve 3D image with separable filters (itk arguments)
template<typename ImageType, typename VectorType>
   void wholeConvolveSepFilter( typename ImageType::Pointer &input_img, typename ImageType::Pointer kernelX,typename ImageType::Pointer kernelY,typename ImageType::Pointer kernelZ, Matrix3D<float> &out);

//convolve 3D image with separable filters (eigen arguments)
template<typename ImageType, typename VectorType>
   void wholeConvolveSepFilter( typename ImageType::Pointer &input_img, const VectorType &kernel_eig_x,const VectorType &kernel_eig_y,const VectorType &kernel_eig_z, Matrix3D<float> &out);

//convolve 3D image with separable filters (split eigen vector in 3 equal parts)
template<typename ImageType, typename VectorType>
   void wholeConvolveSepFilterSplitVec( typename ImageType::Pointer &input_img, const VectorType &kernel_eig, Matrix3D<float> &out);

// store itk image into eigen colmun vector
template<typename ImageType, typename VectorType>
VectorType itkImage2EigenVector( typename ImageType::Pointer &input_img,const unsigned int n_rand_samples,const unsigned int tot_n_pixels);

//create itk image reshaping eigen vector
template<typename ImageType, typename VectorType>
typename ImageType::Pointer eigenVector2itkImage(const VectorType &input_vector, const typename ImageType::SizeType &size_image);

//convolve image with bank of sep filters
//TODO: VectorType and MatrixType should be compatible !!!
template<typename ImageType, typename MatrixType, typename VectorType>
void convolveSepFilterBank(MatrixType &sep_features_all,typename ImageType::Pointer &input_img, const MatrixType &sep_filters_matrix);
//MatrixType convolveSepFilterBank( typename ImageType::Pointer &input_img, const MatrixType &sep_filters_matrix);


//convolve image with bank of sep filters and combine them to obtain  convolutions with original filters
//TODO: VectorType and MatrixType should be compatible !!!
template<typename ImageType, typename MatrixType, typename VectorType>
void convolveSepFilterBankComb(MatrixType &nonsep_features_all, typename ImageType::Pointer &input_img, const MatrixType &sep_filters_matrix, const MatrixType &weight_matrix, const float scale_factor );
//MatrixType convolveSepFilterBankComb( typename ImageType::Pointer &input_img, const MatrixType &sep_filters_matrix, const MatrixType &weight_matrix, const float scale_factor );


template<typename MatrixType>
void getContextFeatures(MatrixType &nonsep_features_all,const MatrixType &nonsep_features_all_no_context,const float sigma_pool = 1.0);



template<typename ImageType, typename MatrixType, typename VectorType>
void computeFeaturesSepComb(MatrixType &nonsep_features_all, typename ImageType::Pointer &input_img, const MatrixType &sep_filters_matrix, const MatrixType &weight_matrix, const float scale_factor,bool add_context_features =false );
//MatrixType computeFeaturesSepComb( typename ImageType::Pointer &input_img, const MatrixType &sep_filters_matrix, const MatrixType &weight_matrix, const float scale_factor );

#endif
