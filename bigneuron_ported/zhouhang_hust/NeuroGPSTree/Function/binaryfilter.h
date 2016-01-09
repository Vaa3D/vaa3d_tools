/*
 * Copyright (c)2013-2015  Zhou Hang, Shaoqun Zeng, Tingwei Quan
 * Britton Chance Center for Biomedical Photonics, Huazhong University of Science and Technology
 * All rights reserved.
 */
#ifndef BINARYFILTER_H
#define BINARYFILTER_H

#include "ineuronprocessobject.h"
#include "../ngtypes/basetypes.h"
class BinaryFilter;
#ifdef _WIN32
typedef std::tr1::shared_ptr<BinaryFilter> NGBinaryFilter;
#else
typedef std::shared_ptr<BinaryFilter> NGBinaryFilter;
#endif
class BinaryFilter : public INeuronProcessObject
{
public:
#ifdef _WIN32
    typedef std::tr1::shared_ptr<VectorVec3i> BinPtSetPointer;
#else
    typedef std::shared_ptr<VectorVec3i> BinPtSetPointer;
#endif
    static NGBinaryFilter New(){return NGBinaryFilter(new BinaryFilter());}
    BinaryFilter();
    ~BinaryFilter();
    bool Update();
    ConstDataPointer GetOutput();
    ConstDataPointer GetBackNoiseImage();
    BinPtSetPointer GetBinPtSet();
    DataPointer ReleaseData();
    DataPointer ReleaseBackNoiseImage();
    BinPtSetPointer ReleaseBinPtSet();
    void SetThreadNum(int);
    void SetThreshold(double);
    void SetThreValue(double);//2015-8-13
private:
    bool Binary();
    int threadNum ;
    double binThreshold ;
    double threValue;
    //m_Source binary image
    DataPointer m_Back;//backnoise
    BinPtSetPointer m_BinPtSet;
};

#endif // BINARYFILTER_H
