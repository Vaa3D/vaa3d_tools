#ifndef NEURONTREECLUSTER_H
#define NEURONTREECLUSTER_H
#include <list>
#include "../ineuronprocessobject.h"
#include "../../ngtypes/basetypes.h"
template<typename T> class Volume;
typedef Volume<unsigned short> SVolume;
class Tree;
class NeuroTreeCluster;
typedef std::shared_ptr<NeuroTreeCluster> NGNeuronTreeCluster;

class NeuroTreeCluster : public INeuronProcessObject
{
public:
    static NGNeuronTreeCluster New(){return NGNeuronTreeCluster(new NeuroTreeCluster());}
    NeuroTreeCluster();
    ~NeuroTreeCluster();

    bool Update();
    ConstDataPointer GetOutput();
    DataPointer ReleaseData();

    void SetInputCurve(ConstDataPointer);
    void SetInputSoma(ConstDataPointer);
    void SetInputOrigImage(ConstDataPointer);
    void SetInputDeleteID(const VectorVec4d& arg){deleteID = arg;}
    const std::vector<int>& GetTypeList() const {return typeList;}

    //std::vector<std::vector<VectorVec5d> >& GetSeparateTree(){return separateTree;}

private:
    //varient
    ConstDataPointer m_Curve;
    ConstDataPointer m_Soma;
    ConstDataPointer m_OrigImg;
    VectorVec4d deleteID;

    std::vector<int> typeList;
    //std::vector<std::vector<VectorVec5d> > separateTree;
    //function
    //connectionf3
    void ClusterCurvesByConnectInfo(const VectorMat2i &dendSomaInfo, std::vector<std::vector<int> > &dendCurvesCluster);

    int IsTwoCurvesConnect(const Vec3i &ax, const Vec3i &ay);

    void ClusterCurvesByConnectGraph(const Eigen::MatrixXi &dendConGraph,
                                     std::vector<std::vector<int> > &dendCurvesCluster);

    void AddCurveToClusterByConnectGraph(const Eigen::MatrixXi &dendConGraph, const std::vector<int> &conId,
                                         std::vector<int> &isConIdAddedList, std::vector<int> &idexsm);

    //swcp122
    void AddSomaToDendListConInfo(const std::vector<VectorVec5d> &dendList, const VectorMat2i &dendInfo,
                                  const VectorVec3d &somaList, std::vector<VectorVec5d> &newDendList, VectorMat2i &newDendInfo);

    //-----------------------1st assign curve to soma tree and collect unassigned curve for next operation-----------//

    void ClusterTreeToMultiSomaInOneCluster(const VectorMat2i& conInfoInClustre, const std::vector<int>& dendIDInClustre,
                                            const int rawDendNum,
                                            std::vector<VectorVec2i>& somaConnectSet, MatXi& pathGraph, std::vector<std::vector<int>> &allAssignedDendIDSet,
                                            MatXi& compressPathGraph, std::vector<int>& unAssignedList);

    void BuildPathMatrix(const VectorMat2i& conInfoInClustre, const std::vector<int>& dendIDInClustre, MatXi &pathGraph);

    void SearchTreeFromSoma(const MatXi& rawPathGraph, const int somaID, const int threv,
                            std::vector<int> &somaGrowCurveSet);

    void SearchAndBreakTree(const VectorVec2i& aSomaConnectSet, const MatXi& pathGraph, VectorVec2i& resultSet, MatXi& pathGraphCopy);

    //--------------the slowest function----------------//
    void CheckUniqueCurveInTree(const MatXi& breakPathGraph, const std::vector<int>& somaID, const std::vector<int>& somaGrowSet,
                                std::vector<int>& assignedDendIDSet);

    void Intersection(const std::vector<int>& aa, const std::vector<int>& bb, std::vector<int>& cc, std::vector<int>& scc);

    //------------------Bridge is the curve with 2 curves at head and tail--------------//
    void FindBridge(const VectorMat2i& dendSomaInfo, const std::vector<int>& currentDendCurveClustre, const std::vector<int>& unAssignedList,
                    const std::vector<std::vector<int>> &allAssignedDendIDSet,
                    MatXi& pathGraph1, VectorVec3i& possibleConInfo);

    void AttributeUnassignedToPossibleTree(const std::vector<int> &unassignedDendID, const std::vector<int>& aSomaTree, int &connectFlag);

    void FindPossibleConnect(const int conjDendID, const std::vector<int>& currentDendCurveClustre, const std::vector<int>& unAssignedList,
                             const std::vector<std::vector<int>> &allAssignedDendIDSet, const std::vector<int>& idexss,
                             int &possibleConInfo);

    void MapDendWithHeadTailToCompressMat(const std::vector<int>& currentUnassignNet,const std::vector<int>& currentUnassignNetConInfo,
                                          const MatXi& pathGraph1, const VectorVec3i& dendIDListWithHeadTail, const int assignedDendNum,
                                          MatXi &compressPathGraph1, VectorVec3i& validDendIDListWithHeadTail,
                                          std::vector<int> &currentUnNetInOrigPath);

    //---------------there are wrong bridges with only on side , it is strange-----------//
    void BreakSomaTreeUnassignedNet(const MatXi &compressPathGraph1, const int subNetNum, const VectorVec3i& validDendIDListWithHeadTail,
                                    const std::vector<int>& currentDendID, const std::vector<VectorVec5d> &rawDendList,
                                    const SVolume& origImg,
                                    MatXi &newCompressGraph);

    void GetNetBridge(const MatXi& compressPathGraph1, const int subNetNum, const std::vector<int>& validDendIDListWithHeadTail,
                      const std::vector<int>& wetList,
                      int &validIDListWithHeadTail, std::vector<int>& resultWetList);

    void MyDijkstra(const MatXi& graph, const int beg, const int fin, int &mydist, std::vector<int>& mypath);

    void CalcBridgeWet(const std::vector<int>& path, const std::vector<int>& validDendIDListWithHeadTail,
                       std::vector<int>& resultConnectList);

    //----------------decide which side to break-----------------------//
    void JudgeNetBridgeDirection(const VectorVec5d &dendList, const int threv, const SVolume& origImg,
                                 int &flag);
    void WeighRayValue(const VectorVec3d &rayNode, const Volume<unsigned short> &locOrigImg,
                       std::vector<double> &rayNodeWet);
    void CalcRayBurstOnePtRadWet(const Vec3d &curvePt, const Volume<unsigned short> &origImg,
                                 const Vec3d &preDirection, Vec2d &ptRadWet);
    //different from bridgebreaker
    void CalcRayBurstRadWet(const VectorVec5d &noHeadTailDendList, const Volume<unsigned short> &origImg,
                            MatXd &initHeadRadWet, MatXd &initTailRadWet);
    void CalculateOneRayLimit(const std::vector<double> &ray, const double constriction_threv,
                              int &one_ray_limit);
    void SmoothGradientCurvesForTrace(const std::vector<double> &init_one_ray_wet,
                                      std::vector<double> &smooth_one_ray_wet);
    void GetGradientVectorFlowForTrace(const Volume<double> &sphere_ray_wet, Volume<double> &smooth_ray);
    void GetRayLimit(const Volume<double> &sphere_ray_wet, const double constriction_threv,
                     std::vector<std::vector<double> > &ray_limit);
    void GetBoundaryBack(const std::vector<double> &outer_shell, const double threv,
                         std::vector<double> &boundary_back);
    void Principald(const VectorVec3d &dataL, Vec3d &x1);

    //---------------------break unassiged network and cluster them to different soma tree------------//
    void ClusterBrokenNet(const MatXi& newCompressGraph, const std::vector<std::vector<int> >& DD_enlarge,
                          const std::vector<int>& currentUnNetInOrigPath, const std::vector<int>& unAssignedList,
                          const std::vector<int>& currentDendCurveClustre, const int subNetNum,
                          const int assignedDendNum, std::vector<std::vector<int> >& DD_enlarge1);

    void GetConnectUnassignedNet(const MatXi& newCompressGraph, const int ID, std::vector<int>& newNetDendList);

    void BreakPathGraph(MatXi& graph, const std::vector<int>& idexx);

    //-------------------------delete wrong curve , judged by angle , circuit and so on--------------//
    void ReviseTree(const std::vector<VectorVec5d>& dendSomaList, const VectorMat2i& dendSomaInfo,
                    const std::vector<int>& currentSomaTree, const int allCurveNum,
                    std::vector<int>& resultSomaTree);

    void CreateTreeStructure(const std::vector<VectorVec5d> &globalSomaTree, const VectorMat2i &globalSomaTreeInfo,
                             const int &idexx, const std::vector<int>& currentSomaTree,
                             const int deep,
                             std::vector<VectorVec4d> &resultTreeStructure);

    void JudgeTreeCurveDirection(const std::vector<VectorVec5d> &globalSomaTree, const VectorMat2i &globalSomaTreeInfo,
                                 const int idexx, const std::vector<int>& idexxs,
                                 const std::vector<int> &currentSomaTree,
                                 std::vector<int> &connectNodeIDList);

    void CreateOneLevelStructure(const VectorVec4d& currentTreeLevel, const MatXi& localPathGraph,
                                 const std::vector<VectorVec5d>& globalSomaTree, const VectorMat2i &globalSomaTreeInfo,
                                 const std::vector<int>& currentSomaTree,
                                 VectorVec4d& currentLevelInfo, MatXi& resultPathGraph);

    void CalcTreeLevelAngle(const Vec2i &currentTreeNode, const std::vector<int>& idex, const std::vector<VectorVec5d>& globalSomaTree,
                            const VectorMat2i &globalSomaTreeInfo, const std::vector<int>& currentSomaTree,
                            std::vector<double>& dirList, std::vector<double>& angleList);
    void CalcPrinDirAndDistList(const VectorVec3d &ptLine, Vec3d &mainDirection);

    void ReviseOneTree(const std::vector<VectorVec4d>& treeLevel,const std::vector<int>& currentSomaTree,const VectorMat2i &dendSomaInfo,
                       std::vector<VectorVec4d>& newTreeLevel, std::vector<int>& resultSomaTree, VectorMat2i& newDendSomaInfo);

    void ModifyTree(const std::vector<VectorVec4d>& origTreeLevel, const std::vector<VectorVec5d>& dendSomaList,
                    const std::vector<int>& currentSomaTree,
                    std::vector<int>& resultSomaTree);

    void GetTreeLevelFlagList(const std::vector<std::vector<int> >& levelNodeFlag, const int currentLevelID,
                              const std::vector<int> &origSonNodeList, const std::vector<VectorVec4d> &treeLevel,
                              std::vector<std::vector<int> >& newLevelNodeFlag);

    void GetWrongTreeLevelFlag(const std::vector<int> &sonNodeList,const std::vector<int> &currentLevelDepthID,
                               std::vector<int>& wrongIDList);

    //----------------------new algorithm , but also very slow----------//
    void SearchLargeTreeFromSoma(const MatXi &rawPathGraph, const int somaID, const std::vector<int> &allSomaID, const int threv, bool &hasSoma);
    void ClusterTreeToMultiSomaInOneClusterModify(const VectorMat2i &conInfoInClustre, const std::vector<int> &dendIDInClustre, const int rawDendNum, std::vector<VectorVec2i> &somaConnectSet, MatXi &pathGraph, std::vector<std::vector<int> > &allAssignedDendIDSet, MatXi &compressPathGraph, std::vector<int> &unAssignedList);

    //----------------collect the cut tree------------------//
    void RecoverCutTree(const std::vector<int>&, const std::vector<int>&, std::vector<int>&);
    void ClusterCutTree(const std::vector<int>&, const VectorMat2i&, std::vector<std::vector<int>>&);
    void CollectCutTreePiece( const std::vector<std::vector<int>>& clustreInCutTree, const std::vector<VectorVec5d>& newDendList ,std::vector<std::vector<int>>& availableTreeInClustre);

    //rebuild ClusterTreeToMultiSomaInOneCluster with adjacency list-------------//
    void ClusterTreeToMultiSomaInOneCluster(const VectorMat2i &conInfoInClustre, const std::vector<int> &dendIDInClustre, 
        const int rawDendNum, std::vector<VectorVec2i> &somaConnectSet,
        std::vector<std::vector<int> > &allAssignedDendIDSet, MatXi &compressPathGraph, std::vector<int> &unAssignedList);

    //use old BuildPathMatrix function

    void BreakPathListConnect(std::vector<std::list<int>> &pathList, int curveID);

    void SearchTreeFromSomaModify(std::vector<std::list<int> > &rawPathGraph, const int somaID,
        const int threv, std::vector<int>& somaGrowCurveSet);

    void CheckUniqueCurveInTreeModify(const std::vector<std::list<int>> &breakPathGraph,
        const std::vector<int> &somaID,
        const std::vector<int> &somaGrowSet,
        std::vector<std::list<int> >& pathList,
        std::vector<int> &assignedDendIDSet);

    void SearchLargeTreeFromSomaModify( std::vector<std::list<int> > &pathList, const int somaID,
        const std::vector<int>& allSomaID,
        const int threv, bool& hasSoma);

    void SearchAndBreakTreeModify(const VectorVec2i &aSomaConnectSet,
        std::vector<std::list<int>> &pathGraph,
        VectorVec2i &resultSet);

};

#endif // NEURONTREECLUSTER_H
