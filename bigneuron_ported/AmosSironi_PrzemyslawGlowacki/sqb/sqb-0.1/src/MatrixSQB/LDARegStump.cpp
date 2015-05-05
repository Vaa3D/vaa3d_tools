#include "mex.h"

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

#define qDebug(...) mexPrintf (__VA_ARGS__)
#define qFatal(...) do{ mexPrintf (__VA_ARGS__); mexErrMsgTxt("See above"); } while(0)

#include <SQB/Matlab/matlab_utils.hxx>

#include <SQB/Core/RegTree.h>
#include <SQB/Core/Utils.h>

#define SHOW_TIMINGS 0

typedef double WeightsType;
typedef float   FeatsType;

typedef Eigen::Array<FeatsType, Eigen::Dynamic, Eigen::Dynamic>     gFeatArrayType;
typedef Eigen::Array<WeightsType, Eigen::Dynamic, 1>                gResponseArrayType;
typedef Eigen::Array<WeightsType, Eigen::Dynamic, 1>                gWeightsArrayType;

typedef Eigen::Map< const gFeatArrayType >     				gMapFeatArrayType;
typedef Eigen::Map< const gResponseArrayType >              gMapResponseArrayType;
typedef Eigen::Map< const gWeightsArrayType >               gMapWeightsArrayType;


typedef SQB::MatrixFeatureIndexList<gMapFeatArrayType>          MatrixFeatureIndexListType;
typedef SQB::MatrixSampleIndexList<gMapFeatArrayType>           MatrixSampleIndexListType;
typedef SQB::MatrixFeatureValueObject<gMapFeatArrayType>        MatrixFeatureValueObjectType;
typedef SQB::MatrixResponseAndWeightsValueObject<gMapResponseArrayType>   MatrixResponseAndWeightsValueObject;

typedef SQB::RegTree<
            MatrixSampleIndexListType,
            MatrixFeatureIndexListType,
            MatrixFeatureValueObjectType,
            MatrixResponseAndWeightsValueObject >      RegTreeType;

void mexFunctionTrain(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    if (nrhs != 4)
    {
        mexPrintf("Usage: model = LDARegStump(feats, responses, weights, numLevels)\n");
        mexErrMsgTxt("Incorrect input format\n");
    }

    if (nlhs != 1)
        mexErrMsgTxt("One output args expected");

    #define mFeats (prhs[0])
    #define mResponses (prhs[1])
    #define mWeights (prhs[2])
    #define mNumLevels (prhs[3])

    MatlabInputMatrix<FeatsType> pFeats( mFeats, 0, 0, "feats" );
    MatlabInputMatrix<WeightsType> pResponses( mResponses, pFeats.rows(), 1, "labels" );
    MatlabInputMatrix<WeightsType> pWeights( mWeights, pFeats.rows(), 1, "labels" );

    if ( mxGetClassID(mNumLevels) != mxUINT32_CLASS )
        mexErrMsgTxt("numLevels must be UINT32");
    if ( mxGetNumberOfElements(mNumLevels) != 1 )
        mexErrMsgTxt("numLevels must be a scalar");

    const unsigned maxDepth = ((unsigned int *)mxGetData(mNumLevels))[0];

    RegTreeType tree;

    {           
        // for now just copy the values
        Eigen::Map< const gFeatArrayType >	feats( pFeats.data(), pFeats.rows(), pFeats.cols() );
        Eigen::Map< const gResponseArrayType > responses( pResponses.data(), pResponses.rows() );
        Eigen::Map< const gWeightsArrayType >	weights( pWeights.data(), pWeights.rows() );

        tree.learn( MatrixSampleIndexListType(feats),
                    MatrixFeatureIndexListType(feats),
                    MatrixFeatureValueObjectType(feats),
                    MatrixResponseAndWeightsValueObject( responses, weights ),
                    maxDepth);
    }

    plhs[0] = tree.saveToMatlab();

#undef mFeats
#undef mResponses
#undef mWeights
}

void mexFunctionTest(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    if (nrhs != 2)
    {
        mexPrintf("Usage: [score] = LDARegTreePredict( model, feats )\n");
        mexPrintf("\tfeats must be of type SINGLE\n");
        mexErrMsgTxt("Incorrect input format.\n");
    }

    if (nlhs != 1)
        mexErrMsgTxt("Two output args expected");

    #define mFeats (prhs[1])
    #define mModel (prhs[0])

    MatlabInputMatrix<FeatsType> pFeats( mFeats, 0, 0, "feats" );

    RegTreeType tree;
    tree.loadFromMatlab( mModel );

    // for now just copy the values
    Eigen::Map< const gFeatArrayType >	feats( pFeats.data(), pFeats.rows(), pFeats.cols() );

    gWeightsArrayType pred( pFeats.rows() );

    tree.predict(   MatrixSampleIndexListType(feats),
                    MatrixFeatureValueObjectType(feats),
                    pred );

    MatlabOutputMatrix<double>   outMatrix( &plhs[0], feats.rows(), 1 );
    for (unsigned i=0; i < feats.rows(); i++)
        outMatrix.data()[i] = pred.coeff(i);
}


void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
#ifdef SQB_TRAIN
    mexFunctionTrain(nlhs, plhs, nrhs, prhs);
#else
    mexFunctionTest(nlhs, plhs, nrhs, prhs);
#endif
}
