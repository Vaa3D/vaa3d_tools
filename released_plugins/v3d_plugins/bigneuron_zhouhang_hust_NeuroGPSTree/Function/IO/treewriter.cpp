/*
 * Copyright (c)2013-2015  Zhou Hang, Shaoqun Zeng, Tingwei Quan
 * Britton Chance Center for Biomedical Photonics, Huazhong University of Science and Technology
 * All rights reserved.
 */
#include "treewriter.h"
#include "../../ngtypes/tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <QString>

#ifdef _WIN32
#include <iterator>
#endif

TreeWriter::TreeWriter()
{
    identifyName = std::string("TreeWriter");
    xExtend_ = yExtend_ = zExtend_ = 1.0;
}

TreeWriter::~TreeWriter()
{

}

void TreeWriter::SetOutputFileName(const std::string &str)
{
    fileName = str;
}

bool TreeWriter::Update()
{
    if(!m_Input || m_Input->GetIdentifyName() != std::string("SeperateTree")){
        printf("error occured in %s input is null\n", identifyName.c_str());
        return false;
    }
    if(m_Input->GetProcessObject()){
        if(!m_Input->GetProcessObject()->Update()){
            printf("error occured in %s\n", identifyName.c_str());
            return false;
        }
    }
    //output dd1 into many text and dd2 in one text
#ifdef _WIN32
    std::tr1::shared_ptr<const SeperateTree> tmptree = std::tr1::dynamic_pointer_cast<const SeperateTree>(m_Input);
#else
    std::shared_ptr<const SeperateTree> tmptree = std::dynamic_pointer_cast<const SeperateTree>(m_Input);
#endif
    if(!tmptree ) return false;

    //
    const std::vector<std::vector<VectorVec5d> > &swcList = tmptree->GetTree();
    const std::vector<int>& typeList = tmptree->GetTypeList();
    double distThrev = 1.0;
    char line[256];
    //std::string truePath;
    //std::string::size_type pos_begin = 0;
    //std::string::size_type comma_pos = fileName.find_first_of('.', pos_begin);
    //std::string prefix = fileName.substr(pos_begin, comma_pos - pos_begin);
    //for (size_t i = 0; i < swcList.size(); ++i) {
    for (size_t i = 0; i < 1; ++i) {
        //sprintf(line, "_%d",int(i));
        //if (typeList[i] == 2) {
            const std::vector<VectorVec5d>& dendList = swcList[i];
            std::vector<std::vector<std::vector<size_t> > > dendConInfo;
            GetDendConInfoFromDendList(dendList, distThrev, dendConInfo);
            //
            std::vector<size_t> parentNodeList;
            std::vector<std::vector<size_t> > sonNodeList;
            std::vector<std::vector<size_t> > treeLevel;
            GetTreeLevelFromConInfo(dendConInfo, 0, parentNodeList,  sonNodeList, treeLevel );
            //
            std::vector<swcElementStruct> resultSwc;
            GetWholeConnectSWC( dendList, parentNodeList, sonNodeList, treeLevel, dendConInfo,
                distThrev, resultSwc );

            //truePath = prefix + std::string(line) + ".swc";
            //truePath = prefix + ".swc";
            //WriteOneSwc(truePath, resultSwc, typeList[i]);
            if(!WriteOneSwc(fileName, resultSwc, typeList[i])){
                printf("cannot write file.\n");
                return false;
            }
        //} else{
            //truePath = prefix + std::string(line) + "_notree.swc";
            //FILE* fp = fopen((const char*)(truePath.c_str()), "w");
            //if(!fp) {
            //    printf("error write tree!\n");
            //    break;
            //}
            //One file One tree
            //const std::vector<VectorVec5d> &localTree = swcList[i];
            //int globalIndex = 1;
            //for(size_t j = 0; j < localTree.size();++j){
                //One curve
            //    const VectorVec5d &localCurve = localTree[j];
            //    fprintf(fp,"%d %d %lf %lf %lf %lf -1\n", globalIndex, typeList[i], localCurve[0](0) * xExtend_ ,
            //        localCurve[0](1) * yExtend_,localCurve[0](2) * zExtend_, localCurve[0](3));
            //    ++globalIndex;
            //    for(size_t k = 1; k < localCurve.size();++k){
            //        fprintf(fp,"%d %d %lf %lf %lf %lf %d\n", globalIndex, typeList[i], localCurve[k](0) * xExtend_ ,
            //            localCurve[k](1) * yExtend_, localCurve[k](2) * zExtend_, localCurve[k](3), globalIndex - 1);
            //        ++globalIndex;
            //    }
            //}
            //fclose(fp);
        //}
    }
    return true;
}

void TreeWriter::GetDendConInfoFromDendList( const std::vector<VectorVec5d>& dendList, double distThrev, std::vector<std::vector<std::vector<size_t> > >& dendConInfo )
{
    dendConInfo.resize(dendList.size());
    for (size_t i = 0; i < dendList.size(); ++i) {
        dendConInfo[i].resize(2);
    }
    //
    double pMinDistHead, pMinDistTail;
    size_t tmpId;//no use
    for (size_t i = 0; i < dendList.size(); ++i) {
        const Vec5d &pHead = dendList[i][0];
        const Vec5d &pTail = dendList[i].back();
        for (size_t j = 0; j < dendList.size(); ++j) {
            if(i==j) continue;
            pMinDistHead = CalcMinDistFromPtToCurve(pHead, dendList[j], tmpId);
            pMinDistTail = CalcMinDistFromPtToCurve(pTail, dendList[j], tmpId);
            if (pMinDistHead < distThrev) dendConInfo[i][0].push_back(j);
            if (pMinDistTail < distThrev) dendConInfo[i][1].push_back(j);
        }
    }

}

double TreeWriter::CalcMinDistFromPtToCurve( const Vec3d& pt, const VectorVec5d& curve, size_t &minIndex )
{
    double minDist = 100000.0;
    double tmpDist = 0.0;
    minIndex = 0;
    for (size_t i = 0; i < curve.size(); ++i) {
        tmpDist = (pt - curve[i].block(0,0,3,1)).norm();
        if (tmpDist < minDist) {
            minDist = tmpDist;
            minIndex = i;
        }
    }
    return minDist;
}

double TreeWriter::CalcMinDistFromPtToCurve( const Vec5d& pt, const VectorVec5d& curve, size_t &minIndex )
{
    double minDist = 100000.0;
    double tmpDist = 0.0;
    minIndex = 0;
    for (size_t i = 0; i < curve.size(); ++i) {
        tmpDist = (pt.block(0,0,3,1) - curve[i].block(0,0,3,1)).norm();
        if (tmpDist < minDist) {
            minDist = tmpDist;
            minIndex = i;
        }
    }
    return minDist;
}

void TreeWriter::GetTreeLevelFromConInfo( const std::vector<std::vector<std::vector<size_t> > >& dendConInfo, int somaIndex,
                                         std::vector<size_t> &parentNodeList, std::vector<std::vector<size_t> >& sonNodeList,
                                         std::vector<std::vector<size_t> > &treeLevel )
{
    size_t len = dendConInfo.size();
    //create connect matrix
    MatXi conMat(len, len);
    conMat.setZero();
    for (size_t i = 0; i < len; ++i) {
        for (size_t j = 0; j < dendConInfo[i][0].size(); ++j) {
            conMat(i, dendConInfo[i][0][j]) = 1;
            conMat(dendConInfo[i][0][j], i) = 1;
        }
        for (size_t j = 0; j < dendConInfo[i][1].size(); ++j) {
            conMat(i, dendConInfo[i][1][j]) = 1;
            conMat(dendConInfo[i][1][j], i) = 1;
        }
    }
    //initialize treeLevel. treeLevel{1} = [somaIndex];
    treeLevel.push_back(std::vector<size_t>());
    treeLevel[0].push_back(somaIndex);
    parentNodeList.resize(len);
    parentNodeList[somaIndex] = std::numeric_limits<size_t>::max();
    sonNodeList.resize(len);
    std::vector<size_t> list;
    for (size_t i = 0; i < len; ++i) {
        if (conMat(somaIndex, i)!=0) list.push_back(i);
    }//list is sorted
    sonNodeList[somaIndex] = list;
    //parentNodeList(list) = num2cell(somaIndex);
    for (size_t i = 0; i < list.size(); ++i) {
        parentNodeList[list[i]] = somaIndex;
    }
    treeLevel.push_back(std::vector<size_t>());
    treeLevel[1] = list;
    for (size_t i = 0; i < len; ++i) {
        conMat(somaIndex, i) = 0;
        conMat(i, somaIndex) = 0;
    }
    //
    //bool breakFlag = false;
    std::vector<size_t> tmpList;
    for (size_t i = 1; i < len; ++i) {
        //if(breakFlag) break;
        list.clear();//warning!!
        for (size_t j = 0; j < treeLevel[i].size(); ++j) {
            size_t currentPt = treeLevel[i][j];
            tmpList.clear();
            for (size_t k = 0; k < len; ++k) {
                if (conMat(currentPt, k) != 0) tmpList.push_back(k);//tmpList is sorted
            }
            //remove loop
            {
                std::sort(tmpList.begin(), tmpList.end());
                std::vector<size_t> tmp;
                std::set_difference(tmpList.begin(), tmpList.end(), treeLevel[i].begin(), treeLevel[i].end(), std::back_inserter(tmp));
                tmpList.swap(tmp);
            }
            for (size_t i = 0; i < len; ++i) {
                conMat(currentPt, i) = 0;
                conMat(i, currentPt) = 0;
            }
            sonNodeList[currentPt] = tmpList;
            for (size_t i = 0; i < tmpList.size(); ++i) {
                parentNodeList[tmpList[i]] = currentPt;
            }
            std::copy(tmpList.begin(), tmpList.end(), std::back_inserter(list));
        }
        if (!list.empty()) {
            //remove loop
            std::sort(list.begin(), list.end());
            list.erase(std::unique(list.begin(), list.end()), list.end());
            //
            treeLevel.push_back(std::vector<size_t>());
            treeLevel[i+1] = list;
        } else break;
    }
}

void TreeWriter::GetWholeConnectSWC( const std::vector<VectorVec5d>& dendList, const std::vector<size_t> &parentNodeList,
                                    const std::vector<std::vector<size_t> >& sonNodeList, const std::vector<std::vector<size_t> > &treeLevel,
                                    std::vector<std::vector<std::vector<size_t> > >& dendConInfo,
                                    double distThrev, std::vector<swcElementStruct>& resultSwc )
{
    //initial

    if (treeLevel.empty()) {
        printf("treeLevel is empty, programme has returned.\n");
        return;
    }
    if (treeLevel[0].size() > 1) {
        printf("wrong treeLevel. There are more than one root nodes\n");
        return;
    }
    size_t sum = 0;
    for (size_t i = 0; i < dendList.size(); ++i) {
        sum += dendList[i].size();
    }

    resultSwc.resize(sum);
    resultSwc[0].pid = -1;
    resultSwc[0].id = 1;
    resultSwc[0].globalId = -1;
    resultSwc[0].type = 0;
    resultSwc[0].node.setZero();
    for (size_t j = 1; j < resultSwc.size(); ++j) {
        resultSwc[j].id = j+1;
        resultSwc[j].globalId = -1;//must set -1
        resultSwc[j].type = 0;
        resultSwc[j].pid = j;
        resultSwc[j].node.setZero();
    }
    //
    int cishu = 0;
    int curIndex = 0;//matlab is 1
    for (size_t i = 0; i < treeLevel.size(); ++i) {
        const std::vector<size_t>& curNodeList = treeLevel[i];
        if (i == 0) {//root node, treeLevel[0] has only one node
            ++cishu;
            size_t sz = dendList[curNodeList[0]].size();
            for (size_t j = 0; j < sz; ++j) {
                resultSwc[curIndex + j].node = dendList[curNodeList[0]][j].block(0,0,4,1);
                resultSwc[curIndex + j].globalId = curNodeList[0];//here set 0
            }
            curIndex += sz;

        } else{// children node. traverse current order nodes, put them into newSwc
            for (size_t j = 0; j < curNodeList.size(); ++j) {
                ++cishu;
                size_t curNode = curNodeList[j];
                size_t ancestor = parentNodeList[curNode];
                if (isMember(ancestor, dendConInfo[curNode][0]) && isMember(ancestor, dendConInfo[curNode][1])) {//if loop, cut second end.
                    //printf("there is a curve that both ends of one curve connect to another one curve. now break it.\n");
                    dendConInfo[curNode][1].erase(std::remove(dendConInfo[curNode][1].begin(),dendConInfo[curNode][1].end(), ancestor),
                        dendConInfo[curNode][1].end());
                }
                size_t sz = dendList[curNode].size();
                //judge the connectivity
                //if current node connect to parent node
                if (isMember(ancestor, dendConInfo[curNode][0])){//head connects to parent node
                    //head connects to parent node
                    for (size_t j = 0; j < sz; ++j) {
                        resultSwc[curIndex + j].node = dendList[curNode][j].block(0,0,4,1);
                        resultSwc[curIndex + j].globalId = curNode;
                    }
                    Vec2i currentRange;
                    FindSwc8thRange(ancestor, resultSwc, currentRange);
                    size_t minIndex;
                    VectorVec5d tmpDend(currentRange(1) - currentRange(0) + 1);
                    Vec5d tmp;
                    for (size_t kk = currentRange(0); kk <= currentRange(1); ++kk) {
                        tmp << resultSwc[kk].node(0), resultSwc[kk].node(1), resultSwc[kk].node(2), resultSwc[kk].node(3),0;
                        tmpDend[kk - currentRange(0)] = tmp;
                    }

                    double minDist = CalcMinDistFromPtToCurve(dendList[curNode][0], tmpDend, minIndex);
                    if (minDist > distThrev) {
                        //printf("the connected nodes are too far, but we still connect them.\n");
                    }
                    //
                    resultSwc[curIndex].pid = currentRange(0) + minIndex + 1;
                } else if (isMember(ancestor, dendConInfo[curNode][1])) {//children's tail connects to parent node
                    for (size_t j = 0; j < sz; ++j) {//reverse
                        resultSwc[curIndex + sz - j -1].node = dendList[curNode][j].block(0,0,4,1);
                        resultSwc[curIndex + j].globalId = curNode;
                    }
                    Vec2i currentRange;
                    FindSwc8thRange(ancestor, resultSwc, currentRange);
                    size_t minIndex;
                    VectorVec5d tmpDend(currentRange(1) - currentRange(0) + 1);
                    Vec5d tmp;
                    for (size_t kk = currentRange(0); kk <= currentRange(1); ++kk) {
                        tmp << resultSwc[kk].node(0), resultSwc[kk].node(1), resultSwc[kk].node(2), resultSwc[kk].node(3),0;
                        tmpDend[kk - currentRange(0)] = tmp;
                    }
                    double minDist = CalcMinDistFromPtToCurve(dendList[curNode].back(), tmpDend, minIndex);
                    if (minDist > distThrev) {
                        //printf("the connected nodes are too far, but we still connect them.\n");
                    }
                    //
                    resultSwc[curIndex].pid = currentRange(0) + minIndex + 1;
                } else if (isMember(curNode, dendConInfo[ancestor][0]) || isMember(curNode, dendConInfo[ancestor][1])) {
                    //I must find the connective point in children curve. the connective point in current node must be the end point
                    // of the newSwc, because the head of the last curve in newSwc connect to its parent. If not ,throw an exception
                    Vec2i currentRange;
                    FindSwc8thRange(ancestor, resultSwc, currentRange);
                    std::vector<size_t> headNodeIndex;
                    for (size_t k = currentRange(0); k <= currentRange(1); ++k) {
                        if (resultSwc[k].pid != resultSwc[k].id -1) {
                            headNodeIndex.push_back(k);
                        }
                    }
                    std::vector<size_t> searchIndex;
                    if (headNodeIndex.size() >= 2) {
                       searchIndex.push_back(currentRange(0));
                       for (size_t kk = 0; kk < headNodeIndex.size(); ++kk) {
                           searchIndex.push_back(headNodeIndex[kk] - 1);
                           searchIndex.push_back(headNodeIndex[kk]);
                       }
                       searchIndex.push_back(currentRange(1));
                    } else{
                        searchIndex.push_back(currentRange(0));
                        searchIndex.push_back(currentRange(1));
                    }
                    double minDist = 100000;
                    size_t minIndex = 0;
                    size_t connectIndex;
                    for (size_t ij = 0; ij < searchIndex.size(); ++ij) {
                        size_t tmpIndex = 0;
                        const Vec3d tmpNode = resultSwc[searchIndex[ij]].node.block(0,0,3,1);

                        double tmpDist = CalcMinDistFromPtToCurve(tmpNode, dendList[curNode], tmpIndex);
                        if (tmpDist < minDist) {
                            minDist = tmpDist;
                            minIndex = tmpIndex;
                            connectIndex = searchIndex[ij];
                        }
                    }
                    if (minDist > distThrev) {
                        //printf("the connected nodes are too far, but we still connect them.\n");
                    }
                    for (size_t k = 0; k < minIndex; ++k) {
                        resultSwc[curIndex + k].node = dendList[curNode][minIndex - k - 1].block(0,0,4,1);
                        resultSwc[curIndex + k].globalId = curNode;
                    }
                    for (size_t k = minIndex; k < sz; ++k) {
                        resultSwc[curIndex + k].node = dendList[curNode][k].block(0,0,4,1);
                        resultSwc[curIndex + k].globalId = curNode;
                    }
                    resultSwc[curIndex].pid = connectIndex + 1;
                    resultSwc[curIndex + minIndex].pid = connectIndex + 1;
                } else{
                    printf("check the dendConInfo, I cannot find the connectivity!\n");
                    return;
                }
                curIndex += sz;
            }
        }
    }
}

bool TreeWriter::WriteOneSwc( const std::string& curFileName, const std::vector<swcElementStruct>& resultSwc, int type )
{
    FILE* fp = fopen(curFileName.c_str(), "w");
    if(!fp) return false;
    for (size_t i = 0; i < resultSwc.size(); ++i) {
        fprintf(fp, "%d %d %lf %lf %lf %lf %d\n", resultSwc[i].id, type,
            resultSwc[i].node(0) * xExtend_, resultSwc[i].node(1) * yExtend_, resultSwc[i].node(2) * zExtend_, resultSwc[i].node(3),
            resultSwc[i].pid);
    }
    fclose(fp);
    return true;
}

bool TreeWriter::isMember( size_t ind, const std::vector<size_t>& searchRange )
{
    for (size_t i = 0; i < searchRange.size(); ++i) {
        if (ind == searchRange[i]) return true;
    }
    return false;
}

void TreeWriter::FindSwc8thRange( size_t ind, const std::vector<swcElementStruct>& resultSwc, Vec2i& range )
{
    range << -1, -1;
    std::vector<int> allIndex;
    for (size_t i = 0; i < resultSwc.size(); ++i) {
        if (ind == resultSwc[i].globalId) {
            allIndex.push_back(i);
        }
    }
    range(0) = allIndex.front();
    range(1) = allIndex.back();
}


