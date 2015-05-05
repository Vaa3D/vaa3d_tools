// Compile with: g++ -I../../include -I../../../liblbfgs-1.10/include ../../../liblbfgs-1.10/lib/lbfgs.o  -fopenmp -O sqb_trees.cpp -o ../../build/sqb_trees



//#include "mex.h"

// This file is part of SQBlib, a gradient boosting/boosted trees implementation.
//
// Copyright (C) 2012 Carlos Becker, http://sites.google.com/site/carlosbecker
//
// SQBlib is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// SQBlib is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with SQBlib.  If not, see <http://www.gnu.org/licenses/>.

//#define qDebug(...) mexPrintf (__VA_ARGS__)
//#define qFatal(...) do{ mexPrintf (__VA_ARGS__); mexErrMsgTxt("See above"); } while(0)
#define qDebug(...) printf (__VA_ARGS__)
#define qFatal(...) do{ printf (__VA_ARGS__); exit(1); } while(0)

//#include <SQB/Matlab/matlab_utils.hxx>

#define SHOW_TIMINGS 0

#ifndef SHOW_TIMINGS
    #define SHOW_TIMINGS 0
#endif

#include <SQB/Core/RegTree.h>
#include <SQB/Core/Utils.h>

#include <SQB/Core/Booster.h>

#include <SQB/Core/LineSearch.h>

typedef SQB::TreeBoosterWeightsType  WeightsType;
typedef float   FeatsType;

typedef Eigen::Array<FeatsType, Eigen::Dynamic, Eigen::Dynamic>     gFeatArrayType;
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

void mexFunctionTrain(TreeBoosterType &TB/*int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]*/)
{
//    if (nrhs != 4)
//    {
//        mexPrintf("Usage: [model] = SQBTreesTrain( feats, labels, maxIters, options )\n");
//        mexPrintf("\tfeats must be of type SINGLE\n");
//        mexPrintf("\tOptions contains the following fields:\n");
//        mexPrintf("\t\t.loss = 'exploss', 'squaredloss' or 'logloss'\n");
//        mexPrintf("\t\t.shrinkageFactor = between 0 and 1. For instance, 0.02\n");
//        mexPrintf("\t\t.subsamplingFactor = between 0 and 1. Initial suggestion: 0.5\n");
//        mexPrintf("\t\t.maxTreeDepth >= 0. UINT32. Initial suggestion: 2\n");
//        mexPrintf("\t\t.disableLineSearch, UINT32. If != 0 then line search is disabled and shrinkageFactor is used as the step length.\n");
//        mexPrintf("\t\t.mtry, UINT32. Num of variables to search at each split, randomly chosen (as in Random Forests). Optional, otherwise it equals number of columns in feats.\n");
//        mexPrintf("\t\t.randSeed. UINT32. If not provided, time is used to generate a seed.\n");
//        mexPrintf("\t\t.verboseOutput, UINT32. If != 0 then verbose output is enabled (default = false if not specified).\n");
//        mexErrMsgTxt("Incorrect input format\n");
//    }

//    if (nlhs != 1)
//        mexErrMsgTxt("One output arg expected");

//    #define mFeats (prhs[0])
//    #define mLabels (prhs[1])
//    #define mMaxIters (prhs[2])
//    #define mOptions (prhs[3])

//    MatlabInputMatrix<FeatsType> pFeats( mFeats, 0, 0, "feats" );
//    MatlabInputMatrix<WeightsType> pLabels( mLabels, pFeats.rows(), 1, "labels" );
//    MatlabInputMatrix<unsigned int> pMaxIters( mMaxIters, 1, 1, "maxiters" );

//    const unsigned maxIters = pMaxIters.data()[0];

//    TreeBoosterType TB;

    // we will use a random sampler
    SQB::TreeBoosterNaiveResampler< TreeBoosterType::ResamplerBaseObjectType::WeightsArrayType,
                                    TreeBoosterType::ResamplerBaseObjectType::LabelsArrayType >  resampler;

    TB.setResamplerObject( &resampler );

//    // read options, a bit messy right now
//    {
//        mxArray *pLoss = mxGetField( mOptions, 0, "loss" );
//        if (pLoss == NULL)  mexErrMsgTxt("options.loss not found");

//        mxArray *pSF = mxGetField( mOptions, 0, "shrinkageFactor" );
//        if (pSF == NULL)  mexErrMsgTxt("options.shrinkageFactor not found");

//        mxArray *pSubSampFact = mxGetField( mOptions, 0, "subsamplingFactor" );
//        if (pSubSampFact == NULL)  mexErrMsgTxt("options.subsamplingFactor not found");

//        mxArray *pMaxDepth = mxGetField( mOptions, 0, "maxTreeDepth" );
//        if (pMaxDepth == NULL)  mexErrMsgTxt("options.maxTreeDepth not found");

//        mxArray *pRandSeed = mxGetField( mOptions, 0, "randSeed" );
//        const bool usingCustomRandSeed = (pRandSeed != NULL);

//        mxArray *pDisableLineSearch = mxGetField( mOptions, 0, "disableLineSearch" );
//        const bool disableLineSearchSpecified = (pDisableLineSearch != NULL);

//        mxArray *pVerboseOutput = mxGetField( mOptions, 0, "verboseOutput" );
//        const bool verboseOutputSpecified = (pVerboseOutput != NULL);

//        mxArray *pMTry = mxGetField( mOptions, 0, "mtry" );
//        const bool mtrySpecified = (pMTry != NULL);

//        char lossName[40];
//        if ( mxGetString(pLoss, lossName, sizeof(lossName)) != 0 )
//            mexErrMsgTxt("Error reading options.loss");

//        if ( mxGetClassID(pSF) != mxDOUBLE_CLASS )
//            mexErrMsgTxt("options.shrinkageFactor must be double");
//        if ( mxGetNumberOfElements(pSF) != 1 )
//            mexErrMsgTxt("options.shrinkageFactor must be a scalar");

//        {
//            const double sf = ((double *)mxGetData(pSF))[0];
//            if ( sf <= 0 || sf > 1) mexErrMsgTxt("Shrinkage factor must be in (0,1]");
//            TB.setShrinkageFactor( sf );
//        }

//        if (strcmp(lossName, "exploss") == 0)
//            TB.setLoss( SQB::ExpLoss );
//        else if ( strcmp(lossName, "logloss") == 0 )
//            TB.setLoss( SQB::LogLoss );
//        else if ( strcmp(lossName, "squaredloss") == 0 )
//            TB.setLoss( SQB::SquaredLoss );
//        else
//            mexErrMsgTxt("options.loss contains an invalid value");

//        if ( mxGetClassID(pSubSampFact) != mxDOUBLE_CLASS )
//            mexErrMsgTxt("options.subsamplingFactor must be double");
//        if ( mxGetNumberOfElements(pSubSampFact) != 1 )
//            mexErrMsgTxt("options.subsamplingFactor must be a scalar");

//        {
//            const double ss = ((double *)mxGetData(pSubSampFact))[0];
//            if ( ss <= 0 || ss > 1 )    mexErrMsgTxt("Subsampling factor must be in (0,1]");
//            resampler.setResamplingFactor( ss );
//        }

//        if ( mxGetClassID(pMaxDepth) != mxUINT32_CLASS )
//            mexErrMsgTxt("options.pMaxDepth must be UINT32");
//        if ( mxGetNumberOfElements(pMaxDepth) != 1 )
//            mexErrMsgTxt("options.maxDepth must be a scalar");

//        {
//            const unsigned maxDepth = ((unsigned int *)mxGetData(pMaxDepth))[0];
//            if (maxDepth < 0)   // not gonna happen if it is unsigned
//                mexErrMsgTxt("Minimum maxDepth is 0");
//            TB.setMaxTreeDepth( maxDepth );
//        }

//        if ( disableLineSearchSpecified )
//        {
//            if ( mxGetClassID(pDisableLineSearch) != mxUINT32_CLASS )
//                mexErrMsgTxt("options.disableLineSearch must be UINT32");

//            if ( mxGetNumberOfElements(pDisableLineSearch) != 1 )
//                mexErrMsgTxt("options.disableLineSearch must be a scalar");

//            TB.setDisableLineSearch(((unsigned int *)mxGetData(pDisableLineSearch))[0] != 0);
//        }
//        else
//            TB.setDisableLineSearch(false);

//        if ( verboseOutputSpecified )
//        {
//            if ( mxGetClassID(pVerboseOutput) != mxUINT32_CLASS )
//                mexErrMsgTxt("options.verboseOutput must be UINT32");

//            if ( mxGetNumberOfElements(pVerboseOutput) != 1 )
//                mexErrMsgTxt("options.verboseOutput must be a scalar");

//            TB.setVerboseOutput(((unsigned int *)mxGetData(pVerboseOutput))[0] != 0);
//        }
//        else
//            TB.setVerboseOutput(false);

//        if ( mtrySpecified )
//        {
//            if ( mxGetClassID(pMTry) != mxUINT32_CLASS )
//                mexErrMsgTxt("options.mtry must be UINT32");

//            if ( mxGetNumberOfElements(pMTry) != 1 )
//                mexErrMsgTxt("options.mtry must be a scalar");

//            TB.setMTry(((unsigned int *)mxGetData(pMTry))[0]);
//        }
//        else
//            TB.setDisableLineSearch(false);

//        if (usingCustomRandSeed)
//        {
//            if ( mxGetClassID(pRandSeed) != mxUINT32_CLASS )
//                mexErrMsgTxt("options.randSeed must be UINT32");

//            if ( mxGetNumberOfElements(pRandSeed) != 1 )
//                mexErrMsgTxt("options.randSeed must be a scalar");

//            TB.setRandSeed( ((unsigned int *)mxGetData(pRandSeed))[0] );
//        } else
//        {
//            TB.setRandSeed( time(NULL) );
//        }
//    }

    {
        //pFeats.data() returns const FeatsType*
        //pFeats.rows() and pFeats.cols() return unsigned int

        //pLabels.data() returns SQB::TreeBoosterWeightsType (i.e. double)
        //pLabels.rows() returns unsigned int

//        // for now just copy the values
//        gFeatArrayType feats = Eigen::Map< const gFeatArrayType >( pFeats.data(), pFeats.rows(), pFeats.cols() );
//        TreeBoosterType::ResponseArrayType labels = Eigen::Map< const TreeBoosterType::ResponseArrayType >( pLabels.data(), pLabels.rows() );

//        TB.printOptionsSummary();
//        TB.learn( TreeBoosterType::SampleListType(feats),
//                TreeBoosterType::FeatureListType(feats),
//                TreeBoosterType::FeatureValueObjectType(feats),
//                TreeBoosterType::ClassifierResponseValueObjectType(labels),
//                maxIters );
//        TB.printOptionsSummary();

      const unsigned maxIters = 200;
      // for now just copy the values
      FeatsType featuresArray[] = {3, 1, 2,
                                   6, 3, 4,
                                   -1, -2, -4,
                                   -2, -3, -1
                                  };
      unsigned int featuresColsNo = 3;
      unsigned int featuresRowsNo = 4;

      WeightsType labelsArray[] = {1,
                                   1,
                                  -1,
                                  -1};
      unsigned int labelsRowsNo = featuresRowsNo;

      gFeatArrayType feats = Eigen::Map< const gFeatArrayType >( featuresArray, featuresRowsNo, featuresColsNo );
      TreeBoosterType::ResponseArrayType labels = Eigen::Map< const TreeBoosterType::ResponseArrayType >( labelsArray, labelsRowsNo );

      TB.printOptionsSummary();
      TB.learn( TreeBoosterType::SampleListType(feats),
                TreeBoosterType::FeatureListType(feats),
                TreeBoosterType::FeatureValueObjectType(feats),
                TreeBoosterType::ClassifierResponseValueObjectType(labels),
                maxIters );
      TB.printOptionsSummary();
    }

//    plhs[0] = TB.saveToMatlab();

//#undef mFeats
//#undef mLabels
//#undef mMaxIters
//#undef mPredFeats
}


void mexFunctionTest(TreeBoosterType &TB) //int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  printf("Testing!\n");
//    if (nrhs < 2 || nrhs > 3)
//    {
//        mexPrintf("Usage: [score] = SQBTreesPredict( model, feats, [maxIters] )\n");
//        mexPrintf("\tfeats must be of type SINGLE\n");
//        mexPrintf("\tmaxIters is optional and limits the amount of weak learners evaluated for classification.");
//        mexErrMsgTxt("Incorrect input format.\n");
//    }

//    if (nlhs != 1)
//        mexErrMsgTxt("Two output args expected");

//    #define mFeats (prhs[1])
//    #define mModel (prhs[0])
//    #define mMaxIters (prhs[2])

//    MatlabInputMatrix<FeatsType> pFeats( mFeats, 0, 0, "feats" );

//    //const unsigned maxIters = pMaxIters.data()[0];

//    TreeBoosterType TB;

//    // load model
//    TB.loadFromMatlab( mModel );

    unsigned maxIters = TB.numWeakLearners();
//    if (nrhs >= 3)
//    {
//        MatlabInputMatrix<unsigned int> pMaxIters( mMaxIters, 1, 1, "maxiters" );
//        unsigned inputMaxIters = pMaxIters.data()[0];

//        if (inputMaxIters <= 0)
//            mexErrMsgTxt("maxIters must be higher than zero.");

//        if (inputMaxIters > maxIters)
//            mexPrintf("-- WARNING: maxIters is greater than the number of weaklearners used!\n");
//        else
//        {
//            maxIters = inputMaxIters;
//            mexPrintf("Limiting number of weak learners to %d\n", (int)maxIters);
//        }
//    }


    // for now just copy the values
    FeatsType testFeaturesArray[] = {3, 1, 2,
                                 6, 3, 4,
                                 -1, -2, -4,
                                 -2, -3, -1
                                };
    unsigned int testFeaturesColsNo = 3;
    unsigned int testFeaturesRowsNo = 4;
    gFeatArrayType feats = Eigen::Map< const gFeatArrayType >( testFeaturesArray, testFeaturesRowsNo, testFeaturesColsNo );

    TreeBoosterType::ResponseArrayType newScores;
    TB.predict( TreeBoosterType::SampleListType(feats),
                TreeBoosterType::FeatureValueObjectType(feats),
                newScores,
                maxIters );

    for (unsigned i=0; i < feats.rows(); i++) {
      std::cout << newScores.coeff(i) << std::endl;
//      outMatrix.data()[i] = newScores.coeff(i);
    }

//    MatlabOutputMatrix<double>   outMatrix( &plhs[0], feats.rows(), 1 );
//    for (unsigned i=0; i < feats.rows(); i++)
//        outMatrix.data()[i] = newScores.coeff(i);
//}


//void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
//{
//#ifdef SQBTREES_TRAIN
//    mexFunctionTrain(nlhs, plhs, nrhs, prhs);
//#else
//    mexFunctionTest(nlhs, plhs, nrhs, prhs);
//#endif
}

int main(int argc, char *argv[]) {
  printf("Wooow!\n");

  TreeBoosterType TB;

  mexFunctionTrain(TB);

  mexFunctionTest(TB);

//  MEX_PATH = '../build';

//  addpath(MEX_PATH);

//  file_data = load('../datasets/airquality.mat');
//  Data = [file_data.wind, file_data.temperature, file_data.radiation];
//  Labels = file_data.ozone;

//  test.X = Data(1:2:end,:);
//  test.Y = Labels(1:2:end,:);

//  train.X = Data(2:2:end,:);
//  train.Y = Labels(2:2:end,:);

//  opts = [];
//  opts.loss = 'squaredloss'; % can be logloss or exploss

//  % this has to be not too high (max 1.0)
//  opts.shrinkageFactor = 0.01;
//  opts.subsamplingFactor = 0.2;
//  opts.maxTreeDepth = uint32(2);  % this was the default before customization
//  opts.randSeed = uint32(rand()*1000);

//  numIters = 600;
//  tic;
//  model = SQBMatrixTrain(single(train.X), train.Y, uint32(numIters), opts);
//  toc

//  %% Test model

//  pred = SQBMatrixPredict( model, single(test.X) );
//  outErr = sum((pred - test.Y).^2);

//  fprintf('Prediction error (MSE): %f\n', outErr / length(pred));

  return 0;
}
