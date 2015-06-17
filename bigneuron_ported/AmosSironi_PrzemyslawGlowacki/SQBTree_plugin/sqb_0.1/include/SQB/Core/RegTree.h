#ifndef _SQB_REGTREE_H_
#define _SQB_REGTREE_H_

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

#include <vector>
#include <Eigen/Dense>
#include <algorithm>
#include <iostream>
#include <limits>

#include <SQB/Core/Utils.h>

#include <libconfig.h++>

namespace SQB
{
    // FeatureIndexList specialized for a matrix/array (Eigen)
    template<typename TMatrix>
    struct MatrixFeatureIndexList
    {
        typedef TMatrix             MatrixType;
        typedef typename MatrixType::Scalar  ScalarType;

        // FeatureIndexType is a scalar (column)
        typedef unsigned int    FeatureIndexType;

    private:
        unsigned mNumCols;

    public:
        inline MatrixFeatureIndexList( const MatrixType &mat ) :
            mNumCols( mat.cols() ) {}

        /*inline FeatureIndexType operator[] (unsigned fIdx) const
        {
            return fIdx;
        }*/

        // this is a particular case where we can return the reference to the argument!
        inline const FeatureIndexType& operator[] (const unsigned &fIdx) const
        {
            return fIdx;
        }

        // this is called by TreeBooster before each iteration, in case something needs to be done
        inline void beforeLearnIterCallback() const {}

        inline unsigned int size() const {
            return mNumCols;
        }
    };

    // defines a list of samples to go through
    template<typename TMatrix>
    struct MatrixSampleIndexList
    {
        typedef TMatrix             MatrixType;

        // FeatureIndexType is a scalar (column)
        typedef unsigned int    SampleIndexType;

    private:
        unsigned mNumRows;

    public:
        inline MatrixSampleIndexList( const MatrixType &mat ) :
            mNumRows( mat.rows() ) {}

        // sample is just index
        inline const SampleIndexType& operator[] (const unsigned &sIdx) const
        {
            return sIdx;
        }

        inline unsigned int size() const {
            return mNumRows;
        }
    };

    // returns the weight and response of an object
    template<typename VectorType>
    struct MatrixResponseAndWeightsValueObject
    {
        typedef typename VectorType::Scalar  WeightScalarType;

        typedef unsigned int        FeatureIndexType;   // same as above
        typedef unsigned int        SampleIndexType;   // same as above

    private:
        const VectorType    &mResponse;
        const VectorType    &mWeights;

    public:
        inline MatrixResponseAndWeightsValueObject( const VectorType &response,
                                         const VectorType &weights) :
            mResponse(response), mWeights(weights) {}

        // returns sample weight
        inline WeightScalarType weight( const SampleIndexType &sIdx ) const
        {
            return mWeights.coeff( sIdx );
        }

        // response (Y) of a sample
        inline WeightScalarType response( const SampleIndexType &sIdx ) const
        {
            return mResponse.coeff( sIdx );
        }
    };

    // returns the value of a sample index, based on a vector
    template<typename VectorType>
    struct MatrixSingleResponseValueObject
    {
        typedef typename VectorType::Scalar  WeightScalarType;

        typedef unsigned int        FeatureIndexType;   // same as above
        typedef unsigned int        SampleIndexType;   // same as above

    private:
        const VectorType    &mResponse;

    public:
        inline MatrixSingleResponseValueObject( const VectorType &response ) :
            mResponse(response) {}

        // response (Y) of a sample
        inline WeightScalarType operator[]( const SampleIndexType &sIdx ) const
        {
            return mResponse.coeff( sIdx );
        }
    };

    // returns the value of a specific feature at a given sample index
    template<typename TMatrix>
    struct MatrixFeatureValueObject
    {
        typedef TMatrix             MatrixType;

        typedef typename MatrixType::Scalar  FeatureScalarType;

        typedef unsigned int        FeatureIndexType;   // same as above
        typedef unsigned int        SampleIndexType;   // same as above

    private:
        const MatrixType    &mMatrix;

    public:
        inline MatrixFeatureValueObject( const MatrixType &matrix ) :
            mMatrix(matrix) {}

        // returns feature value
        inline FeatureScalarType featureValue( const SampleIndexType &sIdx, const FeatureIndexType &fIdx ) const
        {
            return mMatrix.coeff( sIdx, fIdx );
        }
    };

    // will cache weights + responses. based on unsigned int indices, 0 .. N
    template<typename TFeatIndex, typename TSampleIndex, typename TRespValueObject >
    struct SampleResponseCache
    {
        typedef TFeatIndex  FeatureIndexListType;
        typedef TSampleIndex  SampleIndexListType;
        typedef TRespValueObject  ResponseValueObjectType;

        typedef typename SampleIndexListType::SampleIndexType   SampleIndexType;

        typedef typename ResponseValueObjectType::WeightScalarType  WeightScalarType;

        // matrix type for cache
        typedef typename Eigen::Array<WeightScalarType, Eigen::Dynamic, 1>  CacheResponseVectorType;

        // public access (should not modify them!)
        CacheResponseVectorType mCachedWeights;
        CacheResponseVectorType mCachedResponse;

        SampleResponseCache(  const SampleIndexListType &sIdxList,
                      const ResponseValueObjectType &valueObj )
        {
            const unsigned numSamples = sIdxList.size();

            mCachedWeights.resize(numSamples);
            mCachedResponse.resize(numSamples);

            // fill
            for (unsigned i=0; i < numSamples; i++)
            {
                mCachedWeights.coeffRef(i) = valueObj.weight(i);
                mCachedResponse.coeffRef(i) = valueObj.response(i);
            }
        }
    };

    /** FeatureCache computes the values of each feature for each sample
      *  to speed up computation
      */
    template<typename TFeatIndex, typename TSampleIndex, typename TValueObject >
    struct FeatureCache
    {
        typedef TFeatIndex  FeatureIndexListType;
        typedef TSampleIndex  SampleIndexListType;
        typedef TValueObject  FeatureValueObjectType;
        typedef typename FeatureValueObjectType::FeatureScalarType FeatureScalarType;

        // matrix type for cache
        typedef typename Eigen::Array<FeatureScalarType, Eigen::Dynamic, Eigen::Dynamic>  CacheMatrixType;

        // public access (should not modify them!)
        CacheMatrixType mCachedFeats;


        // if subIdxs != 0 => contains a subset of the indices
        //	to be used. Otherwise, all samples are computed
        inline FeatureCache( const FeatureIndexListType &fIndex,
                    const SampleIndexListType &sIdxList,
                    const FeatureValueObjectType &valueObj,
                    const std::vector<unsigned int> *subIdxs = 0 )
        {
            const unsigned numFeats = fIndex.size();
            const unsigned numSamples = sIdxList.size();

            mCachedFeats.resize( numSamples, numFeats );
            
            if (subIdxs == 0)
            {
                // fill
                #pragma omp parallel for
                for (unsigned i=0; i < numSamples; i++)
                {
                    for (unsigned f=0; f < numFeats; f++)
                    {
                        mCachedFeats.coeffRef(i,f) = valueObj.featureValue( sIdxList[i], fIndex[f] );
                    }
                }
            }
            else
            {
                const unsigned N = subIdxs->size();
                
                #pragma omp parallel for
                for (unsigned q=0; q < N; q++)
                {
                    const unsigned i = (*subIdxs)[q];
                    for (unsigned f=0; f < numFeats; f++)
                    {
                        mCachedFeats.coeffRef(i,f) = valueObj.featureValue( sIdxList[i], fIndex[f] );
                    }
                }
            }
        }
    };


    /* this represents a tree node (either split or leaf)
     *  TScalar: scalar type for threshold / value
     *  TFeatureIndex: feature index type, to be able
     *                   to identify the feature this node works on
     */
    template<typename TScalar, typename TFeatureIndex = unsigned int>
    struct RegTreeNode
    {
        typedef TScalar         ScalarType;
        typedef TFeatureIndex   FeatureIndexType;

        // if it is a leaf
        bool        isLeaf;
        TScalar		value;

        // otherwise
        TScalar		threshold;

        unsigned leftNodeIdx;
        unsigned rightNodeIdx;	// 'pointers' to right and left nodes

        TFeatureIndex featureIdx;		// variable index / pose index / etc

        bool used;

        RegTreeNode()
        {
            used = false;
        }

        // TODO: provide serialization!
    };

    // this is what is returned by the learnStump function
    template<typename T1, typename T2>
    struct RegStumpInfo
    {
        T1  threshold;
        T2  err;
        T2  y1, y2;

        bool isPure;

        RegStumpInfo()
        {
            isPure = false;
        }

        void printInfo()
        {
            std::cout << "------ Stump -----" << std::endl;
            std::cout << "Pure: " << isPure << std::endl;
            std::cout << "Threshold: " << threshold << std::endl;
            std::cout << "Y1: " << y1 << std::endl;
            std::cout << "Y2: " << y2 << std::endl;
            std::cout << "Err: " << err << std::endl;
        }
    };

    // a stack entry for a node to be 'learned'
    struct RegStackEntry
    {
        std::vector<unsigned>   idxList;    // which idxs to use for learning

        unsigned                nodeIdx;    // idx assigned to this node

        bool                    isLeaf;     // if this is a leaf

        double                  value;

        unsigned                treeLevel;  // keeps track of splitting level

        RegStackEntry()
        {
            isLeaf = false;
            treeLevel = 0;
        }
    };

    // used to sort indices
    template<typename T2>
    class SortMatrixByColumnOnlyIndex
    {
    private:
        const T2 &mData;
        const unsigned mCol;

    public:
        SortMatrixByColumnOnlyIndex( const T2 &data, unsigned int col ) : mData(data), mCol(col) {}

        inline bool operator()(unsigned int l, unsigned int r) const
        {
            return mData.coeff(l, mCol) < mData.coeff(r,mCol);
        }
    };
    
    // used to sort indices
    template<typename FeatureValueObjectType, typename SampleIndexListType>
    class SortFeature
    {
    private:
        const FeatureValueObjectType &mFeatValObj;
        const typename FeatureValueObjectType::FeatureIndexType mFeatIdx;
        const SampleIndexListType	&mSampleIndex;

    public:
        SortFeature( const FeatureValueObjectType &featValObj, 
                                    const typename FeatureValueObjectType::FeatureIndexType &featIdx,
                                    const SampleIndexListType &sampleIndex
                                ) 
            : mFeatValObj(featValObj), mFeatIdx(featIdx), mSampleIndex(sampleIndex) {}

        inline bool operator()(unsigned int l, unsigned int r) const
        {
            return mFeatValObj.featureValue(mSampleIndex[l], mFeatIdx) < mFeatValObj.featureValue(mSampleIndex[r],mFeatIdx);
        }
    };

    template<typename TSampleIndex, typename TFeatIndex, typename TFeatValueObject, typename TResponseValueObject >
    class RegTree
    {
    public:
        typedef TFeatIndex              FeatureIndexListType;
        typedef TSampleIndex            SampleIndexListType;
        typedef TFeatValueObject        FeatureValueObjectType;
        typedef TResponseValueObject    ResponseValueObjectType;

        typedef typename FeatureValueObjectType::FeatureScalarType   ScalarType;
        typedef typename ResponseValueObjectType::WeightScalarType   WeightsType;

        typedef typename FeatureIndexListType::FeatureIndexType FeatureIndexType;
        typedef typename SampleIndexListType::SampleIndexType   SampleIndexType;

        typedef RegTreeNode<ScalarType, FeatureIndexType>    TreeType;
        typedef std::vector<unsigned>   IdxListType;

    private:
        std::vector<TreeType>	mNodes;
		
	public:
		std::vector<TreeType>& nodes() { return mNodes; }

    public:

        unsigned numNodes() const {
            return mNodes.size();
        }

    public:
        // SERIALIZATION FOR LIBCONFIG
        #ifdef LIBCONFIGXX_VER_REVISION
        // s must be a libconfig::Setting::TypeList
        void saveToLibconfig(libconfig::Setting &s) const
        {
            libconfig::Setting &base = s;
            
            for (unsigned i=0; i < mNodes.size(); i++)
            {
                libconfig::Setting &child = base.add(libconfig::Setting::TypeGroup);
                
                child.add("isLeaf", libconfig::Setting::TypeBoolean) = mNodes[i].isLeaf;
                child.add("value", libconfig::Setting::TypeFloat) = mNodes[i].value;
                child.add("threshold", libconfig::Setting::TypeFloat) = mNodes[i].threshold;
                child.add("leftNodeIdx", libconfig::Setting::TypeInt64) = (long int) mNodes[i].leftNodeIdx;
                child.add("rightNodeIdx", libconfig::Setting::TypeInt64) = (long int) mNodes[i].rightNodeIdx;
                
//                #ifndef KERNELBOOST_SOURCE
//                    libconfig::Setting &featureIdxGroup = child.add("featureIdx", libconfig::Setting::TypeGroup);
//                    mNodes[i].featureIdx.saveToLibconfig(featureIdxGroup);
//                #else
                    child.add("featureIdx", libconfig::Setting::TypeInt64) = (long int) mNodes[i].featureIdx;
//                #endif
                
                child.add("used", libconfig::Setting::TypeBoolean) = mNodes[i].used;
            }
        }
        
        // s must be a libconfig::Setting::TypeList
        void loadFromLibconfig(const libconfig::Setting &s)
        {
            const libconfig::Setting &base = s;
            
            const unsigned numNodes = base.getLength();
            
            mNodes.clear();
            mNodes.resize(numNodes);
            
            for (unsigned i=0; i < numNodes; i++)
            {
                const libconfig::Setting &child = base[i];
                
                mNodes[i].isLeaf = (bool) child["isLeaf"];
                mNodes[i].value =  (ScalarType) child["value"];
                mNodes[i].threshold = (ScalarType) child["threshold"];
                mNodes[i].leftNodeIdx = (long int) child["leftNodeIdx"];
                mNodes[i].rightNodeIdx = (long int) child["rightNodeIdx"];
                
                mNodes[i].used = (bool)child["used"];
                
                #ifndef KERNELBOOST_SOURCE
                    const libconfig::Setting &featureIdxGroup = child["featureIdx"];
                    mNodes[i].featureIdx = (long int)child["featureIdx"];
                #else
                    mNodes[i].featureIdx.loadFromLibconfig(featureIdxGroup);
                #endif
            }
        }
        #endif
    
    
        // SERIALIZATION FOR MEX
        #ifdef MEX
        mxArray *saveToMatlab() const
        {
            const char *fieldNames[] = { "isLeaf", "value", "threshold", "leftNodeIdx", "rightNodeIdx", "varIdx", "used" };

            mwSize dims[1]; dims[0] = mNodes.size();

            // create array
            mxArray *ret = mxCreateStructArray(1, dims,
                                               7, fieldNames);
            for (unsigned i=0; i < mNodes.size(); i++)
            {
                {
                    mxArray *isLeaf = mxCreateNumericMatrix(1,1, mxUINT8_CLASS, mxREAL);
                    ((unsigned char *)mxGetData(isLeaf))[0] = mNodes[i].isLeaf?1:0;
                    mxSetField( ret, i, "isLeaf", isLeaf );
                }

                {
                    mxArray *value = mxCreateNumericMatrix(1,1, matlabClassID<ScalarType>(), mxREAL);
                    ((ScalarType *)mxGetData(value))[0] = mNodes[i].value;
                    mxSetField( ret, i, "value", value );
                }

                {
                    mxArray *thr = mxCreateNumericMatrix(1,1, matlabClassID<ScalarType>(), mxREAL);
                    ((ScalarType *)mxGetData(thr))[0] = mNodes[i].threshold;
                    mxSetField( ret, i, "threshold", thr );
                }

                {
                    mxArray *leftNodeIdx = mxCreateNumericMatrix(1,1, matlabClassID<unsigned>(), mxREAL);
                    ((unsigned int *)mxGetData(leftNodeIdx))[0] = mNodes[i].leftNodeIdx;
                    mxSetField( ret, i, "leftNodeIdx", leftNodeIdx );
                }

                {
                    mxArray *rightNodeIdx = mxCreateNumericMatrix(1,1, matlabClassID<unsigned>(), mxREAL);
                    ((unsigned int *)mxGetData(rightNodeIdx))[0] = mNodes[i].rightNodeIdx;
                    mxSetField( ret, i, "rightNodeIdx", rightNodeIdx );
                }

                {
                    mxArray *varIdx = mxCreateNumericMatrix(1,1, matlabClassID<unsigned>(), mxREAL);
                    ((unsigned int *)mxGetData(varIdx))[0] = mNodes[i].featureIdx;
                    mxSetField( ret, i, "varIdx", varIdx );
                }

                {
                    mxArray *used = mxCreateNumericMatrix(1,1, mxUINT8_CLASS, mxREAL);
                    ((unsigned char *)mxGetData(used))[0] = mNodes[i].used?1:0;
                    mxSetField( ret, i, "used", used );
                }
            }

            return ret;
        }

        bool loadFromMatlab( const mxArray *data )
        {
            const unsigned numNodes = mxGetNumberOfElements(data);
            mNodes.clear();
            mNodes.resize(numNodes);

            for (unsigned i=0; i < numNodes; i++)
            {
                {
                    mxArray *isLeaf = mxGetField(data, i, "isLeaf");
                    if (mxGetClassID(isLeaf) != mxUINT8_CLASS)
                    {
                        printf("isLeaf type mismatch");
                        return false;
                    }

                    mNodes[i].isLeaf = ((unsigned char *)mxGetData(isLeaf))[0] != 0;
                }

                {
                    mxArray *value = mxGetField(data, i, "value");
                    if (mxGetClassID(value) != matlabClassID<ScalarType>())
                    {
                        printf("value type mismatch");
                        return false;
                    }

                    mNodes[i].value = ((ScalarType *)mxGetData(value))[0];
                }

                {
                    mxArray *threshold = mxGetField(data, i, "threshold");
                    if (mxGetClassID(threshold) != matlabClassID<ScalarType>())
                    {
                        printf("threshold type mismatch");
                        return false;
                    }

                    mNodes[i].threshold = ((ScalarType *)mxGetData(threshold))[0];
                }

                {
                    mxArray *leftNodeIdx = mxGetField(data, i, "leftNodeIdx");
                    if (mxGetClassID(leftNodeIdx) != matlabClassID<unsigned>())
                    {
                        printf("leftNodeIdx type mismatch");
                        return false;
                    }

                    mNodes[i].leftNodeIdx = ((unsigned *)mxGetData(leftNodeIdx))[0];
                }

                {
                    mxArray *rightNodeIdx = mxGetField(data, i, "rightNodeIdx");
                    if (mxGetClassID(rightNodeIdx) != matlabClassID<unsigned>())
                    {
                        printf("rightNodeIdx type mismatch");
                        return false;
                    }

                    mNodes[i].rightNodeIdx = ((unsigned *)mxGetData(rightNodeIdx))[0];
                }

                {
                    mxArray *varIdx = mxGetField(data, i, "varIdx");
                    if (mxGetClassID(varIdx) != matlabClassID<unsigned>())
                    {
                        printf("varIdx type mismatch");
                        return false;
                    }

                    mNodes[i].featureIdx = ((unsigned *)mxGetData(varIdx))[0];
                }

                {
                    mxArray *used = mxGetField(data, i, "used");
                    if (mxGetClassID(used) != mxUINT8_CLASS)
                    {
                        printf("used type mismatch");
                        return false;
                    }

                    mNodes[i].used = ((unsigned char *)mxGetData(used))[0] != 0;
                }
            }

            return true;
        }

        #endif

    public:

        void    printInfo()
        {
            for (unsigned i=0; i < mNodes.size(); i++)
            {
                if ( mNodes[i].isLeaf )
                {
                    std::cout << "LEAF at " << i << std::endl;
                    std::cout << "Value: " << mNodes[i].value << std::endl;
                }
                else
                {
                    std::cout << "SPLIT at " << i << std::endl;
                    std::cout << "Var: " << mNodes[i].varIdx << std::endl;
                    std::cout << "Thr: " << mNodes[i].threshold << std::endl;
                    std::cout << "<  : " << mNodes[i].leftNodeIdx << std::endl;
                    std::cout << ">= : " << mNodes[i].rightNodeIdx << std::endl;
                }
                std::cout << std::endl;
            }
        }
        
        template<typename T2>
        void	predict( const SampleIndexListType &sampIdxs, const FeatureValueObjectType &featValObj,
                         Eigen::Array<T2, Eigen::Dynamic, 1> &pred )
        {
            const unsigned N = sampIdxs.size();
            pred.resize( N );

            #pragma omp parallel for schedule(dynamic)
            for (unsigned i=0; i < N; i++)
            {
                const SampleIndexType &sampIdx = sampIdxs[i];

                // start going through the tree
                unsigned curNode = 0;
                while(true)
                {
                    // then just finish
                    if ( mNodes[curNode].isLeaf )
                    {
                        pred.coeffRef(i) = mNodes[curNode].value;
                        break;
                    }

                    ScalarType featValue = featValObj.featureValue( sampIdx, mNodes[curNode].featureIdx );
                    if ( featValue < mNodes[curNode].threshold )
                        curNode = mNodes[curNode].leftNodeIdx;
                    else
                        curNode = mNodes[curNode].rightNodeIdx;
                }
            }
        }
        
        template<typename OperatorType>
        void    predictWithOperator( const SampleIndexListType &sampIdxs, const FeatureValueObjectType &featValObj,
                         const OperatorType &op )
        {
            const unsigned N = sampIdxs.size();

            //op.init( D, N );

            for (unsigned i=0; i < N; i++)
            {
                const SampleIndexType &sampIdx = sampIdxs[i];

                // start going through the tree
                unsigned curNode = 0;
                while(true)
                {
                    // then just finish
                    if ( mNodes[curNode].isLeaf )
                    {
                        op.apply( i, mNodes[curNode].value );
                        break;
                    }

                    ScalarType featValue = featValObj.featureValue( sampIdx, mNodes[curNode].featureIdx );
                    if ( featValue < mNodes[curNode].threshold )
                        curNode = mNodes[curNode].leftNodeIdx;
                    else
                        curNode = mNodes[curNode].rightNodeIdx;
                }
            }
        }
        
#define REGTREE_USE_CACHE_WHEN_LEARNING 0

        // if beginSampleIdxList != 0 => not all samples are used for training,
        //  only the ones given in the idx list
        void	learn( const SampleIndexListType &sampIdxs, const FeatureIndexListType &featIdxs, const FeatureValueObjectType &featValObj,
                       const ResponseValueObjectType &respValObj,
                       unsigned maxDepth, std::vector<unsigned> *beginSampleIdxList = 0,
                    unsigned mtry = 0	// similar to RFs, if == 0 then all variables are used
                    )
        {
            mNodes.clear();
            mNodes.reserve(100);

            unsigned curNodeIdx = 0;

            // num of features
            const unsigned numFeats = featIdxs.size();
            const unsigned numSamp  = sampIdxs.size();

            std::vector< RegStackEntry >    stack;

            // push first element: all the data
            {
                RegStackEntry   all;

                if (beginSampleIdxList == 0)
                {
                    all.idxList.resize( numSamp );
                    for (unsigned i=0; i < all.idxList.size(); i++)
                        all.idxList[i] = i;
                } else
                    all.idxList = *beginSampleIdxList;  // copy

                all.nodeIdx = curNodeIdx++;    // first node

                all.isLeaf = false;
                all.treeLevel = 0;

                stack.push_back(all);
            }

        #if REGTREE_USE_CACHE_WHEN_LEARNING
            // pre-cache weights + responses
            SampleResponseCache<FeatureIndexListType, SampleIndexListType, ResponseValueObjectType> sampleCache( sampIdxs, respValObj );

            // pre-cache ALL features ( TODO: could result in mem bloat! )
            qDebug("Caching features");
            TimerRT featCacheTimer;
            FeatureCache<FeatureIndexListType, SampleIndexListType, FeatureValueObjectType> fCache( featIdxs, sampIdxs, featValObj, beginSampleIdxList );
            qDebug("End. Caching features took: %e", featCacheTimer.elapsed());
        #endif


            //////////////////////////////////////////////////////
            while( !stack.empty() )
            {
                RegStackEntry top = stack.back();
                stack.pop_back();

                //std::cout << "N: " << top.idxList.size() << std::endl;

                // resize if necessary
                if (mNodes.size() < top.nodeIdx + 1)
                    mNodes.resize(2*top.nodeIdx + 1);

                if ( top.isLeaf )
                {
                    mNodes[top.nodeIdx].isLeaf = true;
                    mNodes[top.nodeIdx].value = top.value;
                    mNodes[top.nodeIdx].used = true;

                    continue;
                }


                // prealloc results
                std::vector< RegStumpInfo<ScalarType,WeightsType> > stumpResults( numFeats );
                
                WeightsType  minErr = std::numeric_limits<WeightsType>::max();
                RegStumpInfo<ScalarType,WeightsType> minStump;
                unsigned minCol = 0;
                
                TimerRT theTimer;
                
                if ((mtry == 0) || (mtry >= numFeats))
                {
                    // go through each var and find the lowest error
                    #pragma omp parallel for schedule(dynamic)
                    for (unsigned col=0; col < numFeats; col++)
                    {
                        //TimerRT timer;
                        #if REGTREE_USE_CACHE_WHEN_LEARNING
                            learnSingleStump<ScalarType, WeightsType, Eigen::Dynamic>( fCache.mCachedFeats, sampleCache.mCachedResponse,
                                            sampleCache.mCachedWeights,
                                            top.idxList, col, &stumpResults[col] );
                        #else
                            learnSingleStump<ScalarType, WeightsType, Eigen::Dynamic>( 
                                            sampIdxs, featValObj, respValObj,
                                            top.idxList, col, &stumpResults[col] );
                        #endif
                        //if (col == 0)
                        //	qDebug("El: %e\n", timer.elapsed());
                    }
                    
                    // find minimum
                    for (unsigned col=0; col < numFeats; col++)
                    {
                        if (minErr > stumpResults[col].err)
                        {
                            minStump = stumpResults[col];
                            minErr = stumpResults[col].err;
                            minCol = col;
                        }
                    }
                } 
                else
                {
                    // mtry, this is not a very efficient version, but it is only for test purposes
                    std::vector<unsigned int>	mtryIdxs(numFeats);
                    for (unsigned e=0; e < numFeats; e++)	mtryIdxs[e] = e;
                    std::random_shuffle( mtryIdxs.begin(), mtryIdxs.end() );
                    
                    #pragma omp parallel for schedule(dynamic)
                    for (unsigned colOffset=0; colOffset < mtry; colOffset++)
                    {
                        const unsigned col = mtryIdxs[colOffset];
                        
                        //TimerRT timer;
                        #if REGTREE_USE_CACHE_WHEN_LEARNING
                            learnSingleStump<ScalarType, WeightsType, Eigen::Dynamic>( fCache.mCachedFeats, sampleCache.mCachedResponse,
                                            sampleCache.mCachedWeights,
                                            top.idxList, col, &stumpResults[col] );
                        #else
                            learnSingleStump<ScalarType, WeightsType, Eigen::Dynamic>( 
                                            sampIdxs, featValObj, respValObj,
                                            top.idxList, col, &stumpResults[col] );
                        #endif
                        //if (col == 0)
                        //	qDebug("El: %e\n", timer.elapsed());
                    }
                    
                    // find minimum
                    for (unsigned colOffset=0; colOffset < mtry; colOffset++)
                    {
                        const unsigned col = mtryIdxs[colOffset];
                        
                        if (minErr > stumpResults[col].err)
                        {
                            minStump = stumpResults[col];
                            minErr = stumpResults[col].err;
                            minCol = col;
                        }
                    }
                }
                
            #if SHOW_TIMINGS
                std::cout << "WL search: " << theTimer.elapsed() << std::endl;
            #endif

                //minStump.printInfo();
                //std::cout << "Min col " << minCol << std::endl;

                if ( minStump.isPure )
                {
                    mNodes[top.nodeIdx].isLeaf = true;
                    mNodes[top.nodeIdx].value = minStump.y2;
                    mNodes[top.nodeIdx].used = true;

                    continue;
                }

                mNodes[top.nodeIdx].featureIdx = featIdxs[minCol];
                mNodes[top.nodeIdx].isLeaf = false;
                mNodes[top.nodeIdx].threshold = minStump.threshold;

                mNodes[top.nodeIdx].leftNodeIdx = curNodeIdx++;
                mNodes[top.nodeIdx].rightNodeIdx = curNodeIdx++;


                // create child stacks
                RegStackEntry   leftEntry, rightEntry;
                for (unsigned i=0; i < top.idxList.size(); i++)
                {
                    #if REGTREE_USE_CACHE_WHEN_LEARNING
                    if ( fCache.mCachedFeats.coeff( top.idxList[i], minCol ) < minStump.threshold )
                    #else
                    if ( featValObj.featureValue( sampIdxs[top.idxList[i]], minCol ) < minStump.threshold )
                    #endif
                        leftEntry.idxList.push_back( top.idxList[i] );
                    else
                        rightEntry.idxList.push_back( top.idxList[i] );
                }

                //std::cout << "Left:  " << leftEntry.idxList.size() << std::endl;
                //std::cout << "Right: " << rightEntry.idxList.size() << std::endl;

                leftEntry.treeLevel = rightEntry.treeLevel = top.treeLevel + 1;

                bool leftTerminate = (leftEntry.treeLevel > maxDepth) || ( leftEntry.idxList.size() < 2 );
                bool rightTerminate = (rightEntry.treeLevel > maxDepth) || ( rightEntry.idxList.size() < 2 );

                if ( leftTerminate )
                {
                    leftEntry.isLeaf = true;
                    leftEntry.value = minStump.y1;
                } else
                    leftEntry.isLeaf = false;

                if ( rightTerminate )
                {
                    rightEntry.isLeaf = true;
                    rightEntry.value = minStump.y2;
                } else
                    rightEntry.isLeaf = false;


                rightEntry.nodeIdx = mNodes[top.nodeIdx].rightNodeIdx;
                leftEntry.nodeIdx = mNodes[top.nodeIdx].leftNodeIdx;

                stack.push_back(rightEntry);
                stack.push_back(leftEntry);
            }

            // find out which are terminal nodes
            unsigned numTermNodes = 0;
            for (unsigned q=0; q < curNodeIdx; q++)
            {
                if (mNodes[q].isLeaf)   numTermNodes++;
            }

            std::cout << "Learned " << curNodeIdx << " nodes, " << numTermNodes << " terminal nodes." << std::endl;

            if (mNodes.size() != curNodeIdx)
                mNodes.erase( mNodes.begin() + mNodes.size(), mNodes.end() );
        }

        // ---- NON CACHE VERSION
#if !REGTREE_USE_CACHE_WHEN_LEARNING
        template<typename T1, typename T2, unsigned int numFeats>
        void learnSingleStump( const SampleIndexListType &sampIdxs, 
                            const FeatureValueObjectType &featValObj,
                            const ResponseValueObjectType &respValObj,
                            const IdxListType    &idxs,  // which samples to classify
                            FeatureIndexType fIdx,
                               RegStumpInfo<T1,T2> *stumpInfo )
        {
            typedef Eigen::Array<T2, Eigen::Dynamic, 1> WeightTypeArray;


            const unsigned N = idxs.size();

            // check if we cannot do anything for this region
            bool isPure = true;
            {
                T1 v1 = featValObj.featureValue( sampIdxs[idxs[0]], fIdx );
                for (unsigned i=0; i < idxs.size(); i++)
                {
                    if ( featValObj.featureValue(sampIdxs[idxs[i]], fIdx) != v1 )
                    {
                        isPure = false;
                        break;
                    }
                }
            }
            
            // sort indices
            IdxListType sortedIdx = idxs;   // just copy now

            std::sort( sortedIdx.begin(), sortedIdx.end(),
                    SortFeature<FeatureValueObjectType, SampleIndexListType>( featValObj, 
                                    fIdx,
                                    sampIdxs ) );


            T2 sumWk1 = 0;
            T2 sumWk2 = 0;
            
            T2 sumWkRk2 = 0;
            T2 sumWkRkSq2 = 0;


            for (unsigned i=0; i < N; i++)
            {
                const unsigned qidx = sortedIdx[i];

                //std::cout << "Sorted: " << qidx << std::endl;

                const T2 w = respValObj.weight(sampIdxs[qidx]);
                const T2 r = respValObj.response(sampIdxs[qidx]);

                const T2 rw = r*w;

                sumWkRk2 += rw;
                sumWkRkSq2 += rw * r;

                sumWk2 += w;
            }

            T2 sumWkRk1 = 0;

            T2 sumWkRkSq1 = 0;

            T2 y1 = 0;
            T2 y2 = sumWkRk2 / (sumWk2 + 10*std::numeric_limits<T2>::epsilon());

            T2 err = 0;

            {
                T2 err1 = y1 * y1 * sumWk1 + sumWkRkSq1 - 2*y1*sumWkRk1;
                T2 err2 = y2 * y2 * sumWk2 + sumWkRkSq2 - 2*y2*sumWkRk2;

                err = err1 + err2;
            }

            //std::cout << "ERR: " << err << std::endl;
            if ( (err < N * std::numeric_limits<T2>::epsilon()) || isPure )
            {
                // then region is pure!
                stumpInfo->isPure = true;
                stumpInfo->y1 = stumpInfo->y2 = y2;
                stumpInfo->err = err;
                return;
            }

            T1 thr = featValObj.featureValue(sampIdxs[sortedIdx[0]], fIdx);

            T2 minErr = err;
            T1 minThr =thr;
            T2 minY1 = y1;
            T2 minY2 = y2;

            T1 prevVal = thr;
            unsigned minIdx = 0;
            

            for (unsigned i=0; i < N - 1; i++)
            {
                const T2 w = respValObj.weight(sampIdxs[sortedIdx[i]]);
                const T2 r = respValObj.response(sampIdxs[sortedIdx[i]]);
                //W.coeff( sortedIdx[i]  );
                
                const T2 rw = r*w;
                const T2 r2w = r*rw;

                // recompute sums
                sumWk1 += w;
                sumWk2 -= w;

                sumWkRk1 += rw;
                sumWkRk2 -= rw;

                sumWkRkSq1 += r2w;
                sumWkRkSq2 -= r2w;

                thr = featValObj.featureValue(sampIdxs[sortedIdx[i+1]], fIdx);
                //X.coeff( sortedIdx[i+1], column  );

                if (prevVal == thr)
                    continue;

                prevVal = thr;

                // find Y
                y1 = sumWkRk1 / (sumWk1 + 10*std::numeric_limits<T2>::epsilon());
                y2 = sumWkRk2 / (sumWk2 + 10*std::numeric_limits<T2>::epsilon());

                T2 err1 = y1 * y1 * sumWk1 + sumWkRkSq1 - 2*y1*sumWkRk1;
                T2 err2 = y2 * y2 * sumWk2 + sumWkRkSq2 - 2*y2*sumWkRk2;

                err = err1 + err2;

                ///std::cout << "Err: " << "  Thr: " << thr << err << "  y1: " << y1 << "  y2: " << y2 << std::endl;

             //   std::cout << "y1: " << y1 << std::endl;
            //    std::cout << "y2: " << y2 << std::endl;

                /*cE = checkErr(X,R,W, X(I+1), y1, y2);

                if (abs(cE - err) > 1e-2)
                    fprintf('%f\n', abs(cE - err));
                end*/

                if (minErr > err)
                {
                    minErr = err;
                    minThr = thr;
                    minIdx = i;
                    minY1 = y1;
                    minY2 = y2;
                }
            }
            

            // search back for the previous threshold to set it in the middle
            T2 prevThr = minThr;
            for (int i=(int)minIdx; i >= 0; i--)
            {
                if ( featValObj.featureValue(sampIdxs[sortedIdx[i]], fIdx) != minThr )
                {
                    prevThr = featValObj.featureValue(sampIdxs[sortedIdx[i]], fIdx);
                    //X.coeff( sortedIdx[i], column );
                    break;
                }
            }

            stumpInfo->threshold = (minThr + prevThr)/2;
            stumpInfo->err = minErr;
            stumpInfo->y1 = minY1;
            stumpInfo->y2 = minY2;
        }
#else	
        // CACHE VERSION
        template<typename T1, typename T2, unsigned int numFeats>
        void learnSingleStump( const Eigen::Array<T1, Eigen::Dynamic, numFeats> &X,
                               const Eigen::Array<T2, Eigen::Dynamic, 1> &R,
                               const Eigen::Array<T2, Eigen::Dynamic, 1> &W,
                               const IdxListType    &idxs,  // which samples to classify
                               unsigned int column,
                               RegStumpInfo<T1,T2> *stumpInfo )
        {
            typedef Eigen::Array<T2, Eigen::Dynamic, 1> WeightTypeArray;


            const unsigned N = idxs.size();

            // check if we cannot do anything for this region
            bool isPure = true;
            {
                T1 v1 = X.coeff( idxs[0], column );
                for (unsigned i=0; i < idxs.size(); i++)
                {
                    if ( X.coeff(idxs[i], column) != v1 )
                    {
                        isPure = false;
                        break;
                    }
                }
            }
            
            // sort indices
            IdxListType sortedIdx = idxs;   // just copy now

            std::sort( sortedIdx.begin(), sortedIdx.end(),
                       SortMatrixByColumnOnlyIndex<Eigen::Array<T1, Eigen::Dynamic, Eigen::Dynamic> >( X, column ) );


            // pre-cache some products
            WeightTypeArray pWR(N), pWR2(N);    // weights * resp

            T2 sumWk1 = 0;
            T2 sumWk2 = 0;

            for (unsigned i=0; i < N; i++)
            {
                const unsigned qidx = sortedIdx[i];

                //std::cout << "Sorted: " << qidx << std::endl;

                const T2 w = W.coeff(qidx);
                const T2 r = R.coeff(qidx);

                const T2 rw = r*w;

                pWR.coeffRef(i) = rw;
                pWR2.coeffRef(i) = rw * r; // R^2

                sumWk2 += w;
            }

            T2 sumWkRk1 = 0;
            T2 sumWkRk2 = pWR.sum();

            T2 sumWkRkSq1 = 0;
            T2 sumWkRkSq2 = pWR2.sum();

            T2 y1 = 0;
            T2 y2 = sumWkRk2 / (sumWk2 + 10*std::numeric_limits<T2>::epsilon());

            T2 err = 0;

            {
                T2 err1 = y1 * y1 * sumWk1 + sumWkRkSq1 - 2*y1*sumWkRk1;
                T2 err2 = y2 * y2 * sumWk2 + sumWkRkSq2 - 2*y2*sumWkRk2;

                err = err1 + err2;
            }

            //std::cout << "ERR: " << err << std::endl;
            if ( (err < N * std::numeric_limits<T2>::epsilon()) || isPure )
            {
                // then region is pure!
                stumpInfo->isPure = true;
                stumpInfo->y1 = stumpInfo->y2 = y2;
                stumpInfo->err = err;
                return;
            }

            T1 thr = X.coeff( sortedIdx[0], column  );

            T2 minErr = err;
            T1 minThr =thr;
            T2 minY1 = y1;
            T2 minY2 = y2;

            T1 prevVal = thr;
            unsigned minIdx = 0;
            

            for (unsigned i=0; i < N - 1; i++)
            {
                const T2 w = W.coeff( sortedIdx[i]  );

                // recompute sums
                sumWk1 += w;
                sumWk2 -= w;

                sumWkRk1 += pWR.coeff(i);
                sumWkRk2 -= pWR.coeff(i);

                sumWkRkSq1 += pWR2.coeff(i);
                sumWkRkSq2 -= pWR2.coeff(i);

                thr = X.coeff( sortedIdx[i+1], column  );

                if (prevVal == thr)
                    continue;

                prevVal = thr;

                // find Y
                y1 = sumWkRk1 / (sumWk1 + 10*std::numeric_limits<T2>::epsilon());
                y2 = sumWkRk2 / (sumWk2 + 10*std::numeric_limits<T2>::epsilon());

                T2 err1 = y1 * y1 * sumWk1 + sumWkRkSq1 - 2*y1*sumWkRk1;
                T2 err2 = y2 * y2 * sumWk2 + sumWkRkSq2 - 2*y2*sumWkRk2;

                err = err1 + err2;

                ///std::cout << "Err: " << "  Thr: " << thr << err << "  y1: " << y1 << "  y2: " << y2 << std::endl;

                //std::cout << "y1: " << y1 << std::endl;
                //std::cout << "y2: " << y2 << std::endl;

                /*cE = checkErr(X,R,W, X(I+1), y1, y2);

                if (abs(cE - err) > 1e-2)
                    fprintf('%f\n', abs(cE - err));
                end*/

                if (minErr > err)
                {
                    minErr = err;
                    minThr = thr;
                    minIdx = i;
                    minY1 = y1;
                    minY2 = y2;
                }
            }
            

            // search back for the previous threshold to set it in the middle
            T2 prevThr = minThr;
            for (int i=(int)minIdx; i >= 0; i--)
            {
                if ( X.coeff( sortedIdx[i], column ) != minThr )
                {
                    prevThr = X.coeff( sortedIdx[i], column );
                    break;
                }
            }

            stumpInfo->threshold = (minThr + prevThr)/2;
            stumpInfo->err = minErr;
            stumpInfo->y1 = minY1;
            stumpInfo->y2 = minY2;
        }
#endif

        // TODO: provide serialization
    };
}

#endif
