#include "neurotreecluster.h"
#include "../../ngtypes/tree.h"
#include "../../ngtypes/soma.h"
#include "../../ngtypes/volume.h"
#include "../volumealgo.h"
#include "../contourutil.h"
#include "traceutil.h"
#include <cstdlib>
#include <stdio.h>
#include <math.h>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <deque>
#include <utility>
#include <Eigen/LU>
#ifdef _WIN32
#include <ctime>
#else
#include <sys/time.h>
#endif
#include <limits>
#include <deque>
#include <stdlib.h>
#include <iostream>

#define M_E        2.71828182845904523536
#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923

NeuroTreeCluster::NeuroTreeCluster()
{
    identifyName = std::string("NeuroTreeCluster");
    m_Source = std::shared_ptr<SeperateTree>(new SeperateTree(this));
}

NeuroTreeCluster::~NeuroTreeCluster()
{

}

bool NeuroTreeCluster::Update()
{
    if(!m_Input ){
        printf("error occured in %s\n", identifyName.c_str());
        return false;
    }
    if(m_Input->GetProcessObject()){//|| !m_Soma
        if(!m_Input->GetProcessObject()->Update()){
            printf("error occured in %s\n", identifyName.c_str());
            return false;
        }
    }
	clock_t beg = clock();
    //TODO:
    std::shared_ptr<const TreeCurve> tmpCurve = std::dynamic_pointer_cast<const TreeCurve>(m_Curve);
    std::shared_ptr<const TreeConnect> tmpConnect = std::dynamic_pointer_cast<const TreeConnect>(m_Input);
    std::shared_ptr<const Soma> tmpSoma = std::dynamic_pointer_cast<const Soma>(m_Soma);
    std::shared_ptr<const SVolume> tmpOrigImg = std::dynamic_pointer_cast<const SVolume>(m_OrigImg);
    const std::vector<VectorVec5d> &tmpRawDendList = tmpCurve->GetCurve();
    std::shared_ptr<SeperateTree> tmpSeperateTree = std::dynamic_pointer_cast<SeperateTree>(m_Source);

    VectorVec3d somaList;
    Vec3d tmp;

    //break curve connection
    std::vector<VectorVec5d> rawDendList = tmpRawDendList;
    for(size_t kk = 0; kk < deleteID.size();++kk){
        int id = int(deleteID[kk](0));//deleteID from 0 not 1
                const Mat2i curConInfo= tmpConnect->GetConnect()[id];
        if(curConInfo(0,0)==0){
            VectorVec5d& curCurve = rawDendList[id];
            VectorVec5d modifyCurve;
            modifyCurve.reserve(curCurve.size());
            std::copy(curCurve.begin() + 1, curCurve.end(), std::back_inserter(modifyCurve));
            curCurve.swap(modifyCurve);
        }
        if(curConInfo(0,1)==0){
            VectorVec5d& curCurve = rawDendList[id];
            curCurve.pop_back();
        }
    }


    if(!tmpSoma){
        tmpSoma = std::shared_ptr<const Soma>(new Soma());
    }
    for(int i = 0; i < tmpSoma->size(); ++i){
        tmp << tmpSoma->GetCell(i).x, tmpSoma->GetCell(i).y, tmpSoma->GetCell(i).z;
        somaList.push_back(tmp);
    }

    std::vector<VectorVec5d> newDendList;
    VectorMat2i newDendConInfo;
    AddSomaToDendListConInfo(rawDendList, tmpConnect->GetConnect(), somaList, newDendList, newDendConInfo);
    std::vector<std::vector<int> > dendCurvesCluster;
    ClusterCurvesByConnectInfo(newDendConInfo, dendCurvesCluster);

    std::vector<std::vector<int> > availableTreeInClustre;
    typeList.clear();

//#pragma omp parallel for
    for(int final = 0; final < int(dendCurvesCluster.size());
        ++final){
        printf("%d\n", int(final));

        //TODO:TEST
        const std::vector<int> &currentDendCurveClustre = dendCurvesCluster[final];
        //printf("currentDendCurveClustre :%d\n", int(currentDendCurveClustre.size()));
        std::vector<int> singleCellLabel;
        for(std::vector<int>::size_type i = 0; i < currentDendCurveClustre.size(); ++i){
            if(currentDendCurveClustre[i] > int(rawDendList.size()))
                singleCellLabel.push_back(i);
        }
        //----------------save trees without soma------------//
        if(singleCellLabel.empty()) {
            //continue;
            printf("   cell = 0\n");
            if(currentDendCurveClustre.size() < 3 ){
                int num=0;
                for(size_t kk = 0; kk < currentDendCurveClustre.size();++kk){
                    num += newDendList[currentDendCurveClustre[kk]-1].size();
                }
                if(num < 80)
                    continue;
            }
#pragma omp critical
            {
                availableTreeInClustre.push_back(currentDendCurveClustre);
                typeList.push_back(0);
            }
            
        }
        if(singleCellLabel.size() == 1){
            //continue;
            printf("   cell = 1\n");
            #pragma omp critical
            {
                availableTreeInClustre.push_back(currentDendCurveClustre);
                typeList.push_back(2);
            }
        } else if(singleCellLabel.size() > 1){
			//----------test------------//
			printf("currentDendCurveClustre :%d\n", int(currentDendCurveClustre.size()));
			//availableTreeInClustre.push_back(currentDendCurveClustre);
            //typeList.push_back(2);
            //continue;
			//-----------end test---------------//
            printf("   cell = %d\n", int(singleCellLabel.size()));
            VectorMat2i dendSomaInfo;
            for(VectorMat2i::size_type i = 0 ; i < currentDendCurveClustre.size(); ++i){
                dendSomaInfo.push_back(newDendConInfo[currentDendCurveClustre[i] - 1]);
            }

            //---------test------------//
            /*FILE* fp = fopen("E:/TIFF_double/quan2/denSomaInfo.txt", "w");
            for(int k = 0; k < dendSomaInfo.size(); ++k){
                fprintf(fp, "%d %d %d %d\n", dendSomaInfo[k](0,0),dendSomaInfo[k](0,1),dendSomaInfo[k](1,0),dendSomaInfo[k](1,1));
            }
            fclose(fp);
            fp = fopen("E:/TIFF_double/quan2/currentDendCurveClustre.txt", "w");
            for(int k = 0; k < currentDendCurveClustre.size(); ++k){
                fprintf(fp, "%d\n", currentDendCurveClustre[k]);
            }
            fclose(fp);
            fp = fopen("E:/TIFF_double/quan2/rawDendListNum.txt", "w");
            fprintf(fp, "%d", int(rawDendList.size()));
            fclose(fp);

            fp = fopen("E:/TIFF_double/quan2/rawDendList.txt", "w");
            if(fp){
                for(int i = 0; i < rawDendList.size(); ++i){
                    const VectorVec5d& cur = rawDendList[i];

                    for(VectorVec5d::size_type j = 0; j < cur.size(); ++j){
                        fprintf(fp,"%lf %lf %lf %lf %lf\n", cur[j](0), cur[j](1), cur[j](2), cur[j](3), cur[j](4) );
                    }
                    fprintf(fp,"#\n");
                }
            }
            fclose(fp);

            fp = fopen("E:/TIFF_double/quan2/newDendList.txt", "w");
            if(fp){
                for(int i = 0; i < newDendList.size(); ++i){
                    const VectorVec5d& cur = newDendList[i];

                    for(VectorVec5d::size_type j = 0; j < cur.size(); ++j){
                        fprintf(fp,"%lf %lf %lf %lf %lf\n", cur[j](0), cur[j](1), cur[j](2), cur[j](3), cur[j](4) );
                    }
                    fprintf(fp,"#\n");
                }
            }
            fclose(fp);

            system("pause");*/

            /*availableTreeInClustre.push_back(currentDendCurveClustre);
            typeList.push_back(1);
            break;*/

            std::vector<VectorVec2i> somaConnectSet;
            MatXi pathGraph;
            std::vector<std::vector<int> > allAssignedDendIDSet;
            MatXi compressPathGraph;
            std::vector<int> unAssignedList;
            
            //--------------compare ---------------//
            /*std::vector<std::vector<int> > allAssignedDendIDSet1;
            MatXi compressPathGraph1;
            std::vector<int> unAssignedList1;
            ClusterTreeToMultiSomaInOneClusterModify(dendSomaInfo, currentDendCurveClustre, int(rawDendList.size()),
            somaConnectSet, pathGraph,
            allAssignedDendIDSet1, compressPathGraph1,
            unAssignedList1);*/
            ClusterTreeToMultiSomaInOneCluster(dendSomaInfo, currentDendCurveClustre, int(rawDendList.size()),
                somaConnectSet,
                allAssignedDendIDSet, compressPathGraph,
                unAssignedList);
			printf("ClusterTreeToMultiSomaInOneClusterModify complete.\n");

            /*for (int k = 0; k < allAssignedDendIDSet.size(); ++k) {
                std::vector<int> tmpa=allAssignedDendIDSet[k];
                for (int kk = 0; kk < allAssignedDendIDSet[k].size(); ++kk) {
                    tmpa[kk] = currentDendCurveClustre[allAssignedDendIDSet[k][kk]];
                }
                availableTreeInClustre.push_back(tmpa);
            }
            std::vector<int> tmpa=unAssignedList;
            for (int kk = 0; kk < unAssignedList.size(); ++kk) {
                tmpa[kk] = currentDendCurveClustre[unAssignedList[kk]];
            }
            availableTreeInClustre.push_back(tmpa);
            typeList.push_back(1);
            break;*/
            //printf("ClusterTreeToMultiSomaInOneCluster\n");

            //Test part, skip unAssignedList and allAssignedDendIDSet 2014-4-1
        //    FILE* fp = fopen("/home/zhouhang/TIFF/unAssignedList.txt", "r");
        //    int nima;
        //    while(!feof(fp)){
        //        fscanf(fp, "%d\n", &nima);
        //        --nima;
        //        unAssignedList.push_back(nima);
        //    }
        //    fclose(fp);
        //    fp = fopen("/home/zhouhang/TIFF/allAssignedDendIDSet.txt", "r");
        //    char line[256];
        //    std::vector<int> tmpNima;
        //    while(!feof(fp)){
        //        if (fgets(line, 256, fp) == 0) continue;
        //        if (line[0]=='#'){
        //            allAssignedDendIDSet.push_back(std::vector<int>());
        //            allAssignedDendIDSet[allAssignedDendIDSet.size() - 1].swap(tmpNima);
        //            continue;
        //        }
        //        sscanf(line, "%d", &nima);
        //        --nima;
        //        tmpNima.push_back(nima);
        //    }
        //    fclose(fp);
            //test read end
            MatXi pathGraph1;
            VectorVec3i possibleConInfo;
            FindBridge(newDendConInfo,currentDendCurveClustre,unAssignedList,allAssignedDendIDSet,
                       pathGraph1, possibleConInfo);//warning matlab 1 and C++ 0

            printf("FindBridge\n");
            int pathGraph1XLen = pathGraph1.rows();
            int pathGraph1YLen = pathGraph1.cols();
            MatXi pathGraph2;
            pathGraph2.resize(pathGraph1XLen, pathGraph1YLen);
            pathGraph2.setZero();
            for(int i = 0; i < pathGraph1XLen; ++i){
                for(int j =0; j < pathGraph1YLen; ++j){
                    if(1 == pathGraph1(i,j))
                        pathGraph2(i,j) = 1;
                }
            }

            int assignedDendNum = allAssignedDendIDSet.size();
            MatXi pathGraph3 = pathGraph2;
            //pathGraph3.block(0,0,assignedDendNum, assignedDendNum).setZero();
            for(int i = 0; i < assignedDendNum; ++i){
                for(int j = 0; j < pathGraph3.rows();++j){
                    pathGraph3(i,j)=0;
                }
            }
            for(int i = 0; i < assignedDendNum; ++i){
                for(int j = 0; j < pathGraph3.rows();++j){
                    pathGraph3(j,i)=0;
                }
            }
           //-------compare---------//
            /*std::vector<int> newAssignedIDList2(pathGraph1XLen, 0);
            std::vector<std::vector<int> > unassignDendNet2;
            
            for(int i = 0; i < (pathGraph1XLen - assignedDendNum); ++i){
                if(0==newAssignedIDList2[i + assignedDendNum]){
                    std::vector<int> remainGrowCurveSetCopy;
                    std::vector<int> remainGrowCurveSet;
                    SearchTreeFromSoma(pathGraph3, i + assignedDendNum, 15,
                                       remainGrowCurveSetCopy);
                    for(std::vector<int>::size_type j = 0; j < remainGrowCurveSetCopy.size(); ++j){
                        if(remainGrowCurveSetCopy[j] > 0)
                            remainGrowCurveSet.push_back(remainGrowCurveSetCopy[j]);
                    }
                    for(std::vector<int>::size_type j = 0; j < remainGrowCurveSet.size(); ++j){
                        newAssignedIDList2[remainGrowCurveSet[j]]=1;
                    }
                    unassignDendNet2.push_back(remainGrowCurveSet);
                }
            }*/
           
            std::vector<std::list<int> > pathList3(pathGraph3.rows());
            for(int i = 0; i < pathGraph3.rows();++i){
                for(int j = i; j < pathGraph3.rows();++j){
                    if(pathGraph3(i,j) == 1){
                        pathList3[i].push_back(j);
                        pathList3[j].push_back(i);
                    }
                }
            }
            std::vector<std::list<int> > tmpPathList3;
            std::vector<int> newAssignedIDList(pathGraph1XLen, 0);
            std::vector<std::vector<int> > unassignDendNet;
            for(int i = 0; i < (pathGraph1XLen - assignedDendNum); ++i){
                if(0==newAssignedIDList[i + assignedDendNum]){
                    std::vector<int> remainGrowCurveSetCopy;
                    std::vector<int> remainGrowCurveSet;
                    tmpPathList3 = pathList3;
                    //-----------------------2014-6-30---------------------------//
                    SearchTreeFromSomaModify(tmpPathList3, i + assignedDendNum, 50,
                        remainGrowCurveSetCopy);

                    //-----------------------2014-6-30---------------------------//
                    std::sort(remainGrowCurveSetCopy.begin(), remainGrowCurveSetCopy.end());
                    remainGrowCurveSetCopy.erase(std::unique(remainGrowCurveSetCopy.begin(), remainGrowCurveSetCopy.end()), remainGrowCurveSetCopy.end());
                    for(std::vector<int>::size_type j = 0; j < remainGrowCurveSetCopy.size(); ++j){
                        if(remainGrowCurveSetCopy[j] > 0)
                            remainGrowCurveSet.push_back(remainGrowCurveSetCopy[j]);
                    }
                    for(std::vector<int>::size_type j = 0; j < remainGrowCurveSet.size(); ++j){
                        newAssignedIDList[remainGrowCurveSet[j]]=1;
                    }
                    unassignDendNet.push_back(remainGrowCurveSet);
                }
            }



            int unassignDendNetSize = unassignDendNet.size();
            std::vector<std::vector<int> > unassignedNetSomaConInfo(unassignDendNetSize);
            MatXi subPathGraph2;
            MatXi JJ;
            for(int i = 0; i < unassignDendNetSize; ++i){
                std::vector<int>& currentUnassignedNet = unassignDendNet[i];
                subPathGraph2.resize(currentUnassignedNet.size(), assignedDendNum);
                subPathGraph2.setZero();
                for(std::vector<int>::size_type j = 0; j < currentUnassignedNet.size(); ++j){
                    for(int k = 0; k < assignedDendNum; ++k){
                        subPathGraph2(j,k) = pathGraph2(currentUnassignedNet[j], k);
                    }
                }
                if(currentUnassignedNet.size() > 1){
                    JJ = subPathGraph2.colwise().sum();
                } else{
                    JJ = subPathGraph2;
                }
                std::vector<int> idexx;
                for(int j = 0; j < JJ.cols(); ++j){
                    if(JJ(0,j) > 0)
                        idexx.push_back(j);
                }
                if(!idexx.empty()){
                    unassignedNetSomaConInfo[i].swap(idexx);
                }
            }
			printf("step2\n");

            //Residual_parts  partition
            const VectorVec3i& somaIDListWithHeadTail =  possibleConInfo;
            std::vector<std::vector<int> > DD_enlarge;
            DD_enlarge.resize(assignedDendNum);
            //for(int i = 0; i < unassignDendNetSize; ++i){
            for(int i = 0; i < unassignDendNetSize; ++i){
                const std::vector<int>& currentUnassignNet = unassignDendNet[i];
                const std::vector<int>& currentUnassignNetConInfo =
                        unassignedNetSomaConInfo[i];
                if(currentUnassignNetConInfo.size() > 1){
                    //printf("currentUnassignNetConInfo.size() > 1 start\n");
                    MatXi compressPathGraph1;
                    VectorVec3i validDendIDListWithHeadTail;
                    std::vector<int> currentUnNetInOrigPath;
                    //match with matlab, validDendIDListWithHeadTail add 1
                    //but currentUnNetInOrigPath is from 0
                    MapDendWithHeadTailToCompressMat(currentUnassignNet,
                                                     currentUnassignNetConInfo,
                                                     pathGraph1,
                                                     somaIDListWithHeadTail,
                                                     assignedDendNum,
                                                     compressPathGraph1,
                                                     validDendIDListWithHeadTail,
                                                     currentUnNetInOrigPath);
                    //printf("MapDendWithHeadTailToCompressMat\n");

                    int subNetNum = int(currentUnassignNetConInfo.size());
                    std::vector<int> currentDendID;
                    //std::vector<std::vector<int> > DD_enlarge;
                    std::vector<std::vector<int> > DD_enlarge1;
                    for(std::vector<int>::size_type j = 0; j < validDendIDListWithHeadTail.size();
                        ++j){
                        currentDendID.push_back(
                                    currentDendCurveClustre[unAssignedList[
                                currentUnNetInOrigPath[
                                validDendIDListWithHeadTail[j](0)-1] - assignedDendNum]]);
                    }
                    MatXi newCompressGraph;
                    BreakSomaTreeUnassignedNet(compressPathGraph1,subNetNum,validDendIDListWithHeadTail,
                                               currentDendID,rawDendList,*tmpOrigImg, newCompressGraph);
					//----test------//
					/*FILE* fp = fopen("D:/hehe.txt","w");
					for(int ii = 0; ii < compressPathGraph1.rows(); ++ii){
						for (int jj = 0; jj < compressPathGraph1.cols(); ++jj){
							fprintf(fp, "%d ", compressPathGraph1(ii,jj));
						}
						fprintf(fp,"\n");
					}
					fclose(fp);*/
					//std::cout << newCompressGraph << std::endl;
                    ClusterBrokenNet(newCompressGraph,DD_enlarge,currentUnNetInOrigPath,unAssignedList,
                                     currentDendCurveClustre,subNetNum,assignedDendNum,
                                     DD_enlarge1);
                    //printf("it is OK ClusterBrokenNet\n");
                    DD_enlarge = (DD_enlarge1);
                } else if(1 == currentUnassignNetConInfo.size()){
                    //printf("currentUnassignNetConInfo.size() == 1 start\n");
                    int idexx = currentUnassignNetConInfo[0];
                    std::vector<int> Lssj = DD_enlarge[idexx];
                    std::vector<int> Label_dentric;
                    for(std::vector<int>::size_type m = 0; m < currentUnassignNet.size();++m){
                        Label_dentric.push_back(
                                    currentDendCurveClustre[unAssignedList[currentUnassignNet[m]
                                - assignedDendNum]]);
                    }
                    //int sz = DD_enlarge[idexx].size();
                    DD_enlarge[idexx].resize(Label_dentric.size() + Lssj.size());
                    std::copy(Label_dentric.begin(), Label_dentric.end(), DD_enlarge[idexx].begin());
                    std::copy(Lssj.begin(), Lssj.end(), DD_enlarge[idexx].begin() + Label_dentric.size());
                    //printf("it is OK 1 == currentUnassignNetConInfo\n");
                }
            }
            printf("step3\n");

            int nxx = allAssignedDendIDSet.size();
            std::vector<std::vector<int> > somaTreeInClustre;
            somaTreeInClustre.resize(nxx);
            for(int i = 0; i < nxx; ++i){
                if(!DD_enlarge[i].empty()){
                    std::vector<int> &curTree = somaTreeInClustre[i];
                    curTree.push_back(currentDendCurveClustre[singleCellLabel[i]]);
                    for(std::vector<int>::size_type j = 0; j < allAssignedDendIDSet[i].size(); ++j){
                        curTree.push_back(currentDendCurveClustre[allAssignedDendIDSet[i][j]]);
                    }
                    for(std::vector<int>::size_type j = 0; j < DD_enlarge[i].size(); ++j){
                        curTree.push_back(DD_enlarge[i][j]);
                    }
                } else{
                    std::vector<int> &curTree = somaTreeInClustre[i];
                    curTree.push_back(currentDendCurveClustre[singleCellLabel[i]]);
                    for(std::vector<int>::size_type j = 0; j < allAssignedDendIDSet[i].size(); ++j){
                        curTree.push_back(currentDendCurveClustre[allAssignedDendIDSet[i][j]]);
                    }
                }
            }


            for(std::vector<int>::size_type i = 0; i < somaTreeInClustre.size();++i){
                const std::vector<int> &currentSomaTree = somaTreeInClustre[i];
                std::vector<int> resultTreeInClustre;
                ReviseTree(newDendList, newDendConInfo, currentSomaTree, rawDendList.size(),
                           resultTreeInClustre);
#pragma omp critical
                {
                    availableTreeInClustre.push_back(resultTreeInClustre);
                    printf("resultTreeInClustre size : %d \n", int(resultTreeInClustre.size()));
                    typeList.push_back(2);
                    int oldSz = int(availableTreeInClustre.size());
                    std::vector<int> cutTree;
                    RecoverCutTree(currentSomaTree, resultTreeInClustre, cutTree);
                    printf("cutTree size : %d \n", int(cutTree.size()));
                    std::vector<std::vector<int> > clustreInCutTree;
                    ClusterCutTree(cutTree, newDendConInfo, clustreInCutTree);
                    CollectCutTreePiece(clustreInCutTree,newDendList, availableTreeInClustre);
                    int diff = int(availableTreeInClustre.size()) - oldSz;
                    for (int z = 0; z < diff; ++z) {
                        typeList.push_back(0);
                    }
                }
            }
        }

    }


    std::vector<std::vector<VectorVec5d> >& separateTree=tmpSeperateTree->GetTree();
    for(size_t i = 0; i < availableTreeInClustre.size();++i){
        std::vector<VectorVec5d> tmp;
        std::vector<int>& tmpID = availableTreeInClustre[i];
        for(size_t j = 0; j < tmpID.size(); ++j){
            tmp.push_back(newDendList[tmpID[j] - 1]);
            //tmp.push_back(newDendList[tmpID[j] ]);
        }
        separateTree.push_back(tmp);
    }
    tmpSeperateTree->SetTypeList(typeList);
	clock_t end = clock();
	printf("tree cluster use %d ms.\n",end - beg);
    return true;
}

ConstDataPointer NeuroTreeCluster::GetOutput()
{
    if(!m_Source)
        m_Source = std::shared_ptr<TreeConnect>(new TreeConnect(this));
    return m_Source;
}

DataPointer NeuroTreeCluster::ReleaseData()
{
    m_Source->ReleaseProcessObject();
    DataPointer tData(m_Source);
    m_Source.reset();
    return tData;
}

void NeuroTreeCluster::SetInputCurve(ConstDataPointer arg)
{
    m_Curve = arg;
}

void NeuroTreeCluster::SetInputSoma(ConstDataPointer arg)
{
    m_Soma = arg;
}

void NeuroTreeCluster::SetInputOrigImage(ConstDataPointer arg)
{
    m_OrigImg =arg;
}

void NeuroTreeCluster::ClusterCurvesByConnectInfo(const VectorMat2i &dendConInfo,
                                                   std::vector<std::vector<int> > &dendCurvesCluster)
{
    VectorMat2i::size_type dendConInfoSum = dendConInfo.size();
    VectorVec3i allDendConInfo;
    Vec3i tmpMms;

    Eigen::MatrixXi dendConGraph(dendConInfoSum,dendConInfoSum);
    dendConGraph.setZero();
    for (VectorMat2i::size_type i = 0; i < dendConInfoSum; ++i){
        const Mat2i &MM = dendConInfo[i];
        tmpMms<< MM(0,0), MM(0,1), i + 1;
        allDendConInfo.push_back(tmpMms);
    }

    Vec3i ax; Vec3i ay;
    for (VectorMat2i::size_type i = 0; i < dendConInfoSum; ++i){
        for (VectorMat2i::size_type j = i + 1 ; j < dendConInfoSum; ++j)
        {
            ax << allDendConInfo[i](0), allDendConInfo[i](1), allDendConInfo[i](2);
            ay << allDendConInfo[j](0), allDendConInfo[j](1), allDendConInfo[j](2);
            dendConGraph(i, j) = IsTwoCurvesConnect(ax, ay);
            dendConGraph(j, i) = dendConGraph(i, j);
        }
    }

    dendCurvesCluster.clear();
    ClusterCurvesByConnectGraph(dendConGraph, dendCurvesCluster);
}

int NeuroTreeCluster::IsTwoCurvesConnect(const Vec3i &ax, const Vec3i &ay)
{
    //int nx(3), ny(3);
    int isConnect(0);
    for ( int i = 0; i < 3; ++i){
        for (int j = 0; j < 3; ++j){
            //if ( !( std::abs(ax(i) - 0.0) < EXPO ) && std::abs(ax(i) - ay(j)) < EXPO )
            if (  ax(i) != 0 && ax(i) == ay(j) )
                isConnect = 1;
        }
    }//for
    return isConnect;
}

void NeuroTreeCluster::ClusterCurvesByConnectGraph(const Eigen::MatrixXi &dendConGraph,
                                                    std::vector<std::vector<int> > &dendCurvesCluster)
{
    int nx = dendConGraph.rows();
    std::vector<int> Lbs(nx, 0);
    std::vector<int> currentConSet;
    std::vector<int> idexs;
    for (int i = 0; i < nx; ++i){
        currentConSet.clear();
        if ( 0 == Lbs[i] ){
            std::vector<int> tmp;
            tmp.push_back(i + 1);
            idexs.clear();
            AddCurveToClusterByConnectGraph(dendConGraph, tmp, Lbs, idexs);
            std::copy(idexs.begin(), idexs.end(), std::back_inserter(currentConSet));
            while ( !idexs.empty() ){
                AddCurveToClusterByConnectGraph(dendConGraph, std::vector<int>(idexs), Lbs, idexs);
                std::copy(idexs.begin(), idexs.end(), std::back_inserter(currentConSet));
            }
            dendCurvesCluster.push_back(currentConSet);
        }
    }
}

void NeuroTreeCluster::AddCurveToClusterByConnectGraph(const Eigen::MatrixXi &dendConGraph, const std::vector<int> &conId,
                                                        std::vector<int> &isConIdAddedList, std::vector<int> &idexsm)
{
    idexsm.clear();
    std::vector<int>::size_type nx = conId.size();
    std::vector<int> rawConnectId;
    std::vector<int> idexm;
    std::deque<int> uniqConnectId;
    for (std::vector<int>::size_type ik = 0; ik < nx; ++ik){
        int i = conId[ik];
        rawConnectId.clear();
        idexm.clear();
        uniqConnectId.clear();
        for (int j = 0; j < dendConGraph.cols(); ++j){
            if (1 == dendConGraph(i - 1, j))
                rawConnectId.push_back(j);
        }

        if (!rawConnectId.empty()){
            for (std::vector<int>::size_type k = 0; k < rawConnectId.size(); ++k){
                if (0 == isConIdAddedList[rawConnectId[k]])
                    idexm.push_back(k);
            }

            if (!idexm.empty()){
                uniqConnectId.clear();
                for (std::vector<int>::size_type kk = 0; kk < idexm.size(); ++kk){
                    uniqConnectId.push_back(rawConnectId[idexm[kk]] + 1);
                    isConIdAddedList[uniqConnectId.back() - 1] = 1;
                }
                if (0 == isConIdAddedList[i - 1]){
                    uniqConnectId.push_front(i);
                    isConIdAddedList[i - 1] = 1;
                }
            }
        }

        if (rawConnectId.empty() && 0 == isConIdAddedList[i - 1]){
            uniqConnectId.clear();
            uniqConnectId.push_back(i );
            isConIdAddedList[i - 1] = 1;
        }

        std::copy(uniqConnectId.begin(), uniqConnectId.end(), std::back_inserter(idexsm));
    }
}

void NeuroTreeCluster::AddSomaToDendListConInfo(const std::vector<VectorVec5d> &dendList, const VectorMat2i &dendInfo,
                           const VectorVec3d &somaList, std::vector<VectorVec5d> &newDendList, VectorMat2i &newDendInfo)
{
    int nxx = (int)dendList.size();
    //some different between matlab and C
    newDendList = dendList;
    newDendInfo = dendInfo;
    for (int i = 0; i < nxx; ++i){
        Mat2i aa = dendInfo[i];
        if (aa(0,0) < 0 ){
            aa(0,0) = -aa(0,0) + nxx;
            newDendInfo[i] = aa;
        }
        if (aa(0,1) < 0 ){
            aa(0,1) = -aa(0,1) + nxx;
            newDendInfo[i] = aa;
        }
    }

    VectorVec5d tmpDend;
    Mat2i tmpConInfo;
    tmpConInfo.setZero();
    Vec5d tmp;
    for (VectorVec5d::size_type i = 0; i < somaList.size(); ++i){
        tmp << somaList[i](0), somaList[i](1), somaList[i](2), 0.0, 0.0;
        tmpDend.clear();
        tmpDend.push_back(tmp);
        newDendList.push_back(tmpDend);
        //newDendInfo{nxx+i}=zeros(2,2);
        newDendInfo.push_back(tmpConInfo);
    }
}

void NeuroTreeCluster::ClusterTreeToMultiSomaInOneClusterModify(const VectorMat2i &conInfoInClustre,
                                                           const std::vector<int> &dendIDInClustre,
                                                           const int rawDendNum,
                                                           std::vector<VectorVec2i> &somaConnectSet,
                                                           MatXi &pathGraph,
                                                           std::vector<std::vector<int> > &allAssignedDendIDSet,
                                                           MatXi &compressPathGraph,
                                                           std::vector<int> &unAssignedList)
{
    std::vector<int> somaID;
    for(std::vector<int>::size_type i = 0; i < dendIDInClustre.size(); ++i){
        if(dendIDInClustre[i] > rawDendNum)
            somaID.push_back(i);
    }

    int somaNum = somaID.size();
    somaConnectSet.clear();

    BuildPathMatrix(conInfoInClustre, dendIDInClustre, pathGraph);

    VectorVec2i aas;
    Vec2i tmpAAS;
    for(int i = 0; i < somaNum; ++i){
        std::vector<int> somaConnectID;
        for(int j = 0 ; j < pathGraph.rows(); ++j){
            if(pathGraph(somaID[i], j) == 1){
                somaConnectID.push_back(j);
            }
        }

        aas.clear();
        //aas.resize(somaConnectID.size());
        for(std::vector<int>::size_type j = 0; j < somaConnectID.size();++j){
            tmpAAS << somaID[i], somaConnectID[j];
            aas.push_back(tmpAAS);
        }
        somaConnectSet.push_back(VectorVec2i());
        somaConnectSet[somaConnectSet.size() - 1].swap(aas);
    }

    allAssignedDendIDSet.clear();
	allAssignedDendIDSet.resize(somaNum);
    std::vector<int> isAssignedList;
    isAssignedList.resize(pathGraph.rows(),0);
    //MatXi breakPathGraph;

    //std::vector<int> allSoma012Level;
	std::vector<std::vector<int>> allSoma012Level;
	std::vector<std::vector<int> > allSoma012LevelList;

	//for(int kk = 0; kk < somaNum; ++kk){
	//	for(int i = 0; i < somaNum; ++i){
	//		if (i == kk)
	//			continue;
	//		breakPathGraph = pathGraph;
	//		VectorVec2i &aSomaConnectSet = somaConnectSet[i];
	//		breakPathGraph.row(somaID[i]).setZero();// *= 0;
	//		breakPathGraph.col(somaID[i]).setZero();// *= 0;
	//		for(VectorVec2i::size_type ii = 0; ii < aSomaConnectSet.size(); ++ii){
	//			breakPathGraph.row(aSomaConnectSet[ii](1)).setZero();
	//			breakPathGraph.col(aSomaConnectSet[ii](1)).setZero();
	//		}
	//		std::vector<int> somaGrowSet;
	//		SearchTreeFromSoma(pathGraph, somaID[i], 3, somaGrowSet);
	//		//printf("%d soma SearchTreeFromSoma\n",i);
	//		size_t oldSz = allSoma012Level.size();
	//		allSoma012Level.resize(oldSz + somaGrowSet.size());
	//		std::copy(somaGrowSet.begin(), somaGrowSet.end(), allSoma012Level.begin() + oldSz);
	//	}//for somaNum
	//	std::sort(allSoma012Level.begin(), allSoma012Level.end());
	//	allSoma012Level.erase(std::unique(allSoma012Level.begin(), allSoma012Level.end()),
	//						allSoma012Level.end());
	//	allSoma012LevelList.push_back(allSoma012Level);
	//	allSoma012Level.clear();
	//}

	/*std::sort(allSoma012Level.begin(), allSoma012Level.end());
	allSoma012Level.erase(std::unique(allSoma012Level.begin(), allSoma012Level.end()),
						allSoma012Level.end());
	allSoma012LevelList.push_back(allSoma012Level);
	allSoma012Level.clear();*/

	//printf("hehe begin.\n");
	//for(int i = 0; i < somaNum; ++i){
	//	std::vector<int> somaGrowSet;
	//	SearchTreeFromSoma(pathGraph, somaID[i], 3, somaGrowSet);
	//	/*size_t oldSz = allSoma012Level.size();
	//	allSoma012Level.resize(oldSz + somaGrowSet.size());
	//	std::copy(somaGrowSet.begin(), somaGrowSet.end(), allSoma012Level.begin() + oldSz);*/
	//	allSoma012Level.push_back(somaGrowSet);
	//}//all curve

	//printf("allSoma012Level complete.\n");

	//for (int i = 0; i < somaNum; ++i)
	//{
	//	std::vector<int> curSomaNeighborCurve;
	//	for (int j = 0; j < somaNum; j++)
	//	{
	//		if(j == i) continue;
	//		std::copy(allSoma012Level[j].begin(), allSoma012Level[j].end(), std::back_inserter(curSomaNeighborCurve));
	//	}
	//	std::sort(curSomaNeighborCurve.begin(), curSomaNeighborCurve.end());
	//	curSomaNeighborCurve.erase(std::unique(curSomaNeighborCurve.begin(), curSomaNeighborCurve.end()),
	//					curSomaNeighborCurve.end());
	//	allSoma012LevelList.push_back(curSomaNeighborCurve);
	//}
 //   
	//printf("allSoma012LevelList complete.\n");
    //printf("allSoma012Level=%d\n", int(allSoma012Level.size()));

    //printf("somaNum=%d\n", somaNum);
	int process = 0;
//#pragma omp parallel for
    for(int i = 0; i < somaNum; ++i){
        //printf("%d soma\n",i);
		MatXi breakPathGraph;
        breakPathGraph = pathGraph;
        VectorVec2i &aSomaConnectSet = somaConnectSet[i];
        breakPathGraph.row(somaID[i]).setZero();// *= 0;
        breakPathGraph.col(somaID[i]).setZero();// *= 0;
        for(VectorVec2i::size_type ii = 0; ii < aSomaConnectSet.size(); ++ii){
            breakPathGraph.row(aSomaConnectSet[ii](1)).setZero();
            breakPathGraph.col(aSomaConnectSet[ii](1)).setZero();
        }

		//-----------2014-5-26--------------//
		MatXi pathGraph11=pathGraph;
		for (int IKK=0; IKK < somaNum; ++IKK){
			if (IKK!=i){
				VectorVec2i &aSomaConnectSet1 = somaConnectSet[IKK];
				for(VectorVec2i::size_type ii = 0; ii < aSomaConnectSet1.size(); ++ii){
					pathGraph11.row(aSomaConnectSet1[ii](1)).setZero();
					pathGraph11.col(aSomaConnectSet1[ii](1)).setZero();
				}
			}
		}


        std::vector<int> somaGrowSet;
        SearchTreeFromSoma(pathGraph11, somaID[i], 6, somaGrowSet);
        //printf("%d soma SearchTreeFromSoma\n",i);
        std::vector<int> assignedDendIDSet;
        //TODO:
		//printf("%d allSoma012Level=%d\n",i, int(allSoma012LevelList[i].size()));
        //CheckUniqueCurveInTree(breakPathGraph, allSoma012LevelList[i], somaGrowSet, assignedDendIDSet);
        CheckUniqueCurveInTree(breakPathGraph, somaID, somaGrowSet, assignedDendIDSet);
#pragma omp critical
		{
			printf("%d UniqueCurveInTree\n",int(assignedDendIDSet.size()));
			++process;
			printf("%d of %d complete.\n",process, somaNum);
			for(std::vector<int>::size_type j = 0; j < assignedDendIDSet.size(); ++j){
				isAssignedList[assignedDendIDSet[j]] = 1;
			}
			//allAssignedDendIDSet.push_back(std::vector<int>());
			//allAssignedDendIDSet[allAssignedDendIDSet.size() - 1].swap(assignedDendIDSet);
			allAssignedDendIDSet[i] = assignedDendIDSet;
		}
    }//for somaNum
    //printf("nima2\n");

    for(std::vector<int>::size_type i = 0; i < somaID.size(); ++i){
        isAssignedList[somaID[i]] = 1;
    }
    unAssignedList.clear();
    for(std::vector<int>::size_type i = 0; i < isAssignedList.size(); ++i){
        if(isAssignedList[i] == 0){
            unAssignedList.push_back(i);
        }
    }

    int nk = unAssignedList.size() + allAssignedDendIDSet.size();
    int ndd = allAssignedDendIDSet.size();
    compressPathGraph.resize(nk,nk);
    for(std::vector<int>::size_type i = 0; i < unAssignedList.size(); ++i){
        std::vector<int> idll;
        for(int j = 0; j < pathGraph.rows(); ++j){
            if(pathGraph(unAssignedList[i], j) == 1){
                idll.push_back(j);
            }
        }

        for(int j = 0; j < ndd; ++j){
            std::vector<int> cc;
            std::vector<int> scc;
            Intersection(idll, allAssignedDendIDSet[j], cc, scc);
            if(!cc.empty()){
                compressPathGraph(ndd + i, j) = 1;
                compressPathGraph(j, ndd + i) = 1;
            }
        }
    }
    //printf("nima3\n");

    for(std::vector<int>::size_type i = 0; i < unAssignedList.size(); ++i){
        std::vector<int> idll;
        for(int j = 0; j < pathGraph.rows(); ++j){
            if(pathGraph(unAssignedList[i], j) == 1){
                idll.push_back(j);
            }
        }

        std::vector<int> cc;
        std::vector<int> scc;
        Intersection(idll, unAssignedList, cc, scc);
        if(!cc.empty()){
            for(std::vector<int>::size_type k = 0; k < scc.size(); ++k){
                compressPathGraph(ndd + i, scc[k] + ndd) = 1;
                compressPathGraph(scc[k] + ndd, ndd + i) = 1;
            }
        }
    }
    //printf("nima4\n");

    for(int i = 0; i < nk; ++i){
        for(int j = i + 1; j < nk; ++j){
            if(compressPathGraph(i,j)==0){
                compressPathGraph(i,j) = std::numeric_limits<int>::max();
                compressPathGraph(j,i) = std::numeric_limits<int>::max();
            }
        }
    }
    //printf("nima5\n");
}

void NeuroTreeCluster::ClusterTreeToMultiSomaInOneCluster(const VectorMat2i &conInfoInClustre,
                                                           const std::vector<int> &dendIDInClustre,
                                                           const int rawDendNum,
                                                           std::vector<VectorVec2i> &somaConnectSet,
                                                           MatXi &pathGraph,
                                                           std::vector<std::vector<int> > &allAssignedDendIDSet,
                                                           MatXi &compressPathGraph,
                                                           std::vector<int> &unAssignedList)
{
    std::vector<int> somaID;
    for(std::vector<int>::size_type i = 0; i < dendIDInClustre.size(); ++i){
        if(dendIDInClustre[i] > rawDendNum)
            somaID.push_back(i);
    }

    int somaNum = somaID.size();
    somaConnectSet.clear();

    BuildPathMatrix(conInfoInClustre, dendIDInClustre, pathGraph);
    //printf("BuildPathMatrix\n");

    VectorVec2i aas;
    Vec2i tmpAAS;
    for(int i = 0; i < somaNum; ++i){
        std::vector<int> somaConnectID;
        for(int j = 0 ; j < pathGraph.rows(); ++j){
            if(pathGraph(somaID[i], j) == 1){
                somaConnectID.push_back(j);
            }
        }

        aas.clear();
        //aas.resize(somaConnectID.size());
        for(std::vector<int>::size_type j = 0; j < somaConnectID.size();++j){
            tmpAAS << somaID[i], somaConnectID[j];
            aas.push_back(tmpAAS);
        }
        somaConnectSet.push_back(VectorVec2i());
        somaConnectSet[somaConnectSet.size() - 1].swap(aas);
    }

    //printf("nima1\n");

    allAssignedDendIDSet.clear();
    std::vector<int> isAssignedList;
    isAssignedList.resize(pathGraph.rows(),0);
    MatXi breakPathGraph;
    //printf("somaNum=%d\n", somaNum);
//#pragma omp parallel for
    for(int i = 0; i < somaNum; ++i){
        //printf("%d soma\n",i);
        breakPathGraph = pathGraph;
        VectorVec2i &aSomaConnectSet = somaConnectSet[i];
        breakPathGraph.row(somaID[i]).setZero();// *= 0;
        breakPathGraph.col(somaID[i]).setZero();// *= 0;
        for(VectorVec2i::size_type ii = 0; ii < aSomaConnectSet.size(); ++ii){
            breakPathGraph.row(aSomaConnectSet[ii](1)).setZero();
            breakPathGraph.col(aSomaConnectSet[ii](1)).setZero();
        }
        std::vector<int> somaGrowSet;
        SearchTreeFromSoma(pathGraph, somaID[i], 6, somaGrowSet);
        //printf("%d soma SearchTreeFromSoma\n",i);
        std::vector<int> assignedDendIDSet;
        CheckUniqueCurveInTree(breakPathGraph, somaID, somaGrowSet, assignedDendIDSet);
        //printf("%d soma CheckUniqueCurveInTree\n",i);
        for(std::vector<int>::size_type j = 0; j < assignedDendIDSet.size(); ++j){
            isAssignedList[assignedDendIDSet[j]] = 1;
        }
        allAssignedDendIDSet.push_back(std::vector<int>());
        allAssignedDendIDSet[allAssignedDendIDSet.size() - 1].swap(assignedDendIDSet);
    }//for somaNum
    //printf("nima2\n");

    for(std::vector<int>::size_type i = 0; i < somaID.size(); ++i){
        isAssignedList[somaID[i]] = 1;
    }
    unAssignedList.clear();
    for(std::vector<int>::size_type i = 0; i < isAssignedList.size(); ++i){
        if(isAssignedList[i] == 0){
            unAssignedList.push_back(i);
        }
    }



    int nk = unAssignedList.size() + allAssignedDendIDSet.size();
    int ndd = allAssignedDendIDSet.size();
    compressPathGraph.resize(nk,nk);
    for(std::vector<int>::size_type i = 0; i < unAssignedList.size(); ++i){
        std::vector<int> idll;
        for(int j = 0; j < pathGraph.rows(); ++j){
            if(pathGraph(unAssignedList[i], j) == 1){
                idll.push_back(j);
            }
        }

        for(int j = 0; j < ndd; ++j){
            std::vector<int> cc;
            std::vector<int> scc;
            Intersection(idll, allAssignedDendIDSet[j], cc, scc);
            if(!cc.empty()){
                compressPathGraph(ndd + i, j) = 1;
                compressPathGraph(j, ndd + i) = 1;
            }
        }
    }
    //printf("nima3\n");

    for(std::vector<int>::size_type i = 0; i < unAssignedList.size(); ++i){
        std::vector<int> idll;
        for(int j = 0; j < pathGraph.rows(); ++j){
            if(pathGraph(unAssignedList[i], j) == 1){
                idll.push_back(j);
            }
        }

        std::vector<int> cc;
        std::vector<int> scc;
        Intersection(idll, unAssignedList, cc, scc);
        if(!cc.empty()){
            for(std::vector<int>::size_type k = 0; k < scc.size(); ++k){
                compressPathGraph(ndd + i, scc[k] + ndd) = 1;
                compressPathGraph(scc[k] + ndd, ndd + i) = 1;
            }
        }
    }
    //printf("nima4\n");

    for(int i = 0; i < nk; ++i){
        for(int j = i + 1; j < nk; ++j){
            if(compressPathGraph(i,j)==0){
                compressPathGraph(i,j) = std::numeric_limits<int>::max();
                compressPathGraph(j,i) = std::numeric_limits<int>::max();
            }
        }
    }
    //printf("nima5\n");
}

void NeuroTreeCluster::BuildPathMatrix(const VectorMat2i &conInfoInClustre,
                                        const std::vector<int> &dendIDInClustre, MatXi &pathGraph)
{
    int nxx = conInfoInClustre.size();
    if(pathGraph.rows() != nxx || pathGraph.cols() != nxx)
        pathGraph.resize(nxx, nxx);
    pathGraph.setZero();

    for(int i = 0; i < nxx; ++i){
        for(int j = i + 1; j < nxx; ++j){
            const Mat2i &aa = conInfoInClustre[i];
            const Mat2i &bb = conInfoInClustre[j];
            if(aa(0,0) == dendIDInClustre[j] || aa(0,1)==dendIDInClustre[j]
                    || bb(0,0)==dendIDInClustre[i] || bb(0,1)==dendIDInClustre[i]){
                pathGraph(i,j) = 1;
                pathGraph(j,i) = 1;
            }
        }
    }
}

void NeuroTreeCluster::SearchLargeTreeFromSoma(const MatXi &rawPathGraph, const int somaID,
                                                const std::vector<int>& allSomaID,
                                           const int threv, bool& hasSoma)
{
    hasSoma = false;
    int i = 0;
    //std::vector<int> somaGrowCurveSet;
    MatXi pathGraph = rawPathGraph;
    std::vector<VectorVec2i> somaConnectSet;
    std::vector<int> somaConnectID;
    VectorVec2i aas;
    Vec2i tmpAAS;
    for(int j = 0 ; j < pathGraph.rows(); ++j){
        if(pathGraph(somaID, j) == 1){
            somaConnectID.push_back(j);
        }
    }
    aas.clear();
    //aas.resize(somaConnectID.size());
    for(std::vector<int>::size_type j = 0; j < somaConnectID.size();++j){
        tmpAAS << somaID, somaConnectID[j];
        aas.push_back(tmpAAS);
    }

    if(!somaConnectID.empty()){
        somaConnectSet.push_back(aas);
        pathGraph.row(somaID).setZero();// *= 0;
        pathGraph.col(somaID).setZero();// *= 0;
        i=1;
    } else{
        VectorVec2i tmp;
        tmp.push_back(Vec2i(somaID,0));
        somaConnectSet.push_back(tmp);
    }


    if(!aas.empty()){
        std::vector<int> cc1, scc1;
        std::vector<int> testID1;
        for(size_t k = 0; k < somaConnectSet.back().size();++k){
            testID1.push_back(somaConnectSet.back()[k](1));
        }
        Intersection(testID1, allSomaID, cc1, scc1);
        if(!cc1.empty()){
            //printf("jieduan in %d\n", i);
            hasSoma = true;
            return;
        }
        else{
            MatXi pathGraphCopy;
            for(i = 1; i < threv; ++i){//global varient i
                SearchAndBreakTree(somaConnectSet[i - 1], pathGraph, aas, pathGraphCopy);
                pathGraph = pathGraphCopy;
                if(!aas.empty()){
                    somaConnectSet.push_back(VectorVec2i());
                    somaConnectSet[somaConnectSet.size() - 1].swap(aas);
                    std::vector<int> cc, scc;
                    std::vector<int> testID;
                    for(size_t k = 0; k < somaConnectSet.back().size();++k){
                        //if(somaConnectSet.back()[k](1) == 447){
                            //printf("nima\n");
                        //}
                        testID.push_back(somaConnectSet.back()[k](1));
                    }
                    Intersection(testID, allSomaID, cc, scc);
                    if(!cc.empty()){
                        //printf("jieduan in %d\n", i);
                        hasSoma = true;
                        break;
                    }
                }
                else break;
            }
        }
        --i;//warning!
    } else{
        i = 0;
    }

//    if(i > 0){

//    }
//    somaGrowCurveSet.clear();
//    const VectorVec2i &data1 = somaConnectSet[0];
//    somaGrowCurveSet.push_back(data1[0](0));
    //if(somaConnectSet.size() <= i || std::abs(somaConnectSet.size() - i) > 1)
        //printf("error here!\n");
//    for(int ii = 0; ii <= i; ++ii){
//        const VectorVec2i &data = somaConnectSet[ii];
//        if(!data.empty()){
//            for(VectorVec2i::size_type j = 0; j < data.size(); ++j){
//                somaGrowCurveSet.push_back(data[j](1));
//            }
//        }
//    }
//    std::vector<int> tmpSet = somaGrowCurveSet;
//    somaGrowCurveSet.swap(tmpSet);
}

void NeuroTreeCluster::SearchTreeFromSoma(const MatXi &rawPathGraph, const int somaID,
                                           const int threv, std::vector<int>& somaGrowCurveSet)
{
    int i = 0;
    MatXi pathGraph = rawPathGraph;
    std::vector<VectorVec2i> somaConnectSet;
    std::vector<int> somaConnectID;
    VectorVec2i aas;
    Vec2i tmpAAS;
    for(int j = 0 ; j < pathGraph.rows(); ++j){
        if(pathGraph(somaID, j) == 1){
            somaConnectID.push_back(j);
        }
    }
    aas.clear();
    //aas.resize(somaConnectID.size());
    for(std::vector<int>::size_type j = 0; j < somaConnectID.size();++j){
        tmpAAS << somaID, somaConnectID[j];
        aas.push_back(tmpAAS);
    }

    if(!somaConnectID.empty()){
        somaConnectSet.push_back(aas);
        pathGraph.row(somaID).setZero();// *= 0;
        pathGraph.col(somaID).setZero();// *= 0;
        i=1;
    } else{
        VectorVec2i tmp;
        tmp.push_back(Vec2i(somaID,0));
        somaConnectSet.push_back(tmp);
    }


    if(!aas.empty()){
        MatXi pathGraphCopy;
        for(i = 1; i < threv; ++i){//global varient i
            SearchAndBreakTree(somaConnectSet[i - 1], pathGraph, aas, pathGraphCopy);
            pathGraph = pathGraphCopy;
            if(!aas.empty()){
                somaConnectSet.push_back(VectorVec2i());
                somaConnectSet[somaConnectSet.size() - 1].swap(aas);
            }
            else break;
        }
        --i;//warning!
    } else{
        i = 0;
    }

//    if(i > 0){

//    }
    somaGrowCurveSet.clear();
    const VectorVec2i &data1 = somaConnectSet[0];
    somaGrowCurveSet.push_back(data1[0](0));
    //printf("somaConnectSet.size=%d i = %d",somaConnectSet.size(), i);
    //if(somaConnectSet.size() <= i || std::abs(somaConnectSet.size() - i) > 1)
        //printf("error here!\n");
    for(int ii = 0; ii <= i; ++ii){
        const VectorVec2i &data = somaConnectSet[ii];
        if(!data.empty()){
            for(VectorVec2i::size_type j = 0; j < data.size(); ++j){
                somaGrowCurveSet.push_back(data[j](1));
            }
        }
    }
    std::vector<int> tmpSet = somaGrowCurveSet;
    somaGrowCurveSet.swap(tmpSet);
}

void NeuroTreeCluster::SearchAndBreakTree(const VectorVec2i &aSomaConnectSet,
                                           const MatXi &pathGraph,
                                           VectorVec2i &resultSet,
                                           MatXi &pathGraphCopy)
{
    resultSet.clear();
    pathGraphCopy = pathGraph;
    std::vector<int> somaID;
    for(std::vector<int>::size_type i = 0; i < aSomaConnectSet.size(); ++i){
        somaID.push_back(aSomaConnectSet[i](1));
    }
    //int nx = pathGraphCopy.rows();

    //int ss = 1;

    std::vector<int> idex;
    for(std::vector<int>::size_type i = 0; i < somaID.size(); ++i){
        idex.clear();
        for(int j = 0; j < pathGraphCopy.rows(); ++j){
            if(pathGraphCopy(somaID[i], j) ==1){
                idex.push_back(j);
            }
        }
        //int ns = idex.size();
        Vec2i tmp;
        for(std::vector<int>::size_type k = 0; k < idex.size(); ++k){
            tmp << somaID[i], idex[k];
            resultSet.push_back(tmp);
        }
        pathGraphCopy.row(somaID[i]).setZero();// *= 0;
        pathGraphCopy.col(somaID[i]).setZero();// *= 0;
    }
}

void NeuroTreeCluster::CheckUniqueCurveInTree(const MatXi &breakPathGraph,
                                               const std::vector<int> &somaID,
                                               const std::vector<int> &somaGrowSet,
                                               std::vector<int> &assignedDendIDSet)
{
    //int nxx = int(breakPathGraph.rows());
    int nxxs = somaGrowSet.size();
    //printf("somaGrowSet=%d\n", nxxs);
//#pragma omp parallel for
    for(int i = 0; i < nxxs; ++i){
        bool hasSoma=false;
        bool isSoma =false;
        //std::vector<int> newSomaGrowSet;
        //printf("SearchTreeFromSoma 50 =%d of %d\n", i, nxxs);
        for(size_t k = 0; k < somaID.size(); ++k){
            if(somaID[k] == somaGrowSet[i]){
                isSoma = true;
                break;
            }
        }
        if(!isSoma){
            SearchLargeTreeFromSoma(breakPathGraph, somaGrowSet[i], somaID, 20, hasSoma);
        }else{
            hasSoma = true;
        }
        //std::vector<int> cc;
        //std::vector<int> scc;
//        printf("InterSection =%d of %d\n", i, nxxs);
//        Intersection(newSomaGrowSet, somaID, cc, scc);
//        if(cc.empty()){
        if(!hasSoma){
//#pragma omp critical
            //{
           assignedDendIDSet.push_back(somaGrowSet[i]);
            //}
        }
    }
//    int nxx = int(breakPathGraph.rows());
//    int nxxs = somaGrowSet.size();
//    assignedDendIDSet.resize(nxx, 0);
//    for(int i = 0; i < nxxs; ++i){
//        for (int j=0; j < 50; ++j){
//            std::vector<int> LLs;
//            std::vector<int> cc, scc;
//            SearchTreeFromSoma(breakPathGraph,somaGrowSet[i], somaID,j,LLs);
//            Intersection(LLs, somaID, cc,scc);
//            if(cc.empty()){
//                assignedDendIDSet[i]=
//            }
//        }
//    }
}

void NeuroTreeCluster::Intersection(const std::vector<int> &aa, const std::vector<int> &bb,
                                     std::vector<int> &cc, std::vector<int> &scc)
{
    int nxx = bb.size();
    cc.clear();
    scc.clear();
    for(int i =0; i < nxx; ++i){
        int idex = 0;
        for(std::vector<int>::size_type j = 0; j < aa.size(); ++j){
            if(bb[i] == aa[j]){
                idex = 1;
                break;
            }
        }
        if(idex){
            scc.push_back(i);
            cc.push_back(bb[i]);
        }
    }
}

void NeuroTreeCluster::FindBridge(const VectorMat2i &dendSomaInfo,
                                   const std::vector<int> &currentDendCurveClustre,
                                   const std::vector<int> &unAssignedList,
                                   const std::vector<std::vector<int> > &allAssignedDendIDSet,
                                   MatXi &pathGraph1, VectorVec3i &possibleConInfo)
{
    int nxx = allAssignedDendIDSet.size();
    int nxy = unAssignedList.size();
    if(pathGraph1.rows() != nxx + nxy  || pathGraph1.cols() != nxx + nxy){
        pathGraph1.resize(nxx + nxy, nxx + nxy);
    }
    pathGraph1.setZero();
    for(int i = 0; i < nxx; ++i){
        std::vector<int> aSomaTree;
        const std::vector<int>& curAllAssignedDendIDSet = allAssignedDendIDSet[i];
        for(std::vector<int>::size_type j = 0; j < curAllAssignedDendIDSet.size(); ++j){
            aSomaTree.push_back(currentDendCurveClustre[curAllAssignedDendIDSet[j]]);
        }
        for(int j = 0; j < nxy; ++j){
            const Mat2i& unassignedConInfo = dendSomaInfo[currentDendCurveClustre[unAssignedList[j]] - 1];
            int flag;
            std::vector<int> tmpp;
            tmpp.push_back(unassignedConInfo(0,0));
            tmpp.push_back(unassignedConInfo(0,1));
            AttributeUnassignedToPossibleTree(tmpp, aSomaTree, flag);
            if(flag ==1){
                pathGraph1(i, j + nxx) = 1;
                pathGraph1(j + nxx, i) = 1;
            }
        }
    }
    std::vector<int> allUnassignedID;
    for(std::vector<int>::size_type i = 0; i < unAssignedList.size(); ++i){
        allUnassignedID.push_back(currentDendCurveClustre[unAssignedList[i]]);
    }

    for(int i = 0; i < nxy; ++i){
        for(int j = i + 1; j < nxy; ++j){
            const Mat2i& unassignedConInfo = dendSomaInfo[currentDendCurveClustre[unAssignedList[i]] - 1];
            const Mat2i& bb = dendSomaInfo[currentDendCurveClustre[unAssignedList[j]] - 1];
            if(unassignedConInfo(0,0) == allUnassignedID[j] || unassignedConInfo(0,1)==allUnassignedID[j]
                    || bb(0,0) == allUnassignedID[i] || bb(0,1) == allUnassignedID[i]){

                pathGraph1(i + nxx, j + nxx) = 1;
                pathGraph1(j+nxx,i+nxx)=1;
            }
        }
    }

    for(int i = 0; i < nxx + nxy; ++i){
        for(int j = 0; j < nxx + nxy;++j){
            if(pathGraph1(i,j) == 0){
                pathGraph1(i,j) = std::numeric_limits<int>::max();
            }
        }
    }

    int nss = allUnassignedID.size();
    possibleConInfo.clear();
    for(int ii = 0; ii < nss; ++ii){
        const Mat2i& unassignedConInfo = dendSomaInfo[currentDendCurveClustre[unAssignedList[ii]] - 1];
        Vec3i ttk;ttk.setZero();
        if(unassignedConInfo(0,0) > 0 && unassignedConInfo(0,1)>0){
            ttk << 1,0,0;
            std::vector<int> idexss;
            for(int j = 0; j < pathGraph1.rows(); ++j){
                if(pathGraph1(ii+nxx,j)==1){
                    idexss.push_back(j);
                }
            }
            if(!idexss.empty()){
                std::vector<int> tmp;
                for(int k = 0; k < pathGraph1.rows(); ++k){
                    tmp.push_back(k);
                }
                int fir;
                FindPossibleConnect(unassignedConInfo(0,0), currentDendCurveClustre, unAssignedList,
                                    allAssignedDendIDSet, tmp, fir);
                int sec;
                FindPossibleConnect(unassignedConInfo(0,1), currentDendCurveClustre, unAssignedList,
                                    allAssignedDendIDSet, tmp, sec);
                ttk << 1, fir + 1, sec + 1;
            }
        }
        possibleConInfo.push_back(ttk);
    }
}

void NeuroTreeCluster::AttributeUnassignedToPossibleTree(const std::vector<int> &unassignedDendID,
                                                          const std::vector<int> &aSomaTree,
                                                          int &connectFlag)
{
    int nx = unassignedDendID.size();
    connectFlag = 0;
    int nmm = aSomaTree.size();
    for(int i = 0; i < nx; ++i){
        for(int j = 0; j < nmm; ++j){
            if(unassignedDendID[i] == aSomaTree[j]){
                connectFlag = 1;
            }
        }
    }
}

void NeuroTreeCluster::FindPossibleConnect(const int conjDendID,
                                            const std::vector<int> &currentDendCurveClustre,
                                            const std::vector<int> &unAssignedList,
                                            const std::vector<std::vector<int> > &allAssignedDendIDSet,
                                            const std::vector<int> &idexss, int &possibleConInfo)
{
    int nDD = allAssignedDendIDSet.size();
    std::vector<int> L1, L2;
    for(std::vector<int>::size_type i = 0; i < idexss.size(); ++i){
        if(idexss[i] < nDD)
            L1.push_back(idexss[i]);
    }
    for(std::vector<int>::size_type i = 0; i < idexss.size(); ++i){
        if(idexss[i] >= nDD)
            L2.push_back(idexss[i] - nDD);
    }

    int flag = 0;
    if(!L2.empty()){
        std::vector<int> unassignedDend;
        for(std::vector<int>::size_type i = 0; i<L2.size();++i){
            unassignedDend.push_back(currentDendCurveClustre[unAssignedList[L2[i]]]);
        }
        int nlxx = unassignedDend.size();
        for(int i = 0; i < nlxx;++i){
            if(conjDendID == unassignedDend[i]){
                flag = 1;
                possibleConInfo = nDD + i;
            }
        }
    }

    if(flag == 0 && !L1.empty()){
        int nl1 = L1.size();
        for(int i = 0; i < nl1; ++i){
            std::vector<int> assignedDend;
            std::vector<int> tmp = allAssignedDendIDSet[L1[i]];
            for(std::vector<int>::size_type j = 0; j < tmp.size(); ++j){
                assignedDend.push_back(currentDendCurveClustre[tmp[j]]);
            }
            int conFlag;
            AttributeUnassignedToPossibleTree(std::vector<int>(1,conjDendID), assignedDend,
                                              conFlag);
            if(1 == conFlag){
                possibleConInfo = L1[i];
            }
        }
    }
}

void NeuroTreeCluster::MapDendWithHeadTailToCompressMat(const std::vector<int> &currentUnassignNet,
                                                         const std::vector<int> &currentUnassignNetConInfo,
                                                         const MatXi &pathGraph1,
                                                         const VectorVec3i &dendIDListWithHeadTail,
                                                         const int assignedDendNum,
                                                         MatXi &compressPathGraph1,
                                                         VectorVec3i &validDendIDListWithHeadTail,
                                                         std::vector<int> &currentUnNetInOrigPath)
{
    validDendIDListWithHeadTail.clear();
    currentUnNetInOrigPath.resize(currentUnassignNetConInfo.size() + currentUnassignNet.size());
	for (int i = 0; i < currentUnassignNetConInfo.size(); i++)
	{
		currentUnNetInOrigPath[i] = currentUnassignNetConInfo[i];
	}
    /*std::copy(currentUnassignNetConInfo.begin(), currentUnassignNetConInfo.end(),
              currentUnNetInOrigPath.begin());*/
    std::copy(currentUnassignNet.begin(), currentUnassignNet.end(),
              currentUnNetInOrigPath.begin() + currentUnassignNetConInfo.size());

    compressPathGraph1.resize(currentUnNetInOrigPath.size(),currentUnNetInOrigPath.size());
    compressPathGraph1.setZero();
    for(std::vector<int>::size_type i = 0; i < currentUnNetInOrigPath.size(); ++i){
        for(std::vector<int>::size_type j = 0; j < currentUnNetInOrigPath.size(); ++j){
            compressPathGraph1(i,j) =
                    pathGraph1(currentUnNetInOrigPath[i], currentUnNetInOrigPath[j]);
        }
    }
    std::vector<int> idexxss;
    int flag;
    for(std::vector<int>::size_type i = 0; i < currentUnassignNet.size();++i){
        flag = dendIDListWithHeadTail[currentUnassignNet[i]-assignedDendNum](0);
        if(1 == flag){
            idexxss.push_back(currentUnassignNet[i]-assignedDendNum);
        }
    }
    if(!idexxss.empty()){
        for(std::vector<int>::size_type i = 0; i < idexxss.size();++i){
            validDendIDListWithHeadTail.push_back(dendIDListWithHeadTail[idexxss[i]]);
            idexxss[i] += assignedDendNum;
        }
    } else {
        validDendIDListWithHeadTail.clear();
    }

    VectorVec3i validDendIDListWithHeadTailCopy;
    validDendIDListWithHeadTailCopy.resize(idexxss.size());
     for (int kk = 0; kk < validDendIDListWithHeadTailCopy.size(); ++kk) {
    validDendIDListWithHeadTailCopy[kk].setZero();
    }
    int nxx = currentUnNetInOrigPath.size();
    if(!validDendIDListWithHeadTail.empty()){
        //to match with matlab, k+1
        for(std::vector<int>::size_type j = 0; j < idexxss.size();++j){
			for(int k = 0; k < nxx; ++k){
				if(idexxss[j] == currentUnNetInOrigPath[k])
					validDendIDListWithHeadTailCopy[j](0) = k + 1;
			}
			if(validDendIDListWithHeadTailCopy[j](0) == 0){
				printf("hello1\n");
			}
		}
		for(std::vector<int>::size_type j = 0; j < idexxss.size();++j){
			for(int k = 0; k < nxx; ++k){
				if(validDendIDListWithHeadTail[j](1) == currentUnNetInOrigPath[k] + 1)
					validDendIDListWithHeadTailCopy[j](1) = k + 1;
			}
			if(validDendIDListWithHeadTailCopy[j](1) == 0){
				printf("hello2\n");
			}
		}
		for(std::vector<int>::size_type j = 0; j < idexxss.size();++j){
			for(int k = 0; k < nxx; ++k){
				if(validDendIDListWithHeadTail[j](2) == currentUnNetInOrigPath[k] + 1)
				validDendIDListWithHeadTailCopy[j](2) = k + 1;
			}
			if(validDendIDListWithHeadTailCopy[j](2) == 0){
					printf("hello3\n");
			}
        }
  //      std::vector<int> LJJ;
  //      //LJJ=[idexxss,validDendIDListWithHeadTail(2,:),validDendIDListWithHeadTail(3,:)];
  //      std::copy(idexxss.begin(), idexxss.end(), std::back_inserter(LJJ));
  //      for (int kkk = 0; kkk < validDendIDListWithHeadTail.size(); ++kkk) {
  //          LJJ.push_back(validDendIDListWithHeadTail[kkk](1));
  //      }
  //      for (int kkk = 0; kkk < validDendIDListWithHeadTail.size(); ++kkk) {
  //          LJJ.push_back(validDendIDListWithHeadTail[kkk](2));
  //      }
  //      std::vector<int> LJJ2(LJJ.size());
  //      int Nxx = currentUnNetInOrigPath.size();
  //      int kkL = -1;
  //      for (int j=0; j < LJJ2.size(); ++j){
  //          for(int  i=0; i < Nxx; ++i){
  //              if (LJJ[j]== (currentUnNetInOrigPath[i]+1)){
  //                  ++kkL;
  //                  LJJ2[kkL] = i + 1;
  //              }
  //          }
  //      }
		//if (kkL < LJJ.size()-1)
		//{
		//	printf("kkL: %d LJJ:%d here's a terrible error!\n", kkL, int(LJJ.size()));
		//	//system("pause");
		//}
  //      for (int i = 0; i < validDendIDListWithHeadTail.size(); ++i) {
  //          validDendIDListWithHeadTail[i](0) = LJJ2[i];
  //          validDendIDListWithHeadTail[i](1) = LJJ2[i+idexxss.size()];
  //          validDendIDListWithHeadTail[i](2) = LJJ2[i + 2 * idexxss.size()];
  //      }
    }
    validDendIDListWithHeadTail.swap(validDendIDListWithHeadTailCopy);
}

void NeuroTreeCluster::BreakSomaTreeUnassignedNet(const MatXi &compressPathGraph1, const int subNetNum,
                                                   const VectorVec3i &validDendIDListWithHeadTail,
                                                   const std::vector<int> &currentDendID,
                                                   const std::vector<VectorVec5d> &rawDendList,
                                                   const SVolume &origImg, MatXi &newCompressGraph)
{
    newCompressGraph = compressPathGraph1;
    std::vector<int> validDendID;
    for(std::vector<int>::size_type i = 0; i < validDendIDListWithHeadTail.size();++i){
        validDendID.push_back(validDendIDListWithHeadTail[i](0));
    }
    std::vector<int> wetList;
    wetList.resize(validDendID.size(), 1);
    for(std::vector<int>::size_type i = 0; i < validDendID.size(); ++i){
        int validDendIDListWithHeadTailCp;
        std::vector<int> tmpWetList;
		std::vector<int> tmp;
		for(int kk = 0; kk < validDendIDListWithHeadTail.size(); ++kk){
			tmp.push_back(validDendIDListWithHeadTail[kk](0));
		}
        GetNetBridge(newCompressGraph, subNetNum, tmp, wetList,
                     validDendIDListWithHeadTailCp, tmpWetList);
        if(validDendIDListWithHeadTailCp > -1){//matlab is 0
            wetList=tmpWetList;
            int tmpID = currentDendID[validDendIDListWithHeadTailCp];
            int flag;
            JudgeNetBridgeDirection(rawDendList[tmpID - 1], 0, origImg,
                                    flag);
            if(0 == flag){
				//continue;
                int axx = validDendIDListWithHeadTail[validDendIDListWithHeadTailCp](1) - 1;
				//if(axx == -1) continue;
                newCompressGraph(axx,validDendID[validDendIDListWithHeadTailCp] - 1)
                        =std::numeric_limits<int>::max();
                newCompressGraph(validDendID[validDendIDListWithHeadTailCp] - 1, axx)
                        =std::numeric_limits<int>::max();
            } else{
				//continue;
                int axx = validDendIDListWithHeadTail[validDendIDListWithHeadTailCp](2) - 1;
				//if(axx == -1) continue;
                newCompressGraph(axx,validDendID[validDendIDListWithHeadTailCp] - 1)
                        =std::numeric_limits<int>::max();
                newCompressGraph(validDendID[validDendIDListWithHeadTailCp] - 1, axx)
                        =std::numeric_limits<int>::max();
            }
        } else{
            break;
        }
    }
}

void NeuroTreeCluster::GetNetBridge(const MatXi &compressPathGraph1, const int subNetNum,
                                     const std::vector<int> &validDendIDListWithHeadTail,
                                     const std::vector<int> &wetList, int &validIDListWithHeadTail,
                                     std::vector<int> &resultWetList)
{
    resultWetList=wetList;
    std::vector<int> dendWetList;
    dendWetList.resize(validDendIDListWithHeadTail.size(), 0);
    for(int i = 0; i < subNetNum - 1; ++i){
        for(int j = i + 1; j < subNetNum; ++j){
            int dist;
            std::vector<int> path;
            MyDijkstra(compressPathGraph1, i,j, dist, path);
            for(std::vector<int>::size_type k = 0; k < path.size(); ++k)
                ++path[k];
            std::vector<int> tmpList;
            CalcBridgeWet(path, validDendIDListWithHeadTail, tmpList);
            for(std::vector<int>::size_type k = 0; k < dendWetList.size();++k){
                dendWetList[k] += tmpList[k];
            }
        }
    }
    int tmpSum = 0;
    for(std::vector<int>::size_type k = 0; k < dendWetList.size();++k){
        tmpSum += dendWetList[k];
    }
    if(tmpSum > 0){
        std::vector<int> tmpList;
        for(std::vector<int>::size_type i = 0; i < dendWetList.size(); ++i){
            tmpList.push_back(dendWetList[i]*resultWetList[i]);
        }
        std::vector<int>::iterator maxIt= std::max_element(tmpList.begin(), tmpList.end());
        int idexxm = std::distance(tmpList.begin(), maxIt);
        validIDListWithHeadTail=idexxm;
        resultWetList[idexxm] = 0;
    } else{
        validIDListWithHeadTail = -1;
    }
}

void NeuroTreeCluster::MyDijkstra(const MatXi &graph, const int beg, const int fin, int &mydist, std::vector<int> &mypath)
{
    int sz = graph.rows();
    std::vector<int> path;
    path.resize(sz, 0);
    std::vector<int> d;
    d.resize(sz,0);
    std::vector<int> mark;
    mark.resize(sz,0);
    for(int i = 0; i < sz;++i){
        d[i]=graph(beg, i);
        path[i]=beg;
    }
    mark[beg]=1;
    path[beg]=0;
    d[beg]=0;
	std::vector<int> ww;
	bool stopFlag = false;
    for(int i = 0; i < sz; ++i){
        if(1 == mark[i]) continue;
		ww.clear();
        int minc = std::numeric_limits<int>::max();
        int w = -1;
        for(int j = sz - 1; j >= 0; --j){
            if(mark[j] == 0 && 100 > d[j]){//std::numeric<int>::max()
                minc = d[j];
				ww.push_back(j);
            }
        }
		for(int k = 0; k < ww.size(); ++k){
			w = ww[k];
			if (w > -1){
				mark[w] = 1;
				for(int j = 0; j < sz; ++j){
					if(mark[j] == 0 && graph(w,j) != std::numeric_limits<int>::max()
							&& d[j] > d[w] + graph(w,j)){
						d[j]=d[w]+graph(w, j);
						path[j] = w;
					}
				}
				if(fin == w){
					stopFlag = true;
				}
			}
			if(stopFlag) break;
		}
    }
    mydist = d[fin];
    if (mydist == std::numeric_limits<int>::max() || path[fin] == beg){
        mypath.clear();
    } else{
        int i = fin;
        mypath.clear();
        while(i != beg){
            mypath.push_back(i);
            i = path[i];
        }
        mypath.push_back(beg);
        if(mypath.size() <3){
            mypath.clear();
        }
        std::reverse(mypath.begin(), mypath.end());
    }
}

void NeuroTreeCluster::CalcBridgeWet(const std::vector<int> &path,
                                      const std::vector<int> &validDendIDListWithHeadTail,
                                      std::vector<int> &resultConnectList)
{
    int nx = validDendIDListWithHeadTail.size();
    int np = path.size();
    resultConnectList.resize(nx, 0);
    for(int i = 0; i < np; ++i){
        for(int j =0; j < nx; ++j){
            if(path[i] == validDendIDListWithHeadTail[j]){
                ++resultConnectList[j];
            }
        }
    }
}

void NeuroTreeCluster::JudgeNetBridgeDirection(const VectorVec5d &dendList, const int threv,
                                                const SVolume &origImg, int &directionFlag)
{
    if(threv > 0) directionFlag = 1;
    if(threv < 0) directionFlag = 0;
    if(0 == threv){
        VectorVec5d tmpDendList;
        Vec5d tmp;
        for(VectorVec3d::size_type i=3; i < dendList.size() - 4; ++i){
            tmp << dendList[i](0), dendList[i](1), dendList[i](2),
                   dendList[i](3), dendList[i](4);
            tmpDendList.push_back(tmp);
        }
        MatXd initHeadRadWet(2,4), initTailRadWet(2,4);
        initHeadRadWet.setZero();
        initTailRadWet.setZero();
        CalcRayBurstRadWet(tmpDendList, origImg,
                           initHeadRadWet, initTailRadWet);
        double r0 = initHeadRadWet.row(0).mean();
        double r1 = initTailRadWet.row(0).mean();
        double v0 = initHeadRadWet.row(1).mean();
        double v1 = initTailRadWet.row(1).mean();
        double v = std::max<double>(std::min<double>(v0, v1), 1);
        double s0 = r0 + 0.5 * v0 / v;
        double s1 = r1 + 0.5 * v1 / v;
        if(s0 > s1) directionFlag = 1;
        else directionFlag = 0;
    }
}

void NeuroTreeCluster::GetBoundaryBack(const std::vector<double> &outer_shell, const double threv, std::vector<double> &boundary_back)
{
    size_t nxx = outer_shell.size();
    boundary_back.clear();
    std::vector<double> adjust_shell;
    double tmp;
    double third = 1.0/3.0;
    for (size_t i = 0; i < nxx; ++i){
        tmp = outer_shell[i] < 400.0 ? outer_shell[i] : 400.0;
        adjust_shell.push_back(tmp);
    }

    for (size_t i = 0; i < 100; ++i){
        for (size_t j = 1; j < nxx - 1; ++j){
            adjust_shell[j] = third * (adjust_shell[j - 1] + adjust_shell[j] + adjust_shell[j + 1]);
        }

        adjust_shell[nxx - 1] = 0.5 * (adjust_shell[nxx - 1] + adjust_shell[nxx - 2]);
        adjust_shell[0] = 0.5 * (adjust_shell[0] + adjust_shell[1]);
    }

    boundary_back = outer_shell;

    for (size_t i = 0; i < nxx; ++i){
        if (std::abs(outer_shell[i]-adjust_shell[i]) > threv * std::sqrt(adjust_shell[i])){
            boundary_back[i] = adjust_shell[i];
        }
    }
}

void NeuroTreeCluster::GetRayLimit(const Volume<double> &sphere_ray_wet, const double constriction_threv, std::vector<std::vector<double> > &ray_limit)
{
    int nx = sphere_ray_wet.x();
    int ny = sphere_ray_wet.y();
    int nz = sphere_ray_wet.z();
    std::vector<double> sphere_ray;
    std::vector<double> tmpUzz;
    sphere_ray.clear();
    int arra(0);

    for (int i = 0; i < ny; ++i){
        tmpUzz.clear();
        for (int j = 0; j < nz; ++j){
            for (int ij = 0; ij < nx; ++ij){
                sphere_ray.push_back(sphere_ray_wet(ij, i, j));
            }

            CalculateOneRayLimit(sphere_ray, constriction_threv, arra);
            tmpUzz.push_back(arra);
            sphere_ray.clear();
        }
        ray_limit.push_back(tmpUzz);
    }
}

void NeuroTreeCluster::GetGradientVectorFlowForTrace(const Volume<double> &sphere_ray_wet, Volume<double> &smooth_ray)
{
    smooth_ray.SetSize(sphere_ray_wet.x(), sphere_ray_wet.y(), sphere_ray_wet.z());

    std::vector<double> S_diffttk;

    for (int i = 0; i < sphere_ray_wet.y(); ++i){//角度1
        for (int j = 0; j < sphere_ray_wet.z(); ++j){//角度2
            std::vector<double> tmpSphere;
            S_diffttk.clear();
            for (int ij = 0; ij < sphere_ray_wet.x(); ++ij){//长度
                tmpSphere.push_back((double)sphere_ray_wet(ij,i,j));
            }

            SmoothGradientCurvesForTrace(tmpSphere, S_diffttk);

            for (size_t ij = 0; ij < S_diffttk.size(); ++ij){
                smooth_ray(ij,i,j) = S_diffttk[ij];
            }
        }
    }
}

void NeuroTreeCluster::SmoothGradientCurvesForTrace(const std::vector<double> &init_one_ray_wet, std::vector<double> &smooth_one_ray_wet)
{
    smooth_one_ray_wet.clear();
    std::vector<double> one_ray_wet(init_one_ray_wet);
    std::vector<double>::size_type n_ttk = one_ray_wet.size();
    //ttk = min(ttk,600);
    for (size_t i = 0; i < n_ttk; ++i){
        one_ray_wet[i] = one_ray_wet[i] < 600.0 ? one_ray_wet[i] : 600.0;
    }

    std::vector<double> tmp_diff;//原始梯度
    //diff_vector<double>(one_ray_wet, tmp_diff);//interp1
    tmp_diff.clear();
    for (std::vector<double>::size_type i = 0; i < one_ray_wet.size() - 1; ++i){
        tmp_diff.push_back(one_ray_wet[i + 1] - one_ray_wet[i]);
    }
    std::vector<double> diff_ray;

    //NG_Util::Interpl_2_Mean(tmp_diff, diff_ray);//插值
    diff_ray.clear();
    for (std::vector<double>::size_type i = 0; i < tmp_diff.size() - 1; ++i){
        diff_ray.push_back(tmp_diff[i]);
        diff_ray.push_back((tmp_diff[i+1] + tmp_diff[i]) / 2.0);
    }
    diff_ray.push_back(tmp_diff.back());


    int n_diff_ttk = diff_ray.size();

    std::vector<double> tmpone;
    tmpone.resize(one_ray_wet.size() - 1);
    std::copy(one_ray_wet.begin(), one_ray_wet.end() - 1, tmpone.begin());
    std::vector<double> interpl_ray;
    //NG_Util::Interpl_2_Mean(tmpone, interpl_ray);
    interpl_ray.clear();
    for (std::vector<double>::size_type i = 0; i < tmpone.size() - 1; ++i){
        interpl_ray.push_back(tmpone[i]);
        interpl_ray.push_back((tmpone[i+1] + tmpone[i]) / 2.0);
    }
    interpl_ray.push_back(tmpone.back());

    interpl_ray.push_back(one_ray_wet.back());

    //vector<double> yy(diff_ray);
    std::vector<double> buffer_ray;// = yy;
    //yy1 = max(-yy, 0);
    double tmpYY;
    for (std::vector<double>::size_type i = 0; i < diff_ray.size(); ++i){
        tmpYY = -diff_ray[i] > 0.0 ? -diff_ray[i] : 0.0;
        tmpYY *= -100.0 / interpl_ray[1 + i];
        buffer_ray.push_back( tmpYY );
    }

    /*平滑梯度*/
    for (int j = 1; j < 101; ++j){//只是循环100次
        for (int i =1; i < n_diff_ttk - 1; ++i){
            buffer_ray[i] = ((std::abs)(diff_ray[i]) * diff_ray[i]
            + 2.0 * (diff_ray[i-1]+diff_ray[i+1])) / ((std::abs)(diff_ray[i]) + 4.0);
            diff_ray = buffer_ray;
        }
    }

    std::vector<int> tts;
    //generate_n(back_inserter(tts), (int)(diff_ray.size() / 2) + 1,GenArray<int>(0, 2));
    for(int i = 0; i < (int)(diff_ray.size() / 2) + 1; ++i){
        tts.push_back(i * 2);//warning this bug
    }

    for (std::vector<int>::size_type i = 0; i < tts.size(); ++i){
        smooth_one_ray_wet.push_back(diff_ray[tts[i]]);
    }

    for (size_t i = 0; i < smooth_one_ray_wet.size(); ++i){
        smooth_one_ray_wet[i] = smooth_one_ray_wet[i] > 0.0? 0.0 : (-smooth_one_ray_wet[i]);
    }
}

void NeuroTreeCluster::CalculateOneRayLimit(const std::vector<double> &ray, const double constriction_threv, int &one_ray_limit)
{
    size_t nx = ray.size();
    one_ray_limit = 0;

    for (size_t i = 0; i < nx; ++i){
        if (ray[i] < constriction_threv){
            break;
        }
        else{
            ++one_ray_limit;
        }
    }
}

//different from BridgeBreaker
void NeuroTreeCluster::CalcRayBurstRadWet(const VectorVec5d &noHeadTailDendList, const Volume<unsigned short> &origImg,
                                 MatXd &initHeadRadWet, MatXd &initTailRadWet)
{//2014-3-24-21-06
    int dendListLength = noHeadTailDendList.size();
    int headFirstValidNodeId = 0;
    int nx = origImg.x();
    int ny = origImg.y();
    int nz = origImg.z();
    for(int i = 0; i < dendListLength; ++i){
        Vec3d currentDendPt(noHeadTailDendList[i](0), noHeadTailDendList[i](1), noHeadTailDendList[i](2));
        //be care
        if(currentDendPt.minCoeff() > 1.0 && currentDendPt(0) < nx -3 && currentDendPt(1) < ny -3
           && currentDendPt(2) < nz -2){
            headFirstValidNodeId = i;
            break;
        }
    }
    //in reverse first pt which is not in boundary
    int tailFirstValidNodeId = 0;
    for(int i = 0; i < dendListLength; ++i){
        Vec3d currentDendPt(noHeadTailDendList[dendListLength - i - 1](0), noHeadTailDendList[dendListLength - i - 1](1),
                noHeadTailDendList[dendListLength - i - 1](2));
        if(currentDendPt.minCoeff() > 1.0 && currentDendPt(0) < nx -3 && currentDendPt(1) < ny -3
           && currentDendPt(2) < nz -2){
            tailFirstValidNodeId = dendListLength - i - 1;//warning : may problem
            break;
        }
    }
    //valid dend length is up the threshold
    if(tailFirstValidNodeId - headFirstValidNodeId > 8){
        VectorVec3d noHeadTailDendListCopy;
        for(int i = headFirstValidNodeId; i <= tailFirstValidNodeId ; ++i){
            noHeadTailDendListCopy.push_back(Vec3d(noHeadTailDendList[i](0), noHeadTailDendList[i](1), noHeadTailDendList[i](2) ));
        }
        int dendListLengthCopy = noHeadTailDendListCopy.size();
        //2014-3-27-11-02
        Vec3d mainDirection1;//, mainDirection2, mainDirection3;
        VectorVec3d tmpLine;
        tmpLine.resize(std::min(8, dendListLengthCopy));
        std::copy(noHeadTailDendListCopy.begin(), noHeadTailDendListCopy.begin() + tmpLine.size(), tmpLine.begin());
        Principald(tmpLine, mainDirection1);
        initHeadRadWet.setZero();
        for(int i = 0; i < 4; ++ i){
            Vec2d tmpRadWet;
            CalcRayBurstOnePtRadWet(noHeadTailDendListCopy[i], origImg, mainDirection1, tmpRadWet);
            initHeadRadWet(0, i) = tmpRadWet(0);
            initHeadRadWet(1, i) = tmpRadWet(1);
        }

        initTailRadWet.setZero();
        tmpLine.clear();
        for(int i = dendListLengthCopy - 1; i >= std::min(0, dendListLengthCopy - 9); --i){
            tmpLine.push_back(noHeadTailDendListCopy[i]);
        }
        Principald(tmpLine, mainDirection1);
        for(int i = 0; i < 4; ++ i){
            Vec2d tmpRadWet;
            CalcRayBurstOnePtRadWet(noHeadTailDendListCopy[dendListLengthCopy - i - 1], origImg, mainDirection1, tmpRadWet);
            initTailRadWet(0, i) = tmpRadWet(0);
            initTailRadWet(1, i) = tmpRadWet(1);
        }
    }
    else{
        initHeadRadWet.setZero();
        initTailRadWet.setZero();
    }
}

void NeuroTreeCluster::CalcRayBurstOnePtRadWet(const Vec3d &curvePt, const Volume<unsigned short> &origImg, const Vec3d &preDirection, Vec2d &ptRadWet)
{
    //const Vec3d &L_data = curvePt;
    double slice = 0.3;//(double)(minLen) / 82.0;
    const int blocksum = 26;//41;
    const int Theta = 20, Phi = 10;
    //int subx = origImg.x();
    //int suby = origImg.y();
    //int subz = origImg.z();
    std::vector<double> raySlice;
    for(int i = 0; i < blocksum; ++i){
        raySlice.push_back(slice * i);
    }

    /*这里是求轮廓区域*/
    Volume<double> sphereRayWet;
    sphereRayWet.SetSize(raySlice.size(), Theta, Phi);
    const double a = 360.0 / (double)Theta;
    const double b = 180.0 / (double)Phi;
    const double PI_180 = M_PI / 180.0;

    //double prev_Density(0.0);
    double rayNodeDense(0.0), rayNodeWet(0.0);
    double x,y,z;

    for (std::vector<double>::size_type k = 0; k < raySlice.size(); ++k)//0
    {
        for (int i = 1; i <= Theta; ++i)
        {
            for (int j = 1; j <= Phi; ++j)
            {
                x = raySlice[k] * std::sin(b * (double)j * PI_180) * std::cos(a * (double)i * PI_180) + curvePt(0);
                y = raySlice[k] * std::sin(b * (double)j * PI_180) * std::sin(a * (double)i * PI_180) + curvePt(1);
                z = raySlice[k] * std::cos(b * (double)j * PI_180) + curvePt(2);
                rayNodeDense = rayNodeWet = 0.0;

                ContourUtil::CalculateSphereOneNode(origImg, 1.0, x, y, z, rayNodeDense, rayNodeWet);

                sphereRayWet(k, i-1, j-1) = (double)(rayNodeDense / (rayNodeWet + 0.0001));
            }
        }
    }//for

    //列存储
    int lenR0_1 = int(raySlice.size()) - 1;
    std::vector<double> outerShell;
    for (int j = 0; j < Phi; ++j){
        for (int i = 0; i < Theta; ++i){
            outerShell.push_back(sphereRayWet(lenR0_1, i, j));
        }
    }

    std::vector<double> boundaryBack;
    GetBoundaryBack(outerShell, 4, boundaryBack);

    //three_vs = mean(Lssx1)+3.5*std(Lssx1)
    double constrictionThrev(0.0);
    double constriction_threv_mean =
        std::accumulate(boundaryBack.begin(), boundaryBack.end(), 0.0) / boundaryBack.size();
    //求标准差---------------------------------------
    double constriction_threv_mean_sqrt_sum(0);
    size_t num_boundary_back = boundaryBack.size();
    for (size_t i = 0; i< num_boundary_back; ++i)
    {
        constriction_threv_mean_sqrt_sum +=
            (boundaryBack[i] - constriction_threv_mean) * (boundaryBack[i] - constriction_threv_mean);
    }
    constrictionThrev = 4.5 * std::sqrt(constriction_threv_mean_sqrt_sum / (boundaryBack.size() - 1));//warning!4.5
    constrictionThrev += constriction_threv_mean;
    for (int i = 0; i < Phi; ++i){
        for (int j = 0; j < Theta; ++j)
            sphereRayWet(lenR0_1, j, i) = boundaryBack[i * Theta + j];
    }


    std::vector<std::vector<double> > rayLimit;
    GetRayLimit(sphereRayWet, constrictionThrev, rayLimit);//three_vs, Uzz
    Volume<double> smoothRay;//Uz
    //    gradientVectorFlowPP_Trace(Sphere_XX, Uz);
    GetGradientVectorFlowForTrace(sphereRayWet, smoothRay);
    //Ux-resultRayLength
    MatXd resultRayLength(smoothRay.x(), smoothRay.y());
    for(int i = 0; i < smoothRay.x(); ++i){
        for(int j = 0; j < smoothRay.y(); ++j){
            resultRayLength(i,j) = smoothRay(i,j,0);
        }
    }


    std::vector<double> lineSegLength;//分段长度
//    generate_n(back_inserter(idexxS), blocksum, GenArray<double>(1.0, 1.0));//注意这里要同步
    for(std::vector<double>::size_type i = 1; i <= raySlice.size(); ++i){
        lineSegLength.push_back(i);
    }

    double currentRayLen(0);
//    vector<double> TT(Uz.x());
//    vector<double> jjt(Uz.x());
    std::vector<double> currentSmoothRay(smoothRay.x());
    std::vector<double> distWet(smoothRay.x());

    double reduce_smooth[2];
    reduce_smooth[0] = 0.9;
    reduce_smooth[1] = 1.0 - reduce_smooth[0];

    int repeat = 10;

    for (int jj = 0; jj < repeat;++jj){
        for (int i = 1; i <= Theta; ++i){
            for (int j = 1; j <= Phi; ++j){
                currentRayLen = resultRayLength(i, j);
                for (int ij = 0; ij < smoothRay.x(); ++ij){
                    currentSmoothRay[ij] = smoothRay(ij, i - 1, j - 1);
                    distWet[ij] = currentSmoothRay[ij] * std::exp( -0.05 *std::pow( lineSegLength[ij] - currentRayLen, 2.0));
                }

                resultRayLength(i,j) += reduce_smooth[0] * ( ( (std::inner_product(lineSegLength.begin(), lineSegLength.end(), distWet.begin(), 0.0)) /
                                                  (std::accumulate(distWet.begin(), distWet.end(), 0.0) ) ) - resultRayLength(i,j) )
                        - reduce_smooth[1] * (4.0 * resultRayLength(i,j) - resultRayLength(i-1,j) - resultRayLength(i+1,j) - resultRayLength(i,j-1) - resultRayLength(i,j+1));
                resultRayLength(i,j) = std::min(resultRayLength(i, j ) , rayLimit[i - 1][j - 1]);

            }
            resultRayLength = (resultRayLength.array() > 0.0).select(resultRayLength, 0.0);
            resultRayLength.row(0) = resultRayLength.row(2);
            resultRayLength.row(Theta + 1) = resultRayLength.row(Theta - 1);
            resultRayLength.col(0) = resultRayLength.col(2);
            resultRayLength.col(Phi + 1) = resultRayLength.col(Phi - 1);
        }
    }//for

    double kk = 0;
    double accLength = 0.0;
    Vec3d diffDirection;//direcs00....wtf
    for(int j = 1; j <= Phi; ++ j){
        for(int i = 1; i <= Theta; ++i){
            diffDirection << std::sin(b * j * PI_180) * std::cos(a * i * PI_180),
                    std::sin(b * j * PI_180) * std::sin(a * i * PI_180),
                    std::cos(b * j * PI_180);
            if(std::abs(diffDirection.dot(preDirection)) < 0.15){
                accLength += 0.3 * std::max(resultRayLength(i,j), 0.0);
                ++kk;
            }
        }
    }
    ptRadWet.setZero();
    VectorVec3d curLinePts;
    curLinePts.push_back(curvePt);
    std::vector<double> aa_data;
    WeighRayValue(curLinePts, origImg, aa_data);
    ptRadWet << std::max(accLength / std::max(1.0, kk), 0.1), aa_data[0];
}

void NeuroTreeCluster::WeighRayValue(const VectorVec3d &rayNode, const Volume<unsigned short> &locOrigImg,
                                std::vector<double> &rayNodeWet)
{
    typedef double spheredist;
    int nxss = rayNode.size();//
    rayNodeWet.clear();

    //坐标
    typedef double spheredist;
    spheredist x,y,z;
    spheredist segmentDense, segmentWet;//, w1;
    for (int i = 0; i < nxss; ++i){
        x = rayNode[i](0);
        y = rayNode[i](1);
        z = rayNode[i](2);
        segmentDense = segmentWet = 0.0;
        ContourUtil::CalculateSphereOneNode(locOrigImg, 0.05, x, y, z, segmentDense, segmentWet);
        rayNodeWet.push_back(segmentDense / (segmentWet + 0.0001));
    }
}

void NeuroTreeCluster::Principald(const VectorVec3d &dataL, Vec3d &x1)
{
    int nx = int(dataL.size());
    Vec3d nn;
    nn.setZero();
    double tmp(0.0);
    Vec3d tmpVec;
    for (int i = 0; i < nx - 1; ++i){
        tmpVec = dataL[i + 1] - dataL[i];
        tmp = tmpVec.norm();
        nn += tmp * tmpVec;
    }
    x1 = nn.normalized();
    //CalcOrthoBasis(x1, x2, x3);
}

void NeuroTreeCluster::ClusterBrokenNet(const MatXi &newCompressGraph,
                                         const std::vector<std::vector<int> > &DD_enlarge,
                                         const std::vector<int> &currentUnNetInOrigPath,
                                         const std::vector<int> &unAssignedList,
                                         const std::vector<int> &currentDendCurveClustre,
                                         const int subNetNum, const int assignedDendNum,
                                         std::vector<std::vector<int> > &DD_enlarge1)
{
    DD_enlarge1=DD_enlarge;
	//std::cout << newCompressGraph <<std::endl;
    for(int i = 0; i < subNetNum; ++i){
        std::vector<int> newNetDendList;
        GetConnectUnassignedNet(newCompressGraph,i, newNetDendList);
        if(newNetDendList.size() > 1){
            std::vector<int> validNewNetDendList;
            for(std::vector<int>::size_type j = 0; j < newNetDendList.size(); ++j){
                if(newNetDendList[j] > subNetNum)
                    validNewNetDendList.push_back(j);
            }
            std::vector<int>  newNetDendListCp;
            for(std::vector<int>::size_type j = 0; j < validNewNetDendList.size(); ++j){
                newNetDendListCp.push_back(newNetDendList[validNewNetDendList[j]]);
            }
            newNetDendList.swap(newNetDendListCp);
            //addDendList=currentDendCurveClustre(unAssignedList(currentUnNetInOrigPath(newNetDendList)-assignedDendNum));
            std::vector<int> addDendList;
            for(std::vector<int>::size_type j = 0; j <newNetDendList.size(); ++j){
                addDendList.push_back(currentDendCurveClustre[unAssignedList[currentUnNetInOrigPath[newNetDendList[j]-1]
                        - assignedDendNum]]);
            }
            int idexCell = currentUnNetInOrigPath[i];
            int sz = DD_enlarge1[idexCell].size();
            //if(DD_enlarge1.size() <= idexCell){
                //printf("error here\n");
                //DD_enlarge1.resize(idexCell +1);
            //}
            DD_enlarge1[idexCell].resize(sz + addDendList.size());
            //printf("it is OK here\n");
            std::copy(addDendList.begin(), addDendList.end(),
                      DD_enlarge1[idexCell].begin() + sz);
        }
    }
}

void NeuroTreeCluster::GetConnectUnassignedNet(const MatXi &newCompressGraph, const int ID,
                                                std::vector<int> &newNetDendList)
{
    MatXi newCompressGraphCp = newCompressGraph;
    int nx = newCompressGraphCp.rows();
    MatXi resultGraph;
    resultGraph.resize(nx, nx);
    resultGraph.setZero();
    resultGraph(0,0) = ID + 1;
    std::vector<int> idexx;
    idexx.push_back(ID);
    newNetDendList.clear();
    newNetDendList.push_back(ID + 1);
    for(int k = 0; k < 150; ++k){
        std::vector<int> aax;
        std::vector<int> aax1;
        for(std::vector<int>::size_type i = 0; i < idexx.size();++i){
            std::vector<int> s1;
            for(int j = 0; j < newCompressGraphCp.rows(); ++j){
                if(newCompressGraphCp(idexx[i], j) == 1){
                    s1.push_back(j + 1);
                }
            }
            newCompressGraphCp.row(idexx[i]).setZero();
            newCompressGraphCp.col(idexx[i]).setZero();
            int sz = aax.size();
            aax.resize(sz + s1.size());
            std::copy(s1.begin(), s1.end(), aax.begin() + sz);
            for(std::vector<int>::size_type l = 0; l < s1.size(); ++l){
                aax1.push_back((idexx[i]+1) * 1000 + s1[l]);
            }
        }
        std::sort(aax.begin(), aax.end());
        aax.erase(std::unique(aax.begin(), aax.end()), aax.end());
        int sz = newNetDendList.size();
        newNetDendList.resize(sz + aax.size());
        std::copy(aax.begin(), aax.end(), newNetDendList.begin() + sz);
        std::sort(aax1.begin(), aax1.end());
        aax1.erase(std::unique(aax1.begin(), aax1.end()), aax1.end());
        idexx = aax;
        for(std::vector<int>::size_type ll = 0; ll < idexx.size();++ll){
            --idexx[ll];
        }
        BreakPathGraph(newCompressGraphCp, idexx);
        if(!aax.empty()){
            for(std::vector<int>::size_type m = 0; m < aax1.size(); ++m){
                resultGraph(m, k+1) = aax1[m];
            }
        } else{
            break;
        }
    }
}

void NeuroTreeCluster::BreakPathGraph(MatXi &graph, const std::vector<int> &idexx)
{
    int nxx = idexx.size();
    for(int i = 0; i < nxx - 1; ++i){
        for(int j = i + 1; j < nxx; ++j){
            graph(idexx[i], idexx[j]) = 0;
            graph(idexx[j], idexx[i]) = 0;
        }
    }
}

void NeuroTreeCluster::ReviseTree(const std::vector<VectorVec5d> &dendSomaList, const VectorMat2i &dendSomaInfo,
                                   const std::vector<int> &currentSomaTree, const int allCurveNum,
                                   std::vector<int> &resultSomaTree)
{
    std::vector<int> idexx;
    for(std::vector<int>::size_type i = 0; i < currentSomaTree.size();++i){
        if(currentSomaTree[i] > allCurveNum){
            idexx.push_back(i);
        }
    }
    if(!idexx.empty()){
        std::vector<VectorVec4d> treeLevel;
        std::vector<VectorVec5d> dendSomaListCp;
        VectorMat2i dendSomaInfoCp;
        for(std::vector<int>::size_type i = 0; i < currentSomaTree.size();++i){
            dendSomaInfoCp.push_back(dendSomaInfo[currentSomaTree[i]-1]);
            dendSomaListCp.push_back(dendSomaList[currentSomaTree[i]-1]);
        }
        CreateTreeStructure(dendSomaListCp, dendSomaInfoCp, idexx[0], currentSomaTree, 29,treeLevel);
        std::vector<VectorVec4d> newTreeLevel;
        VectorMat2i newDendSomaInfo;
        std::vector<int> newSomaTree;
        ReviseOneTree(treeLevel, currentSomaTree, dendSomaInfo,newTreeLevel,
                      newSomaTree, newDendSomaInfo);
        std::vector<int> idexx;
        for(std::vector<int>::size_type i = 0; i < newSomaTree.size(); ++i){
            if(newSomaTree[i] > allCurveNum){
                idexx.push_back(i);
            }
        }
        std::vector<VectorVec5d> tmpDendSomaList;
        VectorMat2i tmpDendSomaInfo;
        for(std::vector<int>::size_type i = 0; i < newSomaTree.size(); ++i){
            tmpDendSomaList.push_back(dendSomaList[newSomaTree[i]-1]);
            tmpDendSomaInfo.push_back(newDendSomaInfo[newSomaTree[i]-1]);
        }
        std::vector<VectorVec4d> treeLevelCp;
        CreateTreeStructure(tmpDendSomaList, tmpDendSomaInfo, idexx[0], newSomaTree,
                            29, treeLevelCp);
        treeLevel.swap(treeLevelCp);
        ModifyTree(treeLevel,dendSomaList,currentSomaTree, resultSomaTree);
    } else{
        resultSomaTree.clear();
    }
}

void NeuroTreeCluster::CreateTreeStructure(const std::vector<VectorVec5d> &globalSomaTree,
                                            const VectorMat2i &globalSomaTreeInfo,
                                            const int &idexx, const std::vector<int> &currentSomaTree,
                                            const int deep, std::vector<VectorVec4d> &resultTreeStructure)
{
    MatXi localPathGraph;
    BuildPathMatrix(globalSomaTreeInfo, currentSomaTree, localPathGraph);
    std::vector<VectorVec4d> treeLevelList;
    treeLevelList.resize(deep);
    std::vector<int> idexxs;
    for(int i = 0; i < localPathGraph.rows();++i){
        if(1 == localPathGraph(idexx, i)){
            idexxs.push_back(i);
        }
    }
    VectorVec4d treeLevelInfo;
    treeLevelInfo.resize(idexxs.size());
    std::vector<int> connectNodeIDList;
    JudgeTreeCurveDirection(globalSomaTree,globalSomaTreeInfo,idexx,idexxs,currentSomaTree,
                            connectNodeIDList);
    for(VectorVec4d::size_type i = 0; i < idexxs.size();++i){
        treeLevelInfo[i](0) = idexx;
        treeLevelInfo[i](1) = idexxs[i];
        treeLevelInfo[i](2) = 1.0;
        treeLevelInfo[i](3) = connectNodeIDList[i];
    }
    treeLevelList[0].swap(treeLevelInfo);
    for(int i = 0; i < localPathGraph.rows(); ++i){
        localPathGraph(idexx, i) = 0;
        localPathGraph(i, idexx) = 0;
    }

    int num = 0;
    for(int i = 1; i < deep; ++i){
        num = i;
        MatXi localPathGraphCp;
        CreateOneLevelStructure(treeLevelList[i-1],localPathGraph,globalSomaTree,
                globalSomaTreeInfo,currentSomaTree, treeLevelInfo, localPathGraphCp);
        localPathGraph = localPathGraphCp;
        if(!treeLevelInfo.empty()){
            treeLevelList[i].swap(treeLevelInfo);
        } else{
            break;
        }
    }
    for(int i = 0; i < num; ++i){
        resultTreeStructure.push_back(treeLevelList[i]);
    }
}

void NeuroTreeCluster::JudgeTreeCurveDirection(const std::vector<VectorVec5d> &globalSomaTree,
                                                const VectorMat2i &globalSomaTreeInfo,
                                                const int idexx, const std::vector<int> &idexxs,
                                                const std::vector<int> &currentSomaTree,
                                                std::vector<int> &connectNodeIDList)
{
    int nx = idexxs.size();
    connectNodeIDList.resize(nx);
    const VectorVec5d &currentGlobalSomaTree = globalSomaTree[idexx];
    int currentGlobalSomaTreeSize = int(currentGlobalSomaTree.size());
    const Mat2i &currentGlobalSomaTreeInfo = globalSomaTreeInfo[idexx];
    for(int i = 0; i < nx; ++i){
        const Mat2i &sonConInfo = globalSomaTreeInfo[idexxs[i]];
        const VectorVec5d &sonDendList = globalSomaTree[idexxs[i]];

        if( sonConInfo(0,0)==currentSomaTree[idexx] )
                connectNodeIDList[i]=1;
        if( sonConInfo(0,1)==currentSomaTree[idexx] )
                connectNodeIDList[i]=sonDendList.size();
        if(sonConInfo(0,0)!=currentSomaTree[idexx] && sonConInfo(0,1)!=currentSomaTree[idexx]){
            if( currentGlobalSomaTreeInfo(0,0) == currentSomaTree[idexxs[i]]){
                std::vector<double> distList;
                distList.resize(sonDendList.size());
                Vec3d a1(currentGlobalSomaTree[0](0), currentGlobalSomaTree[0](1),
                        currentGlobalSomaTree[0](2) );
                //TODO:Matlab unknown datam
                for(std::vector<double>::size_type ii = 0; ii < sonDendList.size(); ++ii){
                    distList[ii] = (a1 - Vec3d(sonDendList[ii](0), sonDendList[ii](1),
                                               sonDendList[ii](2))).norm();
                }
                std::vector<double>::iterator minIt = std::min_element(distList.begin(),
                                                                       distList.end());
                int minDistID = std::distance(distList.begin(), minIt);
                connectNodeIDList[i] = minDistID;
            }

            if( currentGlobalSomaTreeInfo(0,1) == currentSomaTree[idexxs[i]]){
                std::vector<double> distList;
                distList.resize(sonDendList.size());
                Vec3d a1(currentGlobalSomaTree[currentGlobalSomaTreeSize - 1](0),
                        currentGlobalSomaTree[currentGlobalSomaTreeSize - 1](1),
                        currentGlobalSomaTree[currentGlobalSomaTreeSize - 1](2) );
                //TODO:Matlab unknown datam
                for(std::vector<double>::size_type ii = 0; ii < sonDendList.size(); ++ii){
                    distList[ii] = (a1 - Vec3d(sonDendList[ii](0), sonDendList[ii](1),
                                               sonDendList[ii](2))).norm();
                }
                std::vector<double>::iterator minIt = std::min_element(distList.begin(),
                                                                       distList.end());
                int minDistID = std::distance(distList.begin(), minIt);
                connectNodeIDList[i] = minDistID;
            }
        }

    }
}

void NeuroTreeCluster::CreateOneLevelStructure(const VectorVec4d &currentTreeLevel,
                                                const MatXi &localPathGraph,
                                                const std::vector<VectorVec5d> &globalSomaTree,
                                                const VectorMat2i &globalSomaTreeInfo,
                                                const std::vector<int> &currentSomaTree,
                                                VectorVec4d &currentLevelInfo,
                                                MatXi &resultPathGraph)
{
    resultPathGraph = localPathGraph;
    std::vector<int> sonNodeList;
    sonNodeList.resize(currentTreeLevel.size());
    for(VectorVec4d::size_type i = 0; i < currentTreeLevel.size();++i){
        sonNodeList[i] = int(currentTreeLevel[i](1));
    }
    int nx = resultPathGraph.rows();
    //currentLevelInfo.resize(nx);
    currentLevelInfo.clear();
    //int count = 0;
    for(std::vector<int>::size_type i = 0; i < sonNodeList.size();++i){
        std::vector<int> idex;
        for(int j = 0; j < nx; ++j){
            if( 1 == resultPathGraph(sonNodeList[i], j))
                idex.push_back(j);
        }
        //int ns = idex.size();
        std::vector<double> connectAngle, connectNode;
        CalcTreeLevelAngle(Vec2i(currentTreeLevel[i](1), currentTreeLevel[i](3)),
                                 idex, globalSomaTree, globalSomaTreeInfo,
                                 currentSomaTree, connectAngle, connectNode);
        Vec4d tmp;
        for(std::vector<int>::size_type j = 0; j < idex.size();++j){
            tmp << sonNodeList[i], idex[j], connectAngle[j], connectNode[j];
            currentLevelInfo.push_back(tmp);
        }
        resultPathGraph.row(sonNodeList[i]).setZero();
        resultPathGraph.col(sonNodeList[i]).setZero();
    }
}

void NeuroTreeCluster::CalcTreeLevelAngle(const Vec2i &currentTreeNode, const std::vector<int> &idex,
                                           const std::vector<VectorVec5d> &globalSomaTree, const VectorMat2i &globalSomaTreeInfo,
                                           const std::vector<int> &currentSomaTree,
                                           std::vector<double> &dirList, std::vector<double> &angleList)
{
    const VectorVec5d &currentTreeList = globalSomaTree[currentTreeNode[0]];
    int nx = currentTreeList.size();
    int nxx = idex.size();
    dirList.resize(nxx);
    angleList.resize(nxx);
    for(int i = 0; i < nxx; ++i){
        int kk = idex[i];
        const Mat2i& currentConInfo = globalSomaTreeInfo[kk];
        const VectorVec5d& currentTree = globalSomaTree[kk];
        int nm = currentTree.size();
        if(nm > 1){
            if(currentConInfo(0,0) == currentSomaTree[currentTreeNode[0]]){
                angleList[i] = 1;
                std::vector<double> distList;
                distList.resize(nx);
                Vec3d firstTreeNode(currentTree[0](0), currentTree[0](1),
                        currentTree[0](2) );
                Vec3d aDist;
                for(int idd = 0; idd < nx; ++idd){
                    aDist = firstTreeNode - Vec3d(currentTreeList[idd](0),
                                                  currentTreeList[idd](1),
                                                   currentTreeList[idd](2));
                    distList[idd] = aDist.norm();
                }

                std::vector<double>::iterator minIt = std::min_element(distList.begin(),
                                                                       distList.end());
                int minDist = std::distance(distList.begin(), minIt);
                if(minDist < currentTreeNode(1)){
                    VectorVec3d tmpList1;
                    Vec3d tmp;
                    for(int j = std::min<int>(int(minDist) + 12, nx - 1);
                        j >= std::max<int>(int(minDist) - 12, 0); --j){
                        tmp << currentTreeList[j](0), currentTreeList[j](1),
                               currentTreeList[j](2);
                        tmpList1.push_back(tmp);
                    }
                    Vec3d mainDir1;
                    CalcPrinDirAndDistList(tmpList1, mainDir1);

                    VectorVec3d tmpList2;
                    for(int j = 1;
                        j <= std::min<int>(24, nm - 1); ++j){
                        tmp << currentTree[j](0), currentTree[j](1),
                               currentTree[j](2);
                        tmpList2.push_back(tmp);
                    }
                    Vec3d mainDir2;
                    CalcPrinDirAndDistList(tmpList2, mainDir2);
                    dirList[i] = mainDir1.dot(mainDir2);
                } else{
                    VectorVec3d tmpList1;
                    Vec3d tmp;
                    for(int j = std::max<int>(int(minDist) - 12, 0);
                        j <= std::min<int>(int(minDist) + 12, nx - 1); ++j){
                        tmp << currentTreeList[j](0), currentTreeList[j](1),
                               currentTreeList[j](2);
                        tmpList1.push_back(tmp);
                    }
                    Vec3d mainDir1;
                    CalcPrinDirAndDistList(tmpList1, mainDir1);

                    VectorVec3d tmpList2;
                    for(int j = 1;
                        j <= std::min<int>(24, nm - 1); ++j){
                        tmp << currentTree[j](0), currentTree[j](1),
                               currentTree[j](2);
                        tmpList2.push_back(tmp);
                    }
                    Vec3d mainDir2;
                    CalcPrinDirAndDistList(tmpList2, mainDir2);
                    dirList[i] = mainDir1.dot(mainDir2);
                }
            }

            if(currentConInfo(0,1) == currentSomaTree[currentTreeNode[0]]){
                angleList[i] = nm;
                std::vector<double> distList;
                distList.resize(nx);
                Vec3d firstTreeNode(currentTree[nm - 1](0), currentTree[nm - 1](1),
                        currentTree[nm - 1](2) );
                Vec3d aDist;
                for(int idd = 0; idd < nx; ++idd){
                    aDist = firstTreeNode - Vec3d(currentTreeList[idd](0),
                                                  currentTreeList[idd](1),
                                                   currentTreeList[idd](2));
                    distList[idd] = aDist.norm();
                }

                std::vector<double>::iterator minIt = std::min_element(distList.begin(),
                                                                       distList.end());
                int minDist = std::distance(distList.begin(), minIt);
                if(minDist < currentTreeNode(1)){
                    VectorVec3d tmpList1;
                    Vec3d tmp;
                    for(int j = std::min<int>(int(minDist) + 12, nx - 1);
                        j >= std::max<int>(int(minDist) - 12, 0); --j){
                        tmp << currentTreeList[j](0), currentTreeList[j](1),
                               currentTreeList[j](2);
                        tmpList1.push_back(tmp);
                    }
                    Vec3d mainDir1;
                    CalcPrinDirAndDistList(tmpList1, mainDir1);

                    VectorVec3d tmpList2;
                    for(int j = nm - 2;
                        j >= std::max<int>(0, nm - 26); --j){
                        tmp << currentTree[j](0), currentTree[j](1),
                               currentTree[j](2);
                        tmpList2.push_back(tmp);
                    }
                    Vec3d mainDir2;
                    CalcPrinDirAndDistList(tmpList2, mainDir2);
                    dirList[i] = mainDir1.dot(mainDir2);
                } else{
                    VectorVec3d tmpList1;
                    Vec3d tmp;
                    for(int j = std::max<int>(int(minDist) - 12, 0);
                        j <= std::min<int>(int(minDist) + 12, nx - 1); ++j){
                        tmp << currentTreeList[j](0), currentTreeList[j](1),
                               currentTreeList[j](2);
                        tmpList1.push_back(tmp);
                    }
                    Vec3d mainDir1;
                    CalcPrinDirAndDistList(tmpList1, mainDir1);

                    VectorVec3d tmpList2;
                    for(int j = nm - 2;
                        j >= std::max<int>(0, nm - 26); --j){
                        tmp << currentTree[j](0), currentTree[j](1),
                               currentTree[j](2);
                        tmpList2.push_back(tmp);
                    }
                    Vec3d mainDir2;
                    CalcPrinDirAndDistList(tmpList2, mainDir2);
                    dirList[i] = mainDir1.dot(mainDir2);
                }
            }

            if(currentConInfo(0,0) != currentSomaTree[currentTreeNode[0]]
                    && currentConInfo(0,1) != currentSomaTree[currentTreeNode[0]]){
                const MatXi& connectNodeConInfo = globalSomaTreeInfo[currentTreeNode[0]];
                if(connectNodeConInfo(0,0) == currentSomaTree[kk]){
                    //angleList[i] = nm;
                    std::vector<double> distList;
                    distList.resize(nm);
                    Vec3d firstTreeNode(currentTreeList[0](0), currentTreeList[0](1),
                            currentTreeList[0](2) );
                    Vec3d aDist;
                    for(int idd = 0; idd < nm; ++idd){
                        aDist = firstTreeNode - Vec3d(currentTree[idd](0),
                                                      currentTree[idd](1),
                                                       currentTree[idd](2));
                        distList[idd] = aDist.norm();
                    }

                    std::vector<double>::iterator minIt = std::min_element(distList.begin(),
                                                                           distList.end());
                    int minDist = std::distance(distList.begin(), minIt);
                    angleList[i] = minDist;
                    if(minDist < 5){
                        VectorVec3d tmpList1;
                        Vec3d tmp;
                        for(int j = std::min<int>(24, nx - 1);
                            j >= 1; --j){
                            tmp << currentTreeList[j](0), currentTreeList[j](1),
                                   currentTreeList[j](2);
                            tmpList1.push_back(tmp);
                        }
                        Vec3d mainDir1;
                        CalcPrinDirAndDistList(tmpList1, mainDir1);

                        VectorVec3d tmpList2;
                        for(int j = 1;
                            j <= std::min<int>(24, nm - 1); ++j){
                            tmp << currentTree[j](0), currentTree[j](1),
                                   currentTree[j](2);
                            tmpList2.push_back(tmp);
                        }
                        Vec3d mainDir2;
                        CalcPrinDirAndDistList(tmpList2, mainDir2);
                        dirList[i] = mainDir1.dot(mainDir2);
                    }
                    if(minDist>nm-7){
                        VectorVec3d tmpList1;
                        Vec3d tmp;
                        for(int j = std::min<int>(23, nx - 1);
                            j >= 1; --j){
                            tmp << currentTreeList[j](0), currentTreeList[j](1),
                                   currentTreeList[j](2);
                            tmpList1.push_back(tmp);
                        }
                        Vec3d mainDir1;
                        CalcPrinDirAndDistList(tmpList1, mainDir1);

                        VectorVec3d tmpList2;
                        for(int j = nm - 2;
                            j >= std::max<int>(0, nm - 26); --j){
                            tmp << currentTree[j](0), currentTree[j](1),
                                   currentTree[j](2);
                            tmpList2.push_back(tmp);
                        }
                        Vec3d mainDir2;
                        CalcPrinDirAndDistList(tmpList2, mainDir2);
                        dirList[i] = mainDir1.dot(mainDir2);
                    }
                    if(minDist >= 5 && minDist <= nm - 7){
                        dirList[i] = -0.5;
                    }
                }
                if(connectNodeConInfo(0,1) == currentSomaTree[kk]){
                    //angleList[i] = nm;
                    std::vector<double> distList;
                    distList.resize(nm);
                    Vec3d firstTreeNode(currentTreeList.back()(0), currentTreeList.back()(1),
                            currentTreeList.back()(2) );
                    Vec3d aDist;
                    for(int idd = 0; idd < nm; ++idd){
                        aDist = firstTreeNode - Vec3d(currentTree[idd](0),
                                                      currentTree[idd](1),
                                                       currentTree[idd](2));
                        distList[idd] = aDist.norm();
                    }

                    std::vector<double>::iterator minIt = std::min_element(distList.begin(),
                                                                           distList.end());
                    int minDist = std::distance(distList.begin(), minIt);
                    angleList[i] = minDist;
                    if(minDist < 5){
                        VectorVec3d tmpList1;
                        Vec3d tmp;
                        for(int j = std::max<int>(0, nx - 9);
                            j <= nx - 3; ++j){
                            tmp << currentTreeList[j](0), currentTreeList[j](1),
                                   currentTreeList[j](2);
                            tmpList1.push_back(tmp);
                        }
                        Vec3d mainDir1;
                        CalcPrinDirAndDistList(tmpList1, mainDir1);

                        VectorVec3d tmpList2;
                        for(int j = 1;
                            j <= std::min<int>(7, nm - 1); ++j){
                            tmp << currentTree[j](0), currentTree[j](1),
                                   currentTree[j](2);
                            tmpList2.push_back(tmp);
                        }
                        Vec3d mainDir2;
                        CalcPrinDirAndDistList(tmpList2, mainDir2);
                        dirList[i] = mainDir1.dot(mainDir2);
                    }
                    if(minDist>nm-7){
                        VectorVec3d tmpList1;
                        Vec3d tmp;
                        for(int j = std::max<int>(0, nx - 9);
                            j <= nx - 3; ++j){
                            tmp << currentTreeList[j](0), currentTreeList[j](1),
                                   currentTreeList[j](2);
                            tmpList1.push_back(tmp);
                        }
                        Vec3d mainDir1;
                        CalcPrinDirAndDistList(tmpList1, mainDir1);

                        VectorVec3d tmpList2;
                        for(int j = nm - 2;
                            j >= std::max<int>(0, nm - 7); --j){
                            tmp << currentTree[j](0), currentTree[j](1),
                                   currentTree[j](2);
                            tmpList2.push_back(tmp);
                        }
                        Vec3d mainDir2;
                        CalcPrinDirAndDistList(tmpList2, mainDir2);
                        dirList[i] = mainDir1.dot(mainDir2);
                    }
                    if(minDist >= 5 && minDist <= nm - 7){
                        dirList[i] = -0.5;
                    }
                }

            }
        } else{
            dirList[i]=-2;
            angleList[i]=0;
        }
    }
}

void NeuroTreeCluster::CalcPrinDirAndDistList(const VectorVec3d &ptLine, Vec3d &mainDirection)
{
    int nx = ptLine.size();
    Vec3d mm;
//    VectorVec3d ptDistList;
//    for(int i = 0; i < nx - 2; ++i){
//        //mm=ptLine(:,i+2)-ptLine(:,i);
//        mm = ptLine[i + 2] - ptLine[i];
//        ptDistList.push_back(mm.normalized());
//    }

    mainDirection.setZero();
    for(int i = 0; i < nx - 1; ++i){
        mm = ptLine[i + 1] - ptLine[i];
        //warning!
        mm(2) *= 2.0;
        mm /= (mm + Vec3d(0.0001, 0.0001, 0.0001)).norm();
        mainDirection += mm;
    }
    mainDirection /= (mainDirection+Vec3d(0.0001, 0.0001, 0.0001)).norm();
}

void NeuroTreeCluster::ReviseOneTree(const std::vector<VectorVec4d> &treeLevel, const std::vector<int> &currentSomaTree,
                                      const VectorMat2i &dendSomaInfo,
                                      std::vector<VectorVec4d> &newTreeLevel, std::vector<int> &resultSomaTree,
                                      VectorMat2i &newDendSomaInfo)
{
    int currentSomaTreeNum = currentSomaTree.size();
    std::vector<int> IDListWithHeadTail;
    newDendSomaInfo = dendSomaInfo;
    for(int i = 0; i < currentSomaTreeNum; ++i){
        const Mat2i& aa = dendSomaInfo[currentSomaTree[i] - 1];
        if( std::abs(aa(0,0)) > 0 && std::abs(aa(0,1))>0 ){
            IDListWithHeadTail.push_back(i);
        }
    }
    if(!IDListWithHeadTail.empty()){
        int treeLevelDepth = treeLevel.size();
        VectorVec6d allTreeLevelNode;
        for(int j = 0; j < treeLevelDepth; ++j){
            const VectorVec4d& currentTreeLevel = treeLevel[j];
            int currentTreeLevelNum = currentTreeLevel.size();
            Vec6d tmp;
            for(int k = 0; k < currentTreeLevelNum; ++k){
                tmp << currentTreeLevel[k](0), currentTreeLevel[k](1),
                       currentTreeLevel[k](2), currentTreeLevel[k](3),
                       j , k;
                allTreeLevelNode.push_back(tmp);
            }
        }

        for(std::vector<int>::size_type i = 0; i < IDListWithHeadTail.size();
            ++i){

            int aNodeWithHeadTail = IDListWithHeadTail[i];
            std::vector<int> idexxt;
            for(VectorVec6d::size_type j = 0; j < allTreeLevelNode.size();++j){
                if(allTreeLevelNode[j](1) == aNodeWithHeadTail){
                    idexxt.push_back(j);
                }
            }
            if(idexxt.size() == 2){
                if(allTreeLevelNode[idexxt[0]](2) > allTreeLevelNode[idexxt[1]](2) + 0.3){
                    allTreeLevelNode[idexxt[1]](5) = 0;
                    if(allTreeLevelNode[idexxt[1]](3) == 1){
                        newDendSomaInfo[currentSomaTree[
                                aNodeWithHeadTail] - 1](0,0)=0;
                    } else{
                        newDendSomaInfo[currentSomaTree[
                                aNodeWithHeadTail] - 1](0,1)=0;
                    }
                }
                if(allTreeLevelNode[idexxt[0]](2) + 0.3
                        < allTreeLevelNode[idexxt[1]](2)){
                    allTreeLevelNode[idexxt[0]](5) = 0;
                    if(allTreeLevelNode[idexxt[0]](3) == 1){
                        newDendSomaInfo[currentSomaTree[
                                aNodeWithHeadTail] - 1](0,0)=0;
                    } else{
                        newDendSomaInfo[currentSomaTree[
                                aNodeWithHeadTail] - 1](0,1)=0;
                    }
                }
            }
        }

        std::vector<int> validIDList;
        for(VectorVec6d::size_type k = 0; k < allTreeLevelNode.size();++k){
            if(allTreeLevelNode[k](5) > -1)//matlab is 0
                validIDList.push_back(k);
        }
        VectorVec6d allTreeLevelNodeCp;
        for(std::vector<int>::size_type k = 0; k < validIDList.size();++k){
            allTreeLevelNodeCp.push_back(allTreeLevelNode[validIDList[k]]);
        }
        allTreeLevelNode.swap(allTreeLevelNodeCp);
        std::vector<VectorVec4d> newTreeLevel;
        newTreeLevel.resize(treeLevelDepth);
        int kk = 0;
        for(int jj = 0; jj < treeLevelDepth; ++jj){
            validIDList.clear();
            for(VectorVec6d::size_type k = 0; k < allTreeLevelNode.size();++k){
                if(allTreeLevelNode[k](4) == jj)
                    validIDList.push_back(k);
            }
            if(!validIDList.empty()){
                VectorVec4d tmpNode;
                Vec4d tmp;
                for(std::vector<int>::size_type k = 0; k < validIDList.size();++k){
                    tmp << allTreeLevelNode[validIDList[k]](0),
                            allTreeLevelNode[validIDList[k]](1),
                            allTreeLevelNode[validIDList[k]](2),
                            allTreeLevelNode[validIDList[k]](3);
                    tmpNode.push_back(tmp);
                }
                newTreeLevel[jj].swap(tmpNode);
                ++kk;
            } else{
                break;
            }
        }

        if(kk > 0){
            std::vector<int> allNewTreeLevelList;
            for(int jj = 0; jj < kk; ++jj){
                const VectorVec4d& aNewTreeLevel = newTreeLevel[jj];
                for(VectorVec4d::size_type k = 0; k < aNewTreeLevel.size(); ++k){
                    allNewTreeLevelList.push_back(Round(aNewTreeLevel[k](0)));
                }
                for(VectorVec4d::size_type k = 0; k < aNewTreeLevel.size(); ++k){
                    allNewTreeLevelList.push_back(Round(aNewTreeLevel[k](1)));
                }
            }
            std::sort(allNewTreeLevelList.begin(), allNewTreeLevelList.end());
            allNewTreeLevelList.erase(std::unique(allNewTreeLevelList.begin(),
                                                  allNewTreeLevelList.end()),
                                      allNewTreeLevelList.end());

            resultSomaTree.clear();
            for(std::vector<int>::size_type k = 0; k < allNewTreeLevelList.size();
                ++k){
                resultSomaTree.push_back(currentSomaTree[allNewTreeLevelList[k]]);
            }
        }
    } else{
        newTreeLevel = treeLevel;
        resultSomaTree = currentSomaTree;
    }
}

void NeuroTreeCluster::ModifyTree(const std::vector<VectorVec4d> &origTreeLevel,
                                   const std::vector<VectorVec5d> &dendSomaList,
                                   const std::vector<int> &currentSomaTree,
                                   std::vector<int> &resultSomaTree)
{
    std::vector<VectorVec4d> treeLevel = origTreeLevel;
    int depth = treeLevel.size();
    std::vector<std::vector<int> > levelNodeFlag;
    levelNodeFlag.resize(depth);
    for(int i = 0; i < depth; ++i){
        VectorVec4d& currentTreeLevel = treeLevel[i];
        int currentTreeLevelNum = currentTreeLevel.size();
        for(int kk = 0; kk < currentTreeLevelNum; ++kk){
            if(currentTreeLevel[kk](2) <= -1.0)
                currentTreeLevel[kk](2) = - currentTreeLevel[kk](2);
        }
        std::vector<int> tmp(currentTreeLevel.size(), 1);
        levelNodeFlag[i].swap(tmp);
    }
    for(int i = 2; i < depth; ++i){
        const VectorVec4d& currentTreeLevel = treeLevel[i];
        std::vector<int> tmp;
        for(VectorVec4d::size_type k = 0; k < currentTreeLevel.size();++k){
            tmp.push_back(Round(currentTreeLevel[k](0)));
        }
        std::deque<int> diffLevel;
        std::adjacent_difference(tmp.begin(), tmp.end(),
                                 std::back_inserter(diffLevel));
        diffLevel.pop_front();
        std::vector<int> idexx;
        idexx.push_back(-1);
        for(std::deque<int>::size_type j = 0; j < diffLevel.size();++j){
            if( std::abs(diffLevel[j] - 0.0) > 0.001){
                idexx.push_back(j);
            }
        }
        idexx.push_back(currentTreeLevel.size() - 1);
        int ns = idexx.size();
        for(int jj = 0; jj < ns - 1; ++jj){
            std::vector<double> validAngleList;
            for(int k = idexx[jj] + 1; k <= idexx[jj+1]; ++k){
                validAngleList.push_back(currentTreeLevel[k](2));
            }
            int nlls = validAngleList.size();
            double curveLen=0.0;
            double angleWet = 0.0;
            std::vector<int> wrongConnectID;
            for(std::vector<double>::size_type k = 0; k < validAngleList.size();
                ++k){

                if( std::abs(validAngleList[k] + 0.5) < 0.001){
                    wrongConnectID.push_back(k);
                }
            }

            if(wrongConnectID.size() == 1){
                std::vector<int> &mm = levelNodeFlag[i];
                std::vector<int> tmpCurrentTreeLevel;
                for(std::vector<int>::size_type kk = 0; kk < wrongConnectID.size();
                    ++kk){
                    tmpCurrentTreeLevel.push_back(Round(currentTreeLevel[
                                                  idexx[jj] + wrongConnectID[kk] + 1](1)));
                    mm[idexx[jj] + wrongConnectID[kk] + 1] = 0;
                }

                std::vector<std::vector<int> > levelNodeFlagCp;
                GetTreeLevelFlagList(levelNodeFlag, i, tmpCurrentTreeLevel, treeLevel,
                                     levelNodeFlagCp);
                levelNodeFlag.swap(levelNodeFlagCp);
            }

            for(int ijj = 0; ijj < nlls; ++ijj){
                double m = double(dendSomaList[ currentSomaTree[ Round(currentTreeLevel[idexx[jj] + ijj + 1](1))] -1 ].size());
                curveLen += m;
                angleWet += m * validAngleList[ijj];
            }
            double angleLenRatio = angleWet / curveLen;

            if(angleLenRatio < -0.3){
                std::vector<int> &mm = levelNodeFlag[i];
                std::vector<int> tmpCurrentTreeLevel;
                for(int k = idexx[jj]+1; k <= idexx[jj+1];++k ){
                    mm[k] = 0;
                    tmpCurrentTreeLevel.push_back(Round(currentTreeLevel[k](1)));
                }
                std::vector<std::vector<int> > levelNodeFlagCp;
                GetTreeLevelFlagList(levelNodeFlag, i, tmpCurrentTreeLevel, treeLevel,
                                     levelNodeFlagCp);
                levelNodeFlag.swap(levelNodeFlagCp);
            }
        }
    }
    for(int i = 2; i < depth; ++i){
        const VectorVec4d &currentTreeLevel = treeLevel[i];
        int currentTreeLevelNum = currentTreeLevel.size();
        std::vector<double> medianRadiusList(currentTreeLevelNum,0);
        for(int jj = 0; jj < currentTreeLevelNum; ++jj){
            const VectorVec5d& globalOneLevelIDList = dendSomaList[currentSomaTree[Round(currentTreeLevel[jj](1)) ] -1];
            std::vector<double> radiusList;
            for(VectorVec5d::size_type k = 0; k < globalOneLevelIDList.size();++k){
                radiusList.push_back(globalOneLevelIDList[k](3));
            }
            std::sort(radiusList.begin(), radiusList.end());
            medianRadiusList[jj] = MedianAfterSort(radiusList);
        }
        double meanM = 0.0;
        for(std::vector<double>::size_type j = 0; j < medianRadiusList.size();
            ++j){

            meanM += medianRadiusList[j];
        }
        meanM /= double(medianRadiusList.size());
        double stdM = 0.0;
        for(std::vector<double>::size_type j = 0; j < medianRadiusList.size();
            ++j){
            double tmp = medianRadiusList[j] - meanM;
            stdM += tmp * tmp;
        }
        stdM = std::sqrt(stdM / (double(medianRadiusList.size() - 1)));
        double thre = meanM + 6.0 * stdM;
        thre = std::min<double>(2.5, thre);
        for(int ijj = 0; ijj < currentTreeLevelNum; ++ijj){
            if(medianRadiusList[ijj] > thre){
                std::vector<int>& mm = levelNodeFlag[i];
                mm[ijj]=0;
                std::vector<std::vector<int> > levelNodeFlagCp;
                GetTreeLevelFlagList(levelNodeFlag, i,
                                     std::vector<int>(1,Round(currentTreeLevel[ijj](1))),
                                     treeLevel,
                                     levelNodeFlagCp);
                levelNodeFlag.swap(levelNodeFlagCp);
            }
        }
    }

    resultSomaTree.clear();
    for(int i = 0; i < depth; ++i){
        std::vector<int> idexxm;
        std::vector<int> currentTreeLevel = levelNodeFlag[i];
        for(std::vector<int>::size_type j = 0; j < currentTreeLevel.size();++j){
            if(currentTreeLevel[j] == 1){
                idexxm.push_back(j);
            }
        }
        const VectorVec4d& dataK = treeLevel[i];
        for(std::vector<int>::size_type j = 0; j < idexxm.size();++j){
            resultSomaTree.push_back(Round(dataK[idexxm[j]](1)));
        }
    }
    std::vector<int> result;
    for(std::vector<int>::size_type j = 0; j < resultSomaTree.size();++j){
        result.push_back(currentSomaTree[resultSomaTree[j]]);
    }
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    resultSomaTree.swap(result);
}

void NeuroTreeCluster::GetTreeLevelFlagList(const std::vector<std::vector<int> > &levelNodeFlag,
                                             const int currentLevelID, const std::vector<int> &origSonNodeList,
                                             const std::vector<VectorVec4d> &treeLevel,
                                             std::vector<std::vector<int> > &newLevelNodeFlag)
{
    std::vector<int> sonNodeList = origSonNodeList;
    newLevelNodeFlag = levelNodeFlag;
    int nxx = treeLevel.size();
    for(int i = currentLevelID + 1; i < nxx; ++i){
        const VectorVec4d& currentLevelDepth = treeLevel[i];
        std::vector<int> currentLevelDepthID;
        for(VectorVec4d::size_type j = 0; j < currentLevelDepth.size();++j){
            currentLevelDepthID.push_back(Round(currentLevelDepth[j](0)));
        }
        std::vector<int> wrongIDList;
        GetWrongTreeLevelFlag(sonNodeList,currentLevelDepthID, wrongIDList);
        std::vector<int> &tmpFlagList = newLevelNodeFlag[i];
        for(std::vector<int>::size_type j = 0;j < wrongIDList.size();++j){
            tmpFlagList[wrongIDList[j]] = 0;
        }
        //sonNodeList=currentLevelDepth(2,wrongIDList);
        sonNodeList.clear();
        for(size_t j = 0; j < wrongIDList.size();++j)
            sonNodeList.push_back( currentLevelDepth[wrongIDList[j]](1));
    }
}

void NeuroTreeCluster::GetWrongTreeLevelFlag(const std::vector<int> &sonNodeList, const std::vector<int> &currentLevelDepthID, std::vector<int> &wrongIDList)
{
    int nx = sonNodeList.size();
    std::vector<int> IDWetList(currentLevelDepthID.size(), 1);
    for(int i = 0; i < nx; ++i){
        for(std::vector<int>::size_type j = 0; j < currentLevelDepthID.size();++j){
            if(currentLevelDepthID[j] == sonNodeList[i]){
                IDWetList[j] = 0;
            }
        }
    }
    wrongIDList.clear();
    for(std::vector<int>::size_type i = 0; i < IDWetList.size();++i){
        if(IDWetList[i] == 0)
            wrongIDList.push_back(i);
    }
}

void NeuroTreeCluster::RecoverCutTree( const std::vector<int> &wholeTree, const std::vector<int> &reviseTree, std::vector<int> &cutTree )
{
    cutTree.clear();
    for(size_t i = 0; i < wholeTree.size(); ++i){
        int temp = wholeTree[i];
        std::vector<int>::const_iterator iterator = std::find(reviseTree.begin(), reviseTree.end(), temp);
        if(iterator == reviseTree.end()){
            cutTree.push_back(temp);
        }
    }
}

void NeuroTreeCluster::ClusterCutTree( const std::vector<int> &cutTree, const VectorMat2i& conInfo, std::vector<std::vector<int> >& clustreInCutTree)
{
    std::vector<int> setID;
    setID.resize(cutTree.size());
    for(size_t i = 0; i < cutTree.size(); ++i){
        setID[i] = i;
    }
    for(size_t i = 0; i < cutTree.size(); ++i){
        for (size_t j = i + 1; j < cutTree.size(); ++j) {
            if(setID[i] == setID[j]) continue;
            if (conInfo[cutTree[i] - 1](0,0) == cutTree[j] || conInfo[cutTree[i] - 1](0,1) == cutTree[j]
            || conInfo[cutTree[j] - 1](0,0) == cutTree[i] || conInfo[cutTree[j] - 1](0,1) == cutTree[i]) {
                int temp;
                int dst;
                if(setID[j] > setID[i]){
                    temp = setID[j];
                    dst = setID[i];
                    setID[j] = setID[i];
                } else{
                    temp = setID[i];
                    dst = setID[j];
                    setID[i] = setID[j];
                }
                for (size_t k = 0; k < cutTree.size(); ++k) {
                    if(setID[k] == temp){
                        setID[k] = dst;
                    }
                }
            }
        }
    }

    //get all set id
    std::vector<int> allID;
    for(size_t i = 0; i < cutTree.size(); ++i){
        int temp = setID[i];
        std::vector<int>::const_iterator iterator = std::find(allID.begin(), allID.end(), temp);
        if(iterator == allID.end()){
            allID.push_back(temp);
        }
    }

    //fill set
    clustreInCutTree.resize(allID.size());
    for(size_t i = 0; i < cutTree.size(); ++i){
         int temp = setID[i];
        std::vector<int>::iterator iterator = std::find(allID.begin(), allID.end(), temp);
         std::vector<int>::size_type dist = std::distance(allID.begin(), iterator);
        clustreInCutTree[dist].push_back(cutTree[i]);
    }
}

void NeuroTreeCluster::CollectCutTreePiece( const std::vector<std::vector<int>>& clustreInCutTree, const std::vector<VectorVec5d>& newDendList,
    std::vector<std::vector<int>>& availableTreeInClustre)
{
    for (size_t i = 0; i < clustreInCutTree.size(); ++i) {
        const std::vector<int>& currentClustre = clustreInCutTree[i];
        if(currentClustre.size() < 3 ){
            int num=0;
            for(size_t kk = 0; kk < currentClustre.size();++kk){
                num += newDendList[currentClustre[kk]-1].size();
            }
            if(num < 80)
                continue;
        }
        availableTreeInClustre.push_back(currentClustre);
    }
}

void NeuroTreeCluster::ClusterTreeToMultiSomaInOneCluster(const VectorMat2i &conInfoInClustre,
                                                           const std::vector<int> &dendIDInClustre,
                                                           const int rawDendNum,
                                                           std::vector<VectorVec2i> &somaConnectSet,
                                                           std::vector<std::vector<int> > &allAssignedDendIDSet,
                                                           MatXi &compressPathGraph,
                                                           std::vector<int> &unAssignedList)
{
    std::vector<int> somaID;
    for(std::vector<int>::size_type i = 0; i < dendIDInClustre.size(); ++i){
        if(dendIDInClustre[i] > rawDendNum)
            somaID.push_back(i);
    }

    int somaNum = somaID.size();
    somaConnectSet.clear();

	MatXi pathGraph;
    BuildPathMatrix(conInfoInClustre, dendIDInClustre, pathGraph);

    VectorVec2i aas;
    Vec2i tmpAAS;
    for(int i = 0; i < somaNum; ++i){
        std::vector<int> somaConnectID;
        for(int j = 0 ; j < pathGraph.rows(); ++j){
            if(pathGraph(somaID[i], j) == 1){
                somaConnectID.push_back(j);
            }
        }

        aas.clear();
        //aas.resize(somaConnectID.size());
        for(std::vector<int>::size_type j = 0; j < somaConnectID.size();++j){
            tmpAAS << somaID[i], somaConnectID[j];
            aas.push_back(tmpAAS);
        }
        somaConnectSet.push_back(VectorVec2i());
        somaConnectSet[somaConnectSet.size() - 1].swap(aas);
    }

    allAssignedDendIDSet.clear();
	allAssignedDendIDSet.resize(somaNum);
    std::vector<int> isAssignedList;
    isAssignedList.resize(pathGraph.rows(),0);
	
    clock_t allBeg = clock();
	int process = 0;
    std::vector<std::list<int> > pathList;
    std::vector<std::list<int> > breakPathList;
    std::vector<std::list<int> > pathList11;
    std::vector<std::list<int> > tmpPathList;
    pathList.resize(pathGraph.rows());
    for (int l = 0; l < pathGraph.rows(); ++l) {
        for (int m = l + 1; m < pathGraph.cols(); ++m) {
            if(pathGraph(l,m) == 1){
                pathList[l].push_back(m);
                pathList[m].push_back(l);
            }
        }
    }
//#pragma omp parallel for
    for(int i = 0; i < somaNum; ++i){
        //printf("%d soma check\n",i);
        //clock_t onceBeg = clock();
        VectorVec2i &aSomaConnectSet = somaConnectSet[i];
       
        breakPathList = pathList;
        BreakPathListConnect(breakPathList, somaID[i]);
        for(VectorVec2i::size_type ii = 0; ii < aSomaConnectSet.size(); ++ii){
            BreakPathListConnect(breakPathList, aSomaConnectSet[ii](1));
        }

        pathList11 = pathList;
        /*for (int IKK=0; IKK < somaNum; ++IKK){
        if (IKK!=i){
        VectorVec2i &aSomaConnectSet1 = somaConnectSet[IKK];
        for(VectorVec2i::size_type ii = 0; ii < aSomaConnectSet1.size(); ++ii){
        BreakPathListConnect(pathList11, aSomaConnectSet1[ii](1));
        }
        }
        }*/

        std::vector<int> somaGrowSet1;
        SearchTreeFromSomaModify(pathList11, somaID[i], 6, somaGrowSet1);
        std::vector<int> assignedDendIDSet1;

        CheckUniqueCurveInTreeModify(breakPathList, somaID, somaGrowSet1, tmpPathList, assignedDendIDSet1);
        
#pragma omp critical
		{
			//printf("%d UniqueCurveInTree\n",int(assignedDendIDSet1.size()));
			++process;
            printf("%d of %d complete.\n",process, somaNum);
            for(std::vector<int>::size_type j = 0; j < assignedDendIDSet1.size(); ++j){
                isAssignedList[assignedDendIDSet1[j]] = 1;
            }

            allAssignedDendIDSet[i] = assignedDendIDSet1;
		}
        //clock_t onceEnd = clock();
        //printf("CheckUniqueCurveInTree once : %d\n", onceEnd - onceBeg);
    }//for somaNum
    //printf("nima2\n");
    //clock_t allEnd = clock();
    //printf("CheckUniqueCurveInTree all : %d\n", allEnd - allBeg);

    for(std::vector<int>::size_type i = 0; i < somaID.size(); ++i){
        isAssignedList[somaID[i]] = 1;
    }
    unAssignedList.clear();
    for(std::vector<int>::size_type i = 0; i < isAssignedList.size(); ++i){
        if(isAssignedList[i] == 0){
            unAssignedList.push_back(i);
        }
    }

    int nk = unAssignedList.size() + allAssignedDendIDSet.size();
    int ndd = allAssignedDendIDSet.size();
    compressPathGraph.resize(nk,nk);
    for(std::vector<int>::size_type i = 0; i < unAssignedList.size(); ++i){
        std::vector<int> idll;
        for(int j = 0; j < pathGraph.rows(); ++j){
            if(pathGraph(unAssignedList[i], j) == 1){
                idll.push_back(j);
            }
        }

        for(int j = 0; j < ndd; ++j){
            std::vector<int> cc;
            std::vector<int> scc;
            Intersection(idll, allAssignedDendIDSet[j], cc, scc);
            if(!cc.empty()){
                compressPathGraph(ndd + i, j) = 1;
                compressPathGraph(j, ndd + i) = 1;
            }
        }
    }
    //printf("nima3\n");

    for(std::vector<int>::size_type i = 0; i < unAssignedList.size(); ++i){
        std::vector<int> idll;
        for(int j = 0; j < pathGraph.rows(); ++j){
            if(pathGraph(unAssignedList[i], j) == 1){
                idll.push_back(j);
            }
        }

        std::vector<int> cc;
        std::vector<int> scc;
        Intersection(idll, unAssignedList, cc, scc);
        if(!cc.empty()){
            for(std::vector<int>::size_type k = 0; k < scc.size(); ++k){
                compressPathGraph(ndd + i, scc[k] + ndd) = 1;
                compressPathGraph(scc[k] + ndd, ndd + i) = 1;
            }
        }
    }

    for(int i = 0; i < nk; ++i){
        for(int j = i + 1; j < nk; ++j){
            if(compressPathGraph(i,j)==0){
                compressPathGraph(i,j) = std::numeric_limits<int>::max();
                compressPathGraph(j,i) = std::numeric_limits<int>::max();
            }
        }
    }
}

void NeuroTreeCluster::BreakPathListConnect(std::vector<std::list<int>> &pathList, int curveID)
{
    std::list<int> &connectID = pathList[curveID];
    for(auto it = connectID.begin(); it != connectID.end(); ++it){
        int test = *it;
        std::list<int> &conList = pathList[*it];
        conList.remove(curveID);
    }
    connectID.clear();
}

void NeuroTreeCluster::SearchTreeFromSomaModify(std::vector<std::list<int> > &rawPathGraph, const int somaID,
                              const int threv, std::vector<int>& somaGrowCurveSet)
{
    int i = 0;
    std::vector<std::list<int> >& pathGraph = rawPathGraph;
    std::vector<VectorVec2i> somaConnectSet;
    std::vector<int> somaConnectID;
    VectorVec2i aas;
    Vec2i tmpAAS;
    for(auto j = pathGraph[somaID].begin() ; j != pathGraph[somaID].end(); ++j){
        somaConnectID.push_back(*j);
    }

    aas.clear();
    for(std::vector<int>::size_type j = 0; j < somaConnectID.size();++j){
        tmpAAS << somaID, somaConnectID[j];
        aas.push_back(tmpAAS);
    }

    if(!somaConnectID.empty()){
        somaConnectSet.push_back(aas);
        BreakPathListConnect(pathGraph, somaID);
        i=1;
    } else{
        VectorVec2i tmp;
        tmp.push_back(Vec2i(somaID,0));
        somaConnectSet.push_back(tmp);
    }


    if(!aas.empty()){
        MatXi pathGraphCopy;
        for(i = 1; i < threv; ++i){//global varient i

            SearchAndBreakTreeModify(somaConnectSet[i - 1], pathGraph, aas);
            if(!aas.empty()){
                somaConnectSet.push_back(VectorVec2i());
                somaConnectSet[somaConnectSet.size() - 1].swap(aas);
            }
            else break;
        }
        --i;//warning!
    } else{
        i = 0;
    }


    somaGrowCurveSet.clear();
    const VectorVec2i &data1 = somaConnectSet[0];
    somaGrowCurveSet.push_back(data1[0](0));

    for(int ii = 0; ii <= i; ++ii){
        const VectorVec2i &data = somaConnectSet[ii];
        if(!data.empty()){
            for(VectorVec2i::size_type j = 0; j < data.size(); ++j){
                somaGrowCurveSet.push_back(data[j](1));
            }
        }
    }
    std::vector<int> tmpSet = somaGrowCurveSet;
    somaGrowCurveSet.swap(tmpSet);
}

void NeuroTreeCluster::SearchAndBreakTreeModify(const VectorVec2i &aSomaConnectSet,
                              std::vector<std::list<int>> &pathGraph,
                              VectorVec2i &resultSet)
{
    resultSet.clear();
    std::vector<std::list<int>> &pathGraphCopy = pathGraph;
    std::vector<int> somaID;
    for(std::vector<int>::size_type i = 0; i < aSomaConnectSet.size(); ++i){
        somaID.push_back(aSomaConnectSet[i](1));
    }

    std::vector<int> idex;
    for(std::vector<int>::size_type i = 0; i < somaID.size(); ++i){
        idex.clear();
        for(auto it =pathGraphCopy[somaID[i]].begin(); it != pathGraphCopy[somaID[i]].end();++it ){
            idex.push_back(*it);
        }

        Vec2i tmp;
        for(std::vector<int>::size_type k = 0; k < idex.size(); ++k){
            tmp << somaID[i], idex[k];
            resultSet.push_back(tmp);
        }
        BreakPathListConnect(pathGraphCopy, somaID[i]);
    }
}

void NeuroTreeCluster::CheckUniqueCurveInTreeModify(const std::vector<std::list<int>> &breakPathGraph,
    const std::vector<int> &somaID,
    const std::vector<int> &somaGrowSet,
    std::vector<std::list<int> >& pathList,
    std::vector<int> &assignedDendIDSet)
{
    int nxxs = somaGrowSet.size();
    //#pragma omp parallel for
    for(int i = 0; i < nxxs; ++i){
        bool hasSoma=false;
        bool isSoma =false;
        //std::vector<int> newSomaGrowSet;
        //printf("   SearchTreeFromSoma =%d of %d\n", i, nxxs);
        for(size_t k = 0; k < somaID.size(); ++k){
            if(somaID[k] == somaGrowSet[i]){
                isSoma = true;
                break;
            }
        }
        if(!isSoma){
            pathList = breakPathGraph;
            SearchLargeTreeFromSomaModify(pathList, somaGrowSet[i], somaID, 20, hasSoma);
        }else{
            hasSoma = true;
        }

        if(!hasSoma){
            //#pragma omp critical
            //{
            assignedDendIDSet.push_back(somaGrowSet[i]);
            //}
        }
    }
}

void NeuroTreeCluster::SearchLargeTreeFromSomaModify( std::vector<std::list<int> > &pathList, const int somaID,
                                   const std::vector<int>& allSomaID,
                                   const int threv, bool& hasSoma)
{
    hasSoma = false;
    int i = 0;
    //std::vector<int> somaGrowCurveSet;
    //std::vector<std::list<int> > pathList = rawPathList;
    std::vector<VectorVec2i> somaConnectSet;
    std::vector<int> somaConnectID;
    VectorVec2i aas;
    Vec2i tmpAAS;

    for(auto j = pathList[somaID].begin() ; j != pathList[somaID].end(); ++j){
        somaConnectID.push_back(*j);
    }
    aas.clear();
    //aas.resize(somaConnectID.size());
    for(std::vector<int>::size_type j = 0; j < somaConnectID.size();++j){
        tmpAAS << somaID, somaConnectID[j];
        aas.push_back(tmpAAS);
    }

    if(!somaConnectID.empty()){
        somaConnectSet.push_back(aas);
        BreakPathListConnect(pathList, somaID);
        i=1;
    } else{
        VectorVec2i tmp;
        tmp.push_back(Vec2i(somaID,0));
        somaConnectSet.push_back(tmp);
    }

    if(!aas.empty()){
        std::vector<int> cc1, scc1;
        std::vector<int> testID1;
        for(size_t k = 0; k < somaConnectSet.back().size();++k){
            testID1.push_back(somaConnectSet.back()[k](1));
        }
        Intersection(testID1, allSomaID, cc1, scc1);
        if(!cc1.empty()){
            hasSoma = true;
            return;
        }
        else{
            for(i = 1; i < threv; ++i){//global varient i
                SearchAndBreakTreeModify(somaConnectSet[i - 1], pathList, aas);
                //pathGraph = pathGraphCopy;
                if(!aas.empty()){
                    somaConnectSet.push_back(VectorVec2i());
                    somaConnectSet[somaConnectSet.size() - 1].swap(aas);
                    std::vector<int> cc, scc;
                    std::vector<int> testID;
                    for(size_t k = 0; k < somaConnectSet.back().size();++k){
                        testID.push_back(somaConnectSet.back()[k](1));
                    }
                    Intersection(testID, allSomaID, cc, scc);
                    if(!cc.empty()){
                        hasSoma = true;
                        break;
                    }
                }
                else break;
            }
        }
        --i;//warning!
    } else{
        i = 0;
    }
}
