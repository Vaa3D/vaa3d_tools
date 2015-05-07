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

#define MAXBUFSIZE  ((int) 1e6)

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


// TODO: pass options
static void SQBTreesTrain(MatrixXf &input_feats,MatrixXf &labels,const unsigned int maxIters){


    TreeBoosterType TB;

    // we will use a random sampler
    SQB::TreeBoosterNaiveResampler< TreeBoosterType::ResamplerBaseObjectType::WeightsArrayType,
                                    TreeBoosterType::ResamplerBaseObjectType::LabelsArrayType >  resampler;

    TB.setResamplerObject( &resampler );


    TB.printOptionsSummary();
//    TB.learn( TreeBoosterType::SampleListType(input_feats),
//            TreeBoosterType::FeatureListType(input_feats),
//            TreeBoosterType::FeatureValueObjectType(input_feats),
//            TreeBoosterType::ClassifierResponseValueObjectType(labels),
//            maxIters );
//    TB.printOptionsSummary();



   // TB.saveToLibconfig(base);


}


/*


//static void SQBTreesPredict(int model, MatrixXf &feats, const unsigned int maxIters){

gResponseArrayType SQBTreesPredict(const libconfig::Setting &model, MatrixXf &feats){


    TreeBoosterType TB;

    // load model
    //TB.loadFromMatlab( mModel );
    TB.loadFromLibconfig(model);


    unsigned maxIters = TB.numWeakLearners();
    /*
    if (nrhs >= 3)
    {
        MatlabInputMatrix<unsigned int> pMaxIters( mMaxIters, 1, 1, "maxiters" );
        unsigned inputMaxIters = pMaxIters.data()[0];

        if (inputMaxIters <= 0)
            mexErrMsgTxt("maxIters must be higher than zero.");

        if (inputMaxIters > maxIters)
            mexPrintf("-- WARNING: maxIters is greater than the number of weaklearners used!\n");
        else
        {
            maxIters = inputMaxIters;
            mexPrintf("Limiting number of weak learners to %d\n", (int)maxIters);
        }
    }
    //

    // for now just copy the values
   // gFeatArrayType feats = Eigen::Map< const gFeatArrayType >( pFeats.data(), pFeats.rows(), pFeats.cols() );

    TreeBoosterType::ResponseArrayType newScores;
    TB.predict( TreeBoosterType::SampleListType(input_feats),
                TreeBoosterType::FeatureValueObjectType(input_feats),
                newScores,
                maxIters );

    return newScores;
   // MatlabOutputMatrix<double>   outMatrix( &plhs[0], feats.rows(), 1 );
   // for (unsigned i=0; i < feats.rows(); i++)
   //     outMatrix.data()[i] = newScores.coeff(i);

}
*/
