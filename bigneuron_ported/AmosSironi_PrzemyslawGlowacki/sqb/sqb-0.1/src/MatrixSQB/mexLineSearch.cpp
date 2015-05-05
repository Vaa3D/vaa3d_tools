#include "mex.h"

#define qDebug(...) mexPrintf (__VA_ARGS__)
#define qFatal(...) do{ mexPrintf (__VA_ARGS__); mexErrMsgTxt("See above"); } while(0)

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

#include <SQB/Matlab/matlab_utils.hxx>

#include <SQB/Core/Booster.h>
#include <SQB/Core/LineSearch.h>
#include <SQB/Core/Utils.h>

#define SHOW_TIMINGS 0

typedef double WeightsType;

typedef Eigen::Array<WeightsType, Eigen::Dynamic, 1>  ArrayType;
typedef Eigen::Map< const ArrayType, Eigen::Aligned>  ArrayMapType;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    if (nrhs != 4)
    {
        mexPrintf("Usage: alpha = LineSearch(prevScores, newScores, labels, lossType)\n");
        mexErrMsgTxt("Incorrect input format\n");
    }

#define mPrevScores (prhs[0])
#define mNewScores (prhs[1])
#define mLabels (prhs[2])
#define mLossType (prhs[3])

    if (nlhs != 1)
        mexErrMsgTxt("One output arg expected");

    char lossName[40];
    if ( mxGetString(mLossType, lossName, sizeof(lossName)) != 0 )
        mexErrMsgTxt("Error reading options.loss");

    SQB::LossType sqbLoss = SQB::ExpLoss;

    if (strcmp(lossName, "exploss") == 0)
        sqbLoss = SQB::ExpLoss;
    else if ( strcmp(lossName, "logloss") == 0 )
        sqbLoss = SQB::LogLoss;
    else if ( strcmp(lossName, "squaredloss") == 0 )
        sqbLoss = SQB::SquaredLoss;
    else
        mexErrMsgTxt("options.loss contains an invalid value");


    MatlabInputMatrix<WeightsType> pPrev( mPrevScores, 0, 1, "prevScores" );
    MatlabInputMatrix<WeightsType> pNew( mNewScores, pPrev.rows(), 1, "newScores" );
    MatlabInputMatrix<WeightsType> pLabels( mLabels, pPrev.rows(), 1, "labels" );

    // create mappings
    ArrayMapType prevMap( pPrev.data(), pPrev.rows(), pPrev.cols() );
    ArrayMapType newMap( pNew.data(), pNew.rows(), pNew.cols() );
    ArrayMapType labelsMap( pLabels.data(), pLabels.rows(), pLabels.cols() );



    SQB::LineSearch< ArrayType, ArrayMapType >  LS( prevMap, newMap, labelsMap, sqbLoss );

    WeightsType alpha = LS.run();

    MatlabOutputMatrix<WeightsType>   outMatrix( &plhs[0], 1, 1 );
    outMatrix.data()[0] = alpha;
}
