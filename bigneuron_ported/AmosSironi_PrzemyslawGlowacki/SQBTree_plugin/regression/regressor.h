#include <iostream>
#include <fstream>
#include <string>
#include "Eigen/Dense"
#include "superannotator_sel/Matrix3D.h"


//#include "sqb_0.1/include/SQB/Matlab/matlab_utils.hxx"
#include "sqb_0.1/include/SQB/Core/RegTree.h"
#include "sqb_0.1/include/SQB/Core/Utils.h"
#include "sqb_0.1/include/SQB/Core/Booster.h"
#include "sqb_0.1/include/SQB/Core/LineSearch.h"


#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkConvolutionImageFilter.h"
#include "itkImageRandomNonRepeatingConstIteratorWithIndex.h"
#include "itkRescaleIntensityImageFilter.h"

//#include <omp.h>

using namespace std;
using namespace Eigen;

//#define MAXBUFSIZE  ((int) 1e6)

typedef Eigen::VectorXf VectorTypeFloat;
typedef Eigen::VectorXd VectorTypeDouble;
typedef Eigen::MatrixXd MatrixTypeDouble;
typedef Eigen::MatrixXf MatrixTypeFloat;


typedef SQB::TreeBoosterWeightsType  WeightsType;
typedef float   FeatsType;

//typedef Eigen::Array<FeatsType, Eigen::Dynamic, Eigen::Dynamic>     gFeatArrayType;
typedef Eigen::MatrixXf gFeatArrayType;
typedef Eigen::Array<WeightsType, Eigen::Dynamic, 1>                gResponseArrayType;
typedef Eigen::Array<WeightsType, Eigen::Dynamic, 1>                gWeightsArrayType;


typedef SQB::MatrixFeatureIndexList<gFeatArrayType>          MatrixFeatureIndexListType;
typedef SQB::MatrixSampleIndexList<gFeatArrayType>           MatrixSampleIndexListType;
typedef SQB::MatrixFeatureValueObject<gFeatArrayType>        MatrixFeatureValueObjectType;
typedef SQB::MatrixSingleResponseValueObject<gResponseArrayType>   MatrixClassifResponseValueObjectType;

typedef SQB::TreeBooster<
            MatrixSampleIndexListType,
            MatrixFeatureIndexListType,
            MatrixFeatureValueObjectType,
            MatrixClassifResponseValueObjectType >      TreeBoosterType;

////functions declarations
///
///
///
///

//[model] = SQBTreesTrain( feats, labels, maxIters, options )
//static void SQBTreesTrain(MatrixXf &input_feats,MatrixXf &labels,const unsigned int maxIters,libconfig::Setting &base);
static void SQBTreesTrain(MatrixXf &input_feats,MatrixXf &labels,const unsigned int maxIters);


// [score] = SQBTreesPredict( model, feats, [maxIters] )
//static void SQBTreesPredict(int model, MatrixXf &feats, const unsigned int maxIters);
////gResponseArrayType SQBTreesPredict(const libconfig::Setting &model, MatrixXf &feats);

