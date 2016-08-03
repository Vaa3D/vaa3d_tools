/*
 * Copyright (c)2013-2015  Zhou Hang, Shaoqun Zeng, Tingwei Quan
 * Britton Chance Center for Biomedical Photonics, Huazhong University of Science and Technology
 * All rights reserved.
 */
#ifndef TREEWRITER_H
#define TREEWRITER_H
#include "../ineuronio.h"
#include "../../ngtypes/basetypes.h"
#include "../../ngtypes/tree.h"
class TreeWriter;
#ifdef _WIN32
typedef std::tr1::shared_ptr<TreeWriter> NGTreeWriter;
#else
typedef std::shared_ptr<TreeWriter> NGTreeWriter;
#endif
class TreeWriter : public INeuronIO
{
public:
    static NGTreeWriter New(){return NGTreeWriter(new TreeWriter());}
    TreeWriter();
    ~TreeWriter();
    void SetInputXYZExtend(double x,double y,double z){xExtend_= x; yExtend_ = y; zExtend_ = z;}
    void SetOutputFileName(const std::string &);
    bool Update();
protected:
    struct swcElementStruct{int id, type, pid, globalId; Vec4d node;};

    void GetDendConInfoFromDendList(const std::vector<VectorVec5d>& dendList, double distThrev, std::vector<std::vector<std::vector<size_t> > >& denConInfo);

    double CalcMinDistFromPtToCurve(const Vec5d& pt, const VectorVec5d& curve, size_t &minIndex);

    double CalcMinDistFromPtToCurve( const Vec3d& pt, const VectorVec5d& curve, size_t &minIndex );

    void GetTreeLevelFromConInfo(const std::vector<std::vector<std::vector<size_t> > >& denConInfo, int somaIndex, std::vector<size_t> &parentNodeList,
        std::vector<std::vector<size_t> >& sonNodeList, std::vector<std::vector<size_t> > &treeLevel);

    void GetWholeConnectSWC(const std::vector<VectorVec5d>& dendList, const std::vector<size_t> &parentNodeList,
        const std::vector<std::vector<size_t> >& sonNodeList, const std::vector<std::vector<size_t> > &treeLevel, 
        std::vector<std::vector<std::vector<size_t> > >& dendConInfo, 
        double distThrev,
        std::vector<swcElementStruct>& resultSwc);

    bool WriteOneSwc(const std::string& curFileName, const std::vector<swcElementStruct>& resultSwc, int type);

    bool isMember(size_t ind, const std::vector<size_t>& searchRange);

    void FindSwc8thRange(size_t ind, const std::vector<swcElementStruct>& resultSwc, Vec2i& range);

    DataPointer ReleaseData(){return DataPointer(new TreeCurve());}
    void SetInputFileName(const std::string &){}
    ConstDataPointer GetOutput(){return m_Source;}
    
private:
    std::string fileName;
    double xExtend_, yExtend_, zExtend_;
};

#endif // TREEWRITER_H
