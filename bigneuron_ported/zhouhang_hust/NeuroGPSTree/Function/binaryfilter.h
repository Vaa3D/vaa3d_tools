#ifndef BINARYFILTER_H
#define BINARYFILTER_H

#include "ineuronprocessobject.h"
#include "../ngtypes/basetypes.h"
class BinaryFilter;
typedef std::shared_ptr<BinaryFilter> NGBinaryFilter;
class BinaryFilter : public INeuronProcessObject
{
public:
    typedef std::shared_ptr<VectorVec3i> BinPtSetPointer;
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
private:
    bool Binary();
    int threadNum ;
    double binThreshold ;
    //m_Source binary image
    DataPointer m_Back;//backnoise
    BinPtSetPointer m_BinPtSet;
};

#endif // BINARYFILTER_H
