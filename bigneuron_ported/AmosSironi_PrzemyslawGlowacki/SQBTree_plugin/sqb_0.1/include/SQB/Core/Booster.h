#ifndef _SQB_TREE_BOSTER_H
#define _SQB_TREE_BOSTER_H

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

#include </cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/sqb_0.1/include/SQB/Core/RegTree.h>
#include </cvlabdata1/home/asironi/vaa3d/vaa3d_tools/bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/sqb_0.1/include/SQB/Core/Utils.h>

namespace SQB
{
    enum LossType { ExpLoss, LogLoss, SquaredLoss };
}

#include <SQB/Core/LineSearch.h>

namespace SQB
{
    // weak learner structure
    template<typename TRegTreeType, typename TWeightsType>
    struct TreeBoosterWL
    {
        TRegTreeType     tree;
        TWeightsType     alpha;  // weighting coefficient
    };
    
    template<typename ResponsePredType, typename AlphaType>
    struct PredictOperator
    {
            ResponsePredType &mScores;
            AlphaType                mAlpha;
            
            PredictOperator( ResponsePredType &scores, AlphaType alpha ) : 
                    mScores(scores), mAlpha(alpha)
            {
            }

            template<typename ScalarType>
            inline void apply( const unsigned offset, const ScalarType predVal ) const
            {       
                #pragma omp atomic
                mScores.coeffRef( offset ) += mAlpha * predVal;
            }
    };

    // this is fixed for precision issues
    typedef double TreeBoosterWeightsType;

    // base class for resampler
    template<typename TWeightsArrayType, typename TLabelsArrayType>
    class TreeBoosterResamplerBase
    {
    public:
        typedef TWeightsArrayType   WeightsArrayType;
        typedef TLabelsArrayType    LabelsArrayType;

        //  labelArray:     labels (-1/1)
        //  weightArray:    weights
        //  sampledIdxs:    should be resized to the desired number, and indeces from 0 to labels.size() - 1 should be filled in
        //      The resampler can modify the weights, but the new weightArray must have the same size as labelArray
        //          and contain valid entries
        virtual void resample( const LabelsArrayType &labelArray,  WeightsArrayType &weightArray, std::vector<unsigned int> *sampledIdxs ) const = 0;

        virtual bool modifiesWeights() const = 0;   // if this class modifies the weight array
    };

    template<typename WeightsArrayType, typename LabelsArrayType>
    class TreeBoosterNaiveResampler : public TreeBoosterResamplerBase<WeightsArrayType, LabelsArrayType>
    {
    private:
        double mResamplingFactor;
    public:
        TreeBoosterNaiveResampler() : mResamplingFactor(0.5) {}

        void setResamplingFactor( double f ) { mResamplingFactor = f; }

        virtual void resample( const LabelsArrayType &labelArray,  WeightsArrayType &weightArray, std::vector<unsigned int> *sampledIdxs ) const
        {
            const unsigned N = labelArray.size();

            double DtoSample = round(mResamplingFactor * N);
            if (DtoSample < 1)  DtoSample = 1;
            if (DtoSample > N)  DtoSample = N;

            sampleWithoutReplacement( (unsigned) DtoSample, N, sampledIdxs );
        }

        virtual bool modifiesWeights() const { return false; }   // if this class modifies the weight array
    };

    template< typename TSampleListType, typename TFeatureListType, typename TFeatureValueObjectType, typename TClassifierResponseValueObjectType >
    class TreeBooster
    {
    public:
        #warning  TODO: rename to WeightScalarType
        typedef TreeBoosterWeightsType  WeightsType;    // type used for responses and weights

        // used for response (class) and internally for weights/response of the regression trees built
        typedef Eigen::Array<WeightsType, Eigen::Dynamic, 1>                ResponseArrayType;
        typedef Eigen::Array<WeightsType, Eigen::Dynamic, 1>                WeightsArrayType;

    public:
        typedef TSampleListType         SampleListType;
        typedef TFeatureListType        FeatureListType;
        typedef TFeatureValueObjectType FeatureValueObjectType;

        // the response to train a tree is from an Eigen Array
        typedef MatrixResponseAndWeightsValueObject<ResponseArrayType>   TreeResponseValueObjectType;

        // response of the classifier (i.e. label)
        typedef TClassifierResponseValueObjectType   ClassifierResponseValueObjectType;

        // type of trees to be grown
        typedef RegTree<SampleListType, FeatureListType, FeatureValueObjectType, TreeResponseValueObjectType>  RegTreeType;

        // weights (and response) type derived from MatrixFeatureValueObjectType
        //typedef typename MatrixFeatureValueObjectType::WeightScalarType  WeightsType;

        //#warning  TODO: rename to FeatureScalarType
        //typedef typename MatrixFeatureValueObjectType::FeatureScalarType  ScalarType;

        typedef TreeBoosterWL<RegTreeType, WeightsType>  TreeBoosterWLType;

        typedef TreeBoosterResamplerBase<WeightsArrayType, ResponseArrayType>   ResamplerBaseObjectType;

    private:
        std::vector<TreeBoosterWLType> mWeakLearners;
        LossType                      mLossType;
        WeightsType                   mShrinkageFactor;
        unsigned int                  mMaxTreeDepth;

        // if alpha should be searched on the whole training set
        //  or only on the current resampled subset
        bool                          mAlphaSearchOnSubset;
        unsigned int                  mRandSeed;
        
        
        // if it should ignore linesearch
        bool						mDisableLineSearch;

        // resampler
        const ResamplerBaseObjectType     *mResamplerObj;
        
        unsigned int mMTry;
        
        bool	mVerboseOutput;

    public:

        inline WeightsType expLoss( const ResponseArrayType &Y, const ResponseArrayType &curPred, WeightsType normFactor = 1.0 )
        {
            return ((-Y * curPred).exp() * normFactor).sum();
        }

        inline WeightsType logLoss( const ResponseArrayType &Y, const ResponseArrayType &curPred, WeightsType normFactor = 1.0 )
        {
            return ((1 + std::numeric_limits<double>::epsilon() + (- 2 * Y * curPred).exp()).log() * normFactor ).sum();
        }

        inline WeightsType squaredLoss( const ResponseArrayType &Y, const ResponseArrayType &curPred, WeightsType normFactor = 1.0 )
        {
            return ( ( Y - curPred ).square() ).sum() * normFactor;
        }

        TreeBooster()
        {
            mLossType = ExpLoss;
            //mLossType = LogLoss;

            mShrinkageFactor = 0.025;

            mMaxTreeDepth = 2;
            mAlphaSearchOnSubset = false;

            mResamplerObj = 0;
            
            mDisableLineSearch = false;
            
            mMTry = 0;
            
            mVerboseOutput = true;
        }
        
        void setVerboseOutput(bool yes)
        {
            mVerboseOutput = yes;
        }

        void setRandSeed(unsigned int seed)
        {
            mRandSeed = seed;
        }
        
        void setMTry(unsigned int mtry)
        {
            mMTry = mtry;
        }
        
        void setDisableLineSearch(bool yes)
        {
            mDisableLineSearch = yes;
        }

        void setLoss( LossType L )
        {
            mLossType = L;
        }

        void setAlphaSearchOnSubset( bool searchOnSubset )
        {
            mAlphaSearchOnSubset = searchOnSubset;
        }

        void setShrinkageFactor(WeightsType sf)
        {
            mShrinkageFactor = sf;
        }

        void setResamplerObject( const ResamplerBaseObjectType *sampler )
        {
            mResamplerObj = sampler;
        }

        void setMaxTreeDepth( unsigned td )
        {
            mMaxTreeDepth = td;
        }

        static const char * getLossDescription( LossType L )
        {
            switch(L)
            {
            case LogLoss:
                return "Log Loss";
            case ExpLoss:
                return "Exp Loss";
            case SquaredLoss:
                return "Squared Loss";
            default:
                return "INVALID";
            }
        }

        template<typename ResponsePredType>
        void predict( const SampleListType &sampleIndexList,
                      const FeatureValueObjectType &valueObject,
                      ResponsePredType &scores,
                      int maxIters = -1,
                      bool normalize = false)   // if normalize => score is divided by sum(alphas)
        {
            const unsigned N = sampleIndexList.size();

            scores.resize( N );
            scores.setZero();

            unsigned numWL = mWeakLearners.size();
            if ( maxIters > 0 && maxIters <= numWL )
                numWL = maxIters;
            
            typedef PredictOperator<ResponsePredType, WeightsType> PredictOperatorType;

            #pragma omp parallel for
            for (unsigned i=0; i < numWL; i++)
            {
                mWeakLearners[i].tree.predictWithOperator( sampleIndexList, valueObject, PredictOperatorType( scores, mWeakLearners[i].alpha) );
            }

            if(normalize)
            {
                WeightsType sumAlphas = 0.0;
                for (unsigned i=0; i < numWL; i++)
                    sumAlphas += fabs(mWeakLearners[i].alpha);

                scores /= sumAlphas;
            }
        }

        // only keeps the first 'max' stumps, removes the others
        void cropWeakLearnersTo( unsigned max )
        {
            if ( max > mWeakLearners.size() )
                qFatal("Wanted to crop %d when there are only %d weak learners.", (int)max, (int) mWeakLearners.size() );

            const int toCrop = (int) mWeakLearners.size() - (int) max;

            for (int i=0; i < toCrop; i++)
                mWeakLearners.pop_back();
        }

        void    learn( const SampleListType &sampleIndexList,
                       const FeatureListType &featIndexList,
                       const FeatureValueObjectType &featValueObj,
                       const ClassifierResponseValueObjectType &classValueObject,
                       unsigned maxIter)
        {
            srand(mRandSeed);
            
            #ifdef MEX
                // timer to call drawnow
                TimerRT     drawnowTimer;
            #endif

            printf("\nShrinkage factor: %f\n", mShrinkageFactor);
            printf("Loss type: %s\n\n", getLossDescription(mLossType));

            const unsigned N = sampleIndexList.size();

            WeightsArrayType  curPred(N);
            curPred.setZero();

            // used to normalize the predictions to avoid numerical issues
            WeightsType curLoss = 1.0;

            WeightsArrayType  expYF(N);    // exp(-y*f)

            WeightsArrayType  W(N);    // fitting weights
            WeightsArrayType  R(N);    // fitting responses
            WeightsArrayType  score(N);

            // read classes in vector, this won't change and we will need it frequently
            WeightsArrayType Y(N);
            #warning tobefixed should be sampleIndexList[i] instead of i if it was really a proper implementation
            for (unsigned i=0; i < N; i++)
                Y.coeffRef(i) = classValueObject[ i ];


            for (unsigned iter=0; iter < maxIter; iter++)
            {
                TimerRT theTimer;

                WeightsType normFactor = 1.0 / curLoss;

                switch( mLossType )
                {
                    case ExpLoss:
                        expYF = ((-Y * curPred).exp() * normFactor) + std::numeric_limits<double>::epsilon();
                        W = expYF;
                        R = (-Y * expYF) / W;   // TODO: there is a simplification that can be done here!
                        break;

                    case LogLoss:
                        W.setConstant(1.0);
                        expYF = (- 2 * Y * curPred).exp();
                        R = (normFactor * Y * expYF) / ( 1 + expYF);
                        break;

                    case SquaredLoss:
                        W.setConstant(2);
                        R = ( Y - curPred );    // sign already inverted to go in gradient descent dir
                        break;
                }

    #if SHOW_TIMINGS
                std::cout << "Loss part: " << theTimer.elapsed() << std::endl;
    #endif

            #if 0
                if (mVerboseOutput)
                {
                    std::cout << "R0: " << R.coeff(0) << std::endl;
                    std::cout << "W0: " << W.coeff(0) << std::endl;

                    std::cout << "Max/Min R: " << R.maxCoeff() << " / " << R.minCoeff() << std::endl;
                    std::cout << "Max/Min W: " << W.maxCoeff() << " / " << W.minCoeff() << std::endl;
                }
            #endif

                for (unsigned i=0; i < N; i++)
                    if (std::isnan(R.coeff(i)) || std::isnan(W.coeff(i))) {
                        std::cout << "NaN at ExpYF = " << expYF.coeff(i) << std::endl;
                        qFatal("error");
                    }


                // fit regression tree
            #if SHOW_TIMINGS
                theTimer.Reset();
            #endif

                // subsampling, if set
                std::vector<unsigned> subSampIdxs;
                if (mResamplerObj != 0)
                    mResamplerObj->resample( Y, W, &subSampIdxs );
                    
                // re-scale W to avoid numerical problems
                W /= W.maxCoeff();
                R /= R.abs().maxCoeff();

                TreeResponseValueObjectType     treeValueObj(R, W);

                // but before learning, feature callback
                featIndexList.beforeLearnIterCallback();

                mWeakLearners.push_back( TreeBoosterWLType() );
                mWeakLearners.back().tree.learn( sampleIndexList, featIndexList, featValueObj, treeValueObj, mMaxTreeDepth, (mResamplerObj == 0) ? 0 : &subSampIdxs, mMTry );

                // predict values
            #if SHOW_TIMINGS
                std::cout << "Tree part: " << theTimer.elapsed() << std::endl;
                theTimer.Reset();
            #endif

                mWeakLearners.back().tree.predict( sampleIndexList, featValueObj, score );

            #if SHOW_TIMINGS
                std::cout << "Pred part: " << theTimer.elapsed() << std::endl;
            #endif

                if(mVerboseOutput)
                    qDebug("Max/Min score: %f / %f\n", score.maxCoeff(), score.minCoeff());

                // line search
            #if SHOW_TIMINGS
                theTimer.Reset();
            #endif

                double alpha = 1.0;
                if (!mDisableLineSearch)
                {
                    LineSearch<ResponseArrayType, ResponseArrayType> LS( curPred, score, Y, mLossType, mAlphaSearchOnSubset ? (&subSampIdxs) : 0 );
                    alpha = LS.run();
                } else
                    alpha = -1.0;	// because we want the -gradient direction

            #if SHOW_TIMINGS
                std::cout << "LS part: " << theTimer.elapsed() << std::endl;
            #endif

                if (fabs(alpha) < 1e-9)
                    alpha = -1e-9;

                // shrinkage
                alpha = mShrinkageFactor * alpha;
				if(mVerboseOutput)
					qDebug("Alpha %f\n", alpha);

                mWeakLearners.back().alpha = alpha;



                // update prediction score
                curPred += alpha * score;

                // show new loss
                switch(mLossType)
                {
                    case ExpLoss:
                        curLoss = expLoss( Y, curPred );
                        break;
                    case LogLoss:
                        curLoss = logLoss( Y, curPred );
                        break;
                    case SquaredLoss:
                        curLoss = squaredLoss( Y, curPred );
                        break;
                }

                qDebug("Iter %d / New loss: %f\n", iter, curLoss);

                // compute misclassif error
                double miscErr = 0;
                for (unsigned i=0; i < N; i++)
                    if ( curPred.coeff(i) * Y.coeff(i) < 0 )
                        miscErr++;
                miscErr /= N;

                if(mVerboseOutput)
                {
                    qDebug("Misclassif. error: %f\n", miscErr);
                    if (TimerRT::implemented())
                        qDebug("---> Total WL took: %f\n", theTimer.elapsed() );
                }
    #ifdef MEX
                if (drawnowTimer.elapsed() > 2.00)
                {
                    mexEvalString("drawnow");
                    drawnowTimer.Reset();
                }
    #endif

                /*if (fabs(alpha) < 1e-9)
                {
                    std::cout << "Breaking at iter " << iter << ", cannot go on!" << std::endl;
                    break;
                }*/
            }
        }

    public:
        unsigned numWeakLearners() const
        {
            return mWeakLearners.size();
        }

        void printOptionsSummary() const
        {
            printf("\nTreeboster options:\n");
            printf("\tLoss: %s\n", getLossDescription(mLossType));
            printf("\tShrinkage factor: %f\n", mShrinkageFactor);
            printf("\tMax tree depth: %d\n", (int)mMaxTreeDepth);
            if (mMTry != 0)
                printf("\tMTry: %d\n", mMTry);
            else
                printf("\tMTry: not set\n");
            printf("\tDisable line search: %s\n", mDisableLineSearch ? "true" : "false");
            printf("\n");
        }
        
        #ifdef LIBCONFIGXX_VER_REVISION
        // s must be a libconfig::Setting::TypeList
        void saveToLibconfig(libconfig::Setting &base) const
        {
            // save each weaklearner
            for (unsigned i=0; i < mWeakLearners.size(); i++)
            {
                libconfig::Setting &group = base.add(libconfig::Setting::TypeGroup);
                
                libconfig::Setting &tree = group.add("tree", libconfig::Setting::TypeList);
                libconfig::Setting &alpha = group.add("alpha", libconfig::Setting::TypeFloat);
                
                mWeakLearners[i].tree.saveToLibconfig( tree );
                alpha = mWeakLearners[i].alpha;
            }
        }
        
        // s must be a libconfig::Setting::TypeList
        void loadFromLibconfig( const libconfig::Setting &base )
        {
            const unsigned numNodes = base.getLength();
            
            mWeakLearners.clear();
            mWeakLearners.resize( numNodes );
            
            for (unsigned i=0; i < numNodes; i++)
            {
                const libconfig::Setting &group = base[i];
                
                mWeakLearners[i].tree.loadFromLibconfig( group["tree"] );
                mWeakLearners[i].alpha = (double) group["alpha"];
            }
        }
        #endif


    public:
        #ifdef MEX
        mxArray *saveToMatlab() const
        {
            const char *fieldNames[] = { "alpha", "tree" };
            mxArray *ret = mxCreateStructMatrix( 1, mWeakLearners.size(), 2, fieldNames );

            for (unsigned i=0; i < mWeakLearners.size(); i++)
            {
                mxSetField( ret, i, "tree", mWeakLearners[i].tree.saveToMatlab());

                {
                    mxArray *alpha = mxCreateNumericMatrix(1,1, matlabClassID<WeightsType>(), mxREAL);
                    ((WeightsType *)mxGetData(alpha))[0] = mWeakLearners[i].alpha;
                    mxSetField( ret, i, "alpha", alpha);
                }
            }

            return ret;
        }

        bool loadFromMatlab( const mxArray *data )
        {
            const unsigned numEl = mxGetNumberOfElements( data );
            if (numEl == 0)
            {
                printf("Number of elements in cell array cannot be zero");
                return false;
            }

            mWeakLearners.clear();
            mWeakLearners.resize( numEl );

            // NO ERROR CHECKING HERE, BEWARE!
            for (unsigned i=0; i < numEl; i++)
            {
                mWeakLearners[i].alpha = ((const WeightsType *)mxGetData( mxGetField( data, i, "alpha" ) ))[0];

                if (!mWeakLearners[i].tree.loadFromMatlab( mxGetField( data, i, "tree" ) ))
                    return false;
            }

            return true;
        }

        #endif
    };
}

#endif //_SQB_TREE_BOSTER_H
