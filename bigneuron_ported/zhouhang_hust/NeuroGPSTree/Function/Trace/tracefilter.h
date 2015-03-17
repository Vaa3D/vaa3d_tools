#ifndef TRACEFILTER_H
#define TRACEFILTER_H
#include "../ineuronprocessobject.h"
#include "../../ngtypes/basetypes.h"
template<typename T> class Volume;
class Tree;
class TraceFilter;
typedef std::shared_ptr<TraceFilter> NGTraceFilter;
class TraceFilter : public INeuronProcessObject
{
public:

    static NGTraceFilter New(){return NGTraceFilter(new TraceFilter());}
    TraceFilter();
    ~TraceFilter();
    bool Update();
    ConstDataPointer GetOutput();
    DataPointer GetConnect();
    DataPointer ReleaseData();

    void SetInputBack(ConstDataPointer);
    void SetInputBin(ConstDataPointer);
    void SetSoma(ConstDataPointer);
    DataPointer &GetDendConInfo();

private:
    //m_Input origimage
    ConstDataPointer m_Bin;
    ConstDataPointer m_Back;
    ConstDataPointer m_Soma;
    DataPointer rawDendConInfoPointer;
    //VectorMat2i rawDendConInfo;
    //m_Source tree

    void TraceCurvesAndConInfo(Volume<int> &resultIndexImg, const Volume<unsigned short> &origImg, const Volume<NGCHAR> &binImg, const Volume<unsigned short> &backImg,
                 const VectorVec3d &somaList,
       std::vector<VectorVec5d> &rawDendList, //std::vector<mat2d > &dd2,
       VectorMat2i& rawDendConInfo);

    void SelectSeedForTrace(const Volume<NGCHAR> &binImg, const Volume<unsigned short> &origImg, const Volume<int> &indexImg, VectorVec3d &traceSeed);

    void CalcNeighborSignal(
            const Volume<unsigned short> &origImg,//XXv
            const Vec3d& curSeedNode, //data1
            const Vec3d& initVec, //x1
            const double threv,
            VectorVec3d& neighborPtSet,	//dataS0
            std::vector<double>& neighborWet,	//W
            Vec3d& firDir,//x10
            Vec3d& secDir // x11
            );
    void WeighRayValue(const VectorVec3d &rayNode, const Volume<unsigned short> &locOrigImg, std::vector<double> &rayNodeWet);
    //void WeighRayValue(const VectorVec3d &dataL1, const Volume<unsigned char> &L_XX3, std::vector<double> &aa_data);

    void TraceCurvesFromSeed(const Volume<unsigned short> &orig, const Volume<unsigned short> &back,
                                const Vec3d &initialP, Volume<int> &YY,
                                const VectorVec3d &dataC,
                                const int kkms, VectorVec5d &dataLLss, Mat2i &MM);

    void CalcInitDirectionOnTraceSeed(const Volume<unsigned short> &origImg, const Volume<unsigned short> &backImg,
                                      const Vec3d &initPoint, const double windowSize,
                                      Vec3d &vec1);

    void TraceCurvesForwardFromSeed(const Vec3d &seedInitDir,
                                  const Vec3d &initPt, const Volume<int> &indexImg, const VectorVec3d &somaList,
                                  const Volume<unsigned short> &backImg, const Volume<unsigned short> &origImg,
                                  VectorVec5d &resultSeedCurve, Vec2i &curConInfo);

    void IsCurvesCollide(const Volume<int> &indexImg, const int xMin, const int xMax, const int yMin, const int yMax,
                     const int zMin, const int zMax, int &conInfo, double &maxCollideSetWet, double &somaCollideAngle);

    void TraceCurvesFromSoma(const Volume<unsigned short> &XX3, const Volume<unsigned short> &XXb, Volume<int> &YY,
        const VectorVec5d &initialP, //const Volume<unsigned short> &XXb, const Volume<unsigned short> &XXv,
        const int kkms, const Mat3d &somaInitDir, const VectorVec3d &somaList, /*const VectorVec5d &existDendCurves, */
                             VectorVec5d &dataLLss, Mat2i &MM);

    void TraceCurveForwardFromSoma(const Vec3d &dendSomaInitDir, /*const double thre,*/
                                  const Vec3d &initPt, const Volume<int> &indexImg, /*const Volume<int> &YY,*/ const Volume<unsigned short> &backImg,
                                   const Volume<unsigned short> &origImg, const VectorVec3d &somaList,
                                  VectorVec5d &resultSomaCurve);

    void ReconstructShapeForTrace(
        const Vec3d &intialPoint, const Volume<unsigned short> &origImg, //const Volume<unsigned short> &backImg,
        //Vec3d &denSomaInitDirection, /*Vec3d &xx2, Vec3d &xx3,*/
            //Vec3d &denInitCenter,
            MatXd& resultRayLength,
            VectorVec3d &innerSomaPts);

    void InflateBoundary(const VectorVec3d &innerSomaPts, const Volume<unsigned short> &locOrigImg, const double inflateThrev,
                          VectorVec3d &inflatedArea);

    void InflateMarginalAreaAboveThrev(const VectorVec3d &initPoints, Volume<unsigned short> &locIndexImg, const double thre,
                         const int nx, const int ny, const int nz, VectorVec3d &resultInflatedArea);

//    void DetectCellSignal(const Volume<NGCHAR> &locIndexImg, const VectorVec3d &inflatedPoints,
//                                         VectorVec3d &dendInitPts, std::vector<int> &dendInitPtSetSum);

    void DetectCellSignalModify(const Volume<NGCHAR> &locIndexImg, const VectorVec3d &inflatedPoints, const int threv,
                                         VectorVec3d &dendInitPts, std::vector<int> &dendInitPtSetSum);

    void CalcOrthoBasis(const Vec3d &vec1, Vec3d &vec2, Vec3d &vec3);

    void CalcConstraintPCA(const VectorVec3d &neighborPtSet, //data1
                           const Vec3d &curSeedNode,	//data3
                           const Mat3d &convMat, //data2
                           const std::vector<double> &neighborWet, double &P,
                           Mat3d &sigmaH, //sigmaH
                           Vec3d &mdata3//mdata3
                           );

    void CalcPCADirections(const Mat3d &sigmaH,//sigmaH
                   const Vec3d &initVec,//T1
                   const Vec3d &T2,//T2
                   const double threv,
                   Vec3d &vec1
                   );

    void CalcPCAMainDirection(const Vec3d &x0,
                              const Mat3d &sigmaH,//sigmaH
                              const Vec3d &T1, //T1
                              const Vec3d &T2, //T2
                              const double threv,
                              Vec3d &x1//x1
                              );

    void CalcParmOfCurveNode(const Volume<unsigned short> &origImg, const Volume<unsigned short> &backImg, const Vec3d &curNode,
                             double &radius, double &wet);

    void CalcParmOfCurveNodeList(const Volume<unsigned short> &origImg, const Volume<unsigned short> &backImg,
                                 const VectorVec3d &curNode,
                              std::vector<double> &radius, std::vector<double> &rav);

    void CalcPtCurveDirection(const VectorVec3d &ptCurve, Vec3d &fir /*, Vec3d &x2, Vec3d &x3*/);

    void TraceNextCurveNode(
            const Volume<unsigned short> &locOrigImg,//XXv
            //const Volume<unsigned short> &XXb,//XXb
            const Vec3d &curSeedNode,//dataL1
            const double &threv,
            const Vec3d &initDir,
            /*const Vec3d &T2,
            const Vec3d &T3,*/
            Vec3d &nextCurveNode,
            //Vec3d &vmmk,
            Vec3d &nextDenDir,//¾ÍÊÇvmmk
            /*Vec3d &x2,
            Vec3d &x3,*/
            int &isNextNodeValid//idexx
            );

    void ClearShortCurvesAndInvalidConnection(const std::vector<VectorVec5d > &denPt, const VectorMat2i &denCx,
                      std::vector<VectorVec5d > &denPtNew, VectorMat2i &denCxNew/*, std::vector<int> &denNums*/);

    void AddCollideConnectNode(const std::vector<VectorVec5d > &denPt, const VectorMat2i &denCx, const VectorVec3d &myCell,
                      std::vector<VectorVec5d > &f_pts);

    void ReconstructSomaShapeForTrace(const VectorVec3d &somaList, const Volume<unsigned short>& origImg,
                                      Volume<int>& indexImg, VectorMatXd& allRayLen);

    void FindThickDendDirFromSoma(const MatXd& rayLength, const Volume<unsigned short>& origImg,
                                  const Volume<unsigned short>& backImg,
                                  const Vec3d &currentSoma, const Volume<int>& indexImg, const int seriID,
                                  Vec3d& mainDir1, Vec3d& mainDir2, Vec3d& mainDir3, Vec3d &thickDendInitPt);

    void ExtractLocalDomainAlias(const Vec3d& currentSoma, const Volume<unsigned short>& origImg,
                                 const Volume<unsigned short>& backImg,
                                 const Volume<int>& indexImg, const int seriID,
                                 Volume<unsigned short> &locOrigImg, Vec3d& locPt);

    void ForceModifyDirection(VectorMat3d& allThickDendInitDir, std::vector<int>& largeAngleFlagList);

    void DetectCollision(const VectorVec5d& somaCurve, const VectorVec3d& somaList, const Vec2i& collideInfo,
                         const Volume<int>& indexImg, const Volume<unsigned short>& origImg,
                         const Volume<unsigned short>& backImg, const int origID, const int isSoma, const int isContinue,
                         VectorVec5d& modifiedSomaCurve, int &isContinueTrace, Vec2i& resultCollideInfo,
                         int &curID, int &resultConInfo);

    void TraceForwardInKnot(const Vec3d &initMainDir,const Vec3d& initP,const Volume<unsigned short>& backImg,
                            const Volume<unsigned short>& origImg,
                            const Volume<int>& indexImg,const int collideConInfo,
                            VectorVec3d& crossCurve, int &isContinueTrace);

    void PushNodeUseRayBurst(const Vec3d& curveNode, const Volume<unsigned short>& origImg, const double threv, const Vec3d& x1,
            Vec3d& pushedNode);


    //BridgeBreaker
//    void DeleteErrorDendConnect(const std::vector<VectorVec5d > &rawLine, const VectorMat2i &rawConnect,
//                                  const Volume<unsigned short> &origImg,
//                                  VectorMat2i &resultConnect, VectorVec4d &resultIndex);

//    void GetBoundaryBack(const std::vector<double> &outer_shell,
//                         const double threv,
//                         std::vector<double> &boundary_back);

//    void GetRayLimit(const Volume<double> &sphere_ray_wet,
//                     const double constriction_threv,
//                     std::vector<std::vector<double> > &ray_limit);

//    void GetGradientVectorFlowPPForTrace(const Volume<double> &sphere_ray_wet, Volume<double> &smooth_ray);

//    void SmoothGradientCurvesForTrace(const std::vector<double> &init_one_ray_wet,
//                                    std::vector<double> &smooth_one_ray_wet);

//    void CalculateOneRayLimit(const std::vector<double> &ray,
//                              const double constriction_threv,
//                              int &one_ray_limit);

//    void CalcRayBurstRadWet(const VectorVec5d& curLine, const Volume<unsigned short> &origImg,
//                        MatXd& RR0, MatXd &RR1 );
//    void CalcRayBurstOnePtRadWet(const Vec3d& curLinePt, const Volume<unsigned short> &origImg,
//                        const Vec3d& mainDirection1, Vec2d &aa1 );//RayburstRadius1

//    void JudgeHeadTailForDendCurve(const MatXd& RR0, const MatXd &RR1, const Vec2d &connectStat,
//                              double &parentStat);

//    void KnotAnalysis(const VectorVec5d &curLine, const VectorVec5d &adjecentLine,
//                      double k, double threv, double pd,
//                      double &mm);
//    void CalcConnectKnotLocalDirection(const VectorVec5d &adjecentLine, const Vec5d &curLineNode, double knotStat,
//                          Vec3d& resultDirection);

//    void Connectionf3(const VectorMat2i &cc2, std::vector<std::vector<int>>& BBs);

//    int Connectionf3dis(const Vec3d& ax, const Vec3d& ay);

//    void Connectionf3cluster(const Eigen::MatrixXi& AAm, std::vector<std::vector<int> > &BBs);

//    void Connectionf3cluster1(const Eigen::MatrixXi& AAm, const  std::vector<int> &mmk, std::vector<int>& Lbs,
//                              std::vector<int> &idexsm);

//    void Swcpp122(const std::vector<VectorVec3d >& dd1, const VectorMat2i& dd2, const VectorVec3d &dataC,
//                            std::vector<VectorVec3d >& ddd1, VectorMat2i &ddd2, int &nxx);

    void RayBurstSampling(const Volume<double> &Sphere_XX, const double three_vs, std::vector<std::vector<double> > &Uzz);
    void RayBurstSampling1(const std::vector<double> &LLs, const double three_vs, int &arra);
    void Principald(const VectorVec3d &dataL, Vec3d &x1, Vec3d &x2, Vec3d &x3);
    bool Corrode(const Volume<unsigned char> &binImage, const VectorVec3d &binPtSet,
                 const double eroIntensity, Volume<unsigned char> &eroBinImg, VectorVec3d &eroPtSet);
    void RayBurstShape(const Vec3d &initSoma, const Volume<unsigned short> &v, VectorVec3d &rayNode, Volume<double> &smoothRay);
    void RayburstShapeTrack(const Vec3d &initPointt, const Volume<unsigned short> &origImg, const Vec3d &initDir, int len, VectorVec5d &datatt);
};

#endif // TRACEFILTER_H
