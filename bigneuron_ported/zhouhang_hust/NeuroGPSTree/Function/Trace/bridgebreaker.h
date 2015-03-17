#ifndef BRIDGEBREAKER_H
#define BRIDGEBREAKER_H
#include "../ineuronprocessobject.h"
#include <vector>
#include "../../ngtypes/basetypes.h"

template<typename T> class Volume;
//typedef Volume<unsigned short> CVolume;
typedef std::shared_ptr<Volume<unsigned short>> SVolumePointer;
typedef std::shared_ptr<const Volume<unsigned short>> CSVolumePointer;
class Tree;
class BridgeBreaker;
typedef std::shared_ptr<BridgeBreaker> NGBridgeBreaker;

class BridgeBreaker : public INeuronProcessObject
{
public:
    static NGBridgeBreaker New(){return NGBridgeBreaker(new BridgeBreaker());}
    BridgeBreaker();
    ~BridgeBreaker();

    bool Update();
    ConstDataPointer GetOutput();
    DataPointer ReleaseData();

    void SetInputOrigImage(ConstDataPointer arg){m_OrigImg = arg;}
    void SetInputTree(ConstDataPointer arg){m_Tree = arg;}
	//2015-3-3 add work status
	void SetInputSoma(ConstDataPointer arg){m_Soma = arg;}

    VectorVec4d getResultIndex() const;
    void setResultIndex(const VectorVec4d &value);

private:
    //varient
    ConstDataPointer m_OrigImg;
    ConstDataPointer m_Tree;
    VectorVec4d resultIndex;
	ConstDataPointer m_Soma;//2015-3-3
    //function
    //tublartotalffdirjudge
    bool DeleteErrorDendConnect(const std::vector<VectorVec5d > &rawLine, const VectorMat2i &rawConnect,
                                  const Volume<unsigned short> &origImg,
                                  VectorMat2i &resultConnect, VectorVec4d &resultIndex);

    void GetBoundaryBack(const std::vector<double> &outer_shell,
                         const double threv,
                         std::vector<double> &boundary_back);

    void GetRayLimit(const Volume<double> &sphere_ray_wet,
                     const double constriction_threv,
                     std::vector<std::vector<double> > &ray_limit);

    void GetGradientVectorFlowForTrace(const Volume<double> &sphere_ray_wet, Volume<double> &smooth_ray);

    void SmoothGradientCurvesForTrace(const std::vector<double> &init_one_ray_wet,
                                    std::vector<double> &smooth_one_ray_wet);

    void CalculateOneRayLimit(const std::vector<double> &ray,
                              const double constriction_threv,
                              int &one_ray_limit);

    void CalcRayBurstRadWet(const VectorVec5d& curLine, const Volume<unsigned short> &origImg,
                        MatXd& RR0, MatXd &RR1 );
    void CalcRayBurstOnePtRadWet(const Vec3d& curLinePt, const Volume<unsigned short> &origImg,
                        const Vec3d& mainDirection1, Vec2d &aa1 );//RayburstRadius1

    void JudgeHeadTailForDendCurve(const MatXd& RR0, const MatXd &RR1, const Vec2d &connectStat,
                              int &parentStat);

    void KnotAnalysis(const VectorVec5d &currentDendCurve, const VectorVec5d &conjDendCurve,
                      int directionFlag, double angleThrev, int headConjHeadTailFlag,
                      int &mm);
    void CalcConnectKnotLocalDirection(const VectorVec5d &adjecentLine, const Vec5d &curLineNode, int knotStat,
                          Vec3d& resultDirection);

    void Principald(const VectorVec3d &dataL, Vec3d &x1);

    void CalcOrthoBasis(const Vec3d &vec1, Vec3d &vec2, Vec3d &vec3);

    //void WeighRayValue(const VectorVec3d &dataL1, const Volume<unsigned char> &L_XX3, std::vector<double> &aa_data);

    void CalcPrinDirAndDistList(const VectorVec3d& ptLine, Vec3d& mainDirection);


    void WeighRayValue(const VectorVec3d &rayNode, const Volume<unsigned short> &locOrigImg, std::vector<double> &rayNodeWet);
};

#endif // BRIDGEBREAKER_H
