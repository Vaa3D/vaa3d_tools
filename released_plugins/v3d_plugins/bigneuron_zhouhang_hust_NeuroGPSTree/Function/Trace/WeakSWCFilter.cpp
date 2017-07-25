#include "WeakSWCFilter.h"
#include "../../ngtypes/tree.h"
#include "../../ngtypes/soma.h"
#include "../../ngtypes/volume.h"
#include "../volumealgo.h"
#include <cstdlib>
#include <stdio.h>
#include <math.h>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <utility>
#include <iostream>

#ifdef _WIN32
#include <iterator>
#include <ctime>
#else
#include <sys/time.h>
#endif
const double EXPO = 0.000001;

WeakSWCFilter::WeakSWCFilter(void)
{
	identifyName = std::string("WeakSWCFilter");
#ifdef _WIN32
	m_Source = std::tr1::shared_ptr<SeperateTree>(new SeperateTree(this));
#else
	m_Source = std::shared_ptr<SeperateTree>(new SeperateTree(this));
#endif
}

WeakSWCFilter::~WeakSWCFilter(void)
{
}

bool WeakSWCFilter::Update()
{
	if(!m_Input || !m_Back || !m_Bin ){
		printf("error occured in %s\n", identifyName.c_str());
		return false;
	}
	if(m_Input->GetProcessObject()){//|| !m_Soma
		if(!m_Input->GetProcessObject()->Update()){
			printf("error occured in %s\n", identifyName.c_str());
			return false;
		}
	}

#ifdef _WIN32
	std::tr1::shared_ptr<const SeperateTree> tmpSeperateTree =
		std::tr1::dynamic_pointer_cast<const SeperateTree>(m_SWC);
	origImgPointer =
		std::tr1::dynamic_pointer_cast<const Volume<unsigned short> >(m_Input);
	backImgPointer =
		std::tr1::dynamic_pointer_cast<const Volume<unsigned short> >(m_Back);
	binImgPointer =
		std::tr1::dynamic_pointer_cast<const Volume<NGCHAR> >(m_Bin);
	std::tr1::shared_ptr<SeperateTree > tmpTree =
		std::tr1::dynamic_pointer_cast<SeperateTree>(m_Source);
#else
	std::shared_ptr<const SeperateTree> tmpSeperateTree =
		std::dynamic_pointer_cast<const SeperateTree>(m_SWC);
	origImgPointer =
		std::dynamic_pointer_cast<const Volume<unsigned short> >(m_Input);
	backImgPointer =
		std::dynamic_pointer_cast<const Volume<unsigned short> >(m_Back);
	binImgPointer =
		std::dynamic_pointer_cast<const Volume<NGCHAR> >(m_Bin);
	std::shared_ptr<SeperateTree > tmpTree =
		std::dynamic_pointer_cast<SeperateTree>(m_Source);
#endif

	if(!origImgPointer || !backImgPointer || !binImgPointer || !tmpSeperateTree) {
		printf("error backnoise image!\n");
		printf("error occured in %s\n", identifyName.c_str());
		return false;
	}

	//choose the largest swc file
	const std::vector<std::vector<VectorVec5d> >& tmpSwcList = tmpSeperateTree->GetTree();
	std::vector<std::pair<size_t, int> > swcSize(tmpSwcList.size());
	//% [~,Indexx]=sort(Datasize,'descend');
	//% swcCell=swcCell(Indexx);
	size_t curSize(0);
	for (size_t i = 0; i < tmpSwcList.size(); ++i) {
		curSize = 0;
		for(size_t j = 0; j < tmpSwcList[i].size(); ++j){
			curSize += tmpSwcList[i][j].size();
		}
		swcSize[i] = std::make_pair(curSize, i);
	}
	std::sort(swcSize.begin(), swcSize.end(), Pair_1st_great());
	std::vector<std::vector<VectorVec5d> > sortedSwcList(tmpSwcList.size());
	for (size_t i = 0; i < swcSize.size(); ++i) {
		sortedSwcList[i] = tmpSwcList[swcSize[i].second];
	}
	//check
	const std::vector<int>& typeList = tmpSeperateTree->GetTypeList();
	if (typeList[swcSize[0].second] ==0 ) {
		printf("the largest swc is not the soma swc.\n");
		//return false;
	}

	//
	std::vector<char> connectionDetLabel(tmpSwcList.size());
	connectionDetLabel[0] = 1;
	std::vector<std::vector<size_t> > connectionStruct(tmpSwcList.size());
	connectionStruct[0].push_back(0);//matlab is 1
	VectorVec3i filledPtSet;
	indexImg.SetSize(origImgPointer->x(), origImgPointer->y(), origImgPointer->z());
	labelBlock.SetSize(origImgPointer->x(), origImgPointer->y(), origImgPointer->z());
    std::vector<VectorVec5d> addSwc;
	for (size_t i = 1; i < sortedSwcList.size(); ++i) {
		CubesSetLabel(sortedSwcList, connectionStruct[i-1], filledPtSet);
        //test
       /* FILE* fp = fopen("E:/filledPtSet.swc","w");
        int tmpnum = 0;
        for (size_t k = 0; k < filledPtSet.size(); ++k) {
            fprintf(fp, "%d 1 %lf %lf %lf %lf -1\n", ++tmpnum, double(filledPtSet[k](0)), double(filledPtSet[k](1)), double(filledPtSet[k](2)), 1.0);
        }
        fclose(fp);*/
		RegiongrowFusedTraces(filledPtSet, 10.0);
		std::vector<size_t> candidateSWCId;
		for (size_t j = 0; j < connectionDetLabel.size(); ++j) {
			if (connectionDetLabel[j] == 0) {
				candidateSWCId.push_back(j);
			}
		}
		if (!candidateSWCId.empty()) {
			std::vector<size_t> connectionLabel;
			TreeStructureFusedDetection(sortedSwcList, connectionStruct, candidateSWCId, connectionLabel, addSwc);
			if (!connectionLabel.empty()) {
                for (size_t k = 0; k < connectionLabel.size(); ++k) {
                    connectionDetLabel[connectionLabel[k] ] = 1;
                }
                connectionStruct[i].swap(connectionLabel);
			} else break;
		} else break;
	}

	//merge the soma swc
	std::vector<size_t> mergeSwcList;
	for (size_t i = 0; i < connectionStruct.size(); ++i) {
		std::copy(connectionStruct[i].begin(), connectionStruct[i].end(), std::back_inserter(mergeSwcList));
	}
    std::vector<size_t> mergeIndex(mergeSwcList.size());
    for (size_t i = 0; i < mergeIndex.size(); ++i) {
        mergeIndex[i] = swcSize[mergeSwcList[i]].second;
    }
    std::sort(mergeIndex.begin(), mergeIndex.end());
    std::vector<size_t> allIndex(tmpSeperateTree->GetTree().size());
    for (size_t i = 0; i < allIndex.size(); ++i) {
        allIndex[i] = i;
    }
    std::vector<size_t> remainIndex;
    std::set_difference(allIndex.begin(), allIndex.end(), mergeIndex.begin(), mergeIndex.end(), std::back_inserter(remainIndex));
    //std::vector<int> resultType(1+remainIndex.size(),0);
    std::vector<int> resultType(1,0);
    //sometimes the largest tree is not soma tree
    size_t ifFirstSWCSomaNum = 1000000;
    for (size_t i = 0; i < mergeIndex.size(); ++i) {
        if (tmpSeperateTree->GetTypeList()[mergeIndex[i]] == 2) {
            resultType[0] = 2;
            ifFirstSWCSomaNum = mergeIndex[i];
            break;
        }
    }
	
    //std::vector<std::vector<VectorVec5d> > tmpResultTreeList(1+remainIndex.size());
    std::vector<std::vector<VectorVec5d> > tmpResultTreeList(1);
	std::vector<VectorVec5d> tmpResultTree;
    //if soma tree, then put soma curve first
    if (resultType[0] == 2) {
        const std::vector<VectorVec5d>& curSwc = tmpSeperateTree->GetTree()[ifFirstSWCSomaNum];
        for (size_t j = 0; j < curSwc.size();++j) {
            tmpResultTreeList[0].push_back(curSwc[j]);
        }
        for (size_t i = 0; i < mergeSwcList.size(); ++i) {
            if(mergeIndex[i] == ifFirstSWCSomaNum) continue;
            const std::vector<VectorVec5d>& curSwc = tmpSeperateTree->GetTree()[mergeIndex[i]];
            for (size_t j = 0; j < curSwc.size();++j) {
                tmpResultTreeList[0].push_back(curSwc[j]);
            }
        }
    } else {
        for (size_t i = 0; i < mergeSwcList.size(); ++i) {
            const std::vector<VectorVec5d>& curSwc = tmpSeperateTree->GetTree()[mergeIndex[i]];
            for (size_t j = 0; j < curSwc.size();++j) {
                tmpResultTreeList[0].push_back(curSwc[j]);
            }
        }
    }
    //add addSwc
    for (size_t i = 0; i < addSwc.size(); ++i) {
       tmpResultTreeList[0].push_back(addSwc[i]);
    }

	
//    for (size_t i = 0; i < remainIndex.size(); ++i) {
//        tmpResultTreeList[i+1] = tmpSeperateTree->GetTree()[remainIndex[i]];
//        resultType[i+1] = (tmpSeperateTree->GetTypeList()[remainIndex[i]]);
//    }
	tmpTree->GetTree().swap(tmpResultTreeList);
	tmpTree->SetTypeList(resultType);

	return true;
}

ConstDataPointer WeakSWCFilter::GetOutput()
{
	if(!m_Source)
#ifdef _WIN32
		m_Source = std::tr1::shared_ptr<SeperateTree>(new SeperateTree(this));
#else
		m_Source = std::shared_ptr<SeperateTree>(new SeperateTree(this));
#endif
	return m_Source;
}

DataPointer WeakSWCFilter::ReleaseData()
{
	m_Source->ReleaseProcessObject();
	DataPointer tData(m_Source);
	m_Source.reset();
	return tData;
}

void WeakSWCFilter::CubesSetLabel( const std::vector<std::vector<VectorVec5d> >& swcCell, const std::vector<size_t>& SWCId, 
								  VectorVec3i& filledPtSet )
{
	labelBlock.SetSize(origImgPointer->x(), origImgPointer->y(), origImgPointer->z());
	size_t num = SWCId.size();
	filledPtSet.clear();
	VectorVec3i tmpPtSet;
	size_t sum = 0;
	for (size_t i = 0; i < num; ++i) {
		CubesLabel(swcCell[SWCId[i]], tmpPtSet);
		sum += tmpPtSet.size();
		std::copy(tmpPtSet.begin(), tmpPtSet.end(), std::back_inserter(filledPtSet));
	}
	if (sum == 0) {
		labelBlock.SetZero();
		filledPtSet.clear();
	}
}

void WeakSWCFilter::CubesLabel( const std::vector<VectorVec5d>& swcCurve, VectorVec3i& filledPtSet )
{
	int nx = origImgPointer->x() - 1;
	int ny = origImgPointer->y() - 1;
	int nz = origImgPointer->z() - 1;
	size_t ptNum = swcCurve.size();
	VectorVec3i tmpPtSet;
	for (size_t i = 0; i < swcCurve.size(); ++i) {
		VectorVec3d curCurve(swcCurve[i].size());
		for (size_t j = 0; j < swcCurve[i].size(); ++j) {
			curCurve[j] = swcCurve[i][j].block(0,0,3,1);
		}
		tmpPtSet.clear();
		CubeLabel(curCurve, tmpPtSet);
		std::copy(tmpPtSet.begin(), tmpPtSet.end(), std::back_inserter(filledPtSet));
		int x,y,z;
		for(size_t j = 0; j < curCurve.size(); ++j){
			x = std::min(std::max(Round(curCurve[j](0)), 0), nx);
			y = std::min(std::max(Round(curCurve[j](1)), 0), ny);
			z = std::min(std::max(Round(curCurve[j](2)), 0), nz);
			labelBlock(x,y,z) = int(i) + 1;//matlab is j
		}
	}
}

//2016-1-2 add indexImg
void WeakSWCFilter::CubeLabel(const VectorVec3d &curDendrite, VectorVec3i& filledPtSet)
{
	//initial
	const SVolume &origImg = *origImgPointer;
	const SVolume &backImg = *backImgPointer;
	int nxx = origImg.x();
	int nyy = origImg.y();
	int nzz = origImg.z();

	VectorVec5d rawSomaCurve;
	rawSomaCurve.clear();
	Vec5d tmpVec5d;
	for(size_t i = 0; i < curDendrite.size(); ++i){
		tmpVec5d << curDendrite[i](0), curDendrite[i](1), curDendrite[i](2), 0, 0;
		rawSomaCurve.push_back(tmpVec5d);
	}
	VectorVec3d tmpPtCurve;
	Vec3d tmpVec3d;
	for (size_t i = 0; i < rawSomaCurve.size(); ++i){
		tmpVec3d << rawSomaCurve[i](0), rawSomaCurve[i](1), rawSomaCurve[i](2);
		tmpPtCurve.push_back(tmpVec3d);
	}
	//
	if(!curDendrite.empty()){
		std::vector<double> R;//here is list
		std::vector<double> V;
		VectorVec5d resultCurveCopy;
		CalcParmOfCurveNodeList(origImg, backImg, tmpPtCurve, R, V );
		for (size_t i = 0; i < rawSomaCurve.size(); ++i){
			rawSomaCurve[i](3) = R[i];
			rawSomaCurve[i](4) = V[i];
		}
		Vec5d half;
		//
		for (size_t i = 0; i < rawSomaCurve.size() - 1; ++i){
			resultCurveCopy.push_back(rawSomaCurve[i]);
			half = 0.5 * (rawSomaCurve[i] + rawSomaCurve[i + 1]);
			resultCurveCopy.push_back(half);
		}
		resultCurveCopy.push_back(rawSomaCurve[rawSomaCurve.size() - 1]);

		std::vector<double> three;
		for (VectorVec5d::size_type i = 0; i < resultCurveCopy.size(); ++i){
			three.push_back((double)std::min(6, Round(resultCurveCopy[i](3) + 1.5) ) );//2016-1-2
		}
		int id1(0), id2(0), id3(0);
		int xMin, xMax, yMin, yMax, zMin, zMax;
		VectorVec3i test;//test
		for (int ik = 1; ik < 2*int(rawSomaCurve.size())-2; ++ik){
			id1 = Round(resultCurveCopy[ik](0) );
			id2 = Round(resultCurveCopy[ik](1) );
			id3 = Round(resultCurveCopy[ik](2) );

			xMin = std::max(0, id1 - std::max(0, (int)three[ik]));
			xMax = std::min(nxx - 1, id1 + std::max(0, (int)three[ik]));
			yMin = std::max(0, id2 - std::max(0, (int)three[ik]));
			yMax = std::min(nyy - 1, id2 + std::max(0, (int)three[ik]));
			zMin = std::max(0, id3 - std::max(0, (int)three[ik]));
			zMax = std::min(nzz - 1, id3 + std::max(0, (int)three[ik]));

			double threBack(0);

			for (int ii = xMin; ii <= xMax; ++ii){
				for (int jj = yMin; jj <= yMax; ++jj){
					for (int kk = zMin; kk <= zMax; ++kk){
						//2015-6-8
						threBack = double(backImg(ii, jj, kk)) + 10.0 * std::sqrt(double(backImg(ii, jj, kk)));//2016-1-4
						threBack = std::min(threBack, 1.5 * double(backImg(ii, jj, kk)) );
						threBack = std::max(80.0, threBack);
						if (labelBlock(ii, jj, kk) == 0 && double(origImg(ii, jj, kk)) > threBack){
							labelBlock(ii, jj, kk) = -1;
							filledPtSet.push_back(Vec3i(ii,jj,kk));
						}
					}
				}
			}//for
		}//for
		//printf("%d\n",int(test.size()));
	}
}

void WeakSWCFilter::CalcParmOfCurveNodeList(const SVolume &origImg, const SVolume &backImg,
												const VectorVec3d &curNode,
												std::vector<double> &radius, std::vector<double> &rav)
{
	radius.clear();
	rav.clear();

	VectorVec3d::size_type nxx = curNode.size();
	double tmpR(0.0);
	double tmpV(0.0);
	for (VectorVec3d::size_type i = 0; i < nxx; ++i){
		Vec3d tmpdata1;
		tmpdata1(0) = Round(curNode[i](0));//int(curNode[i](0) + 0.5);
		tmpdata1(1) = Round(curNode[i](1));
		tmpdata1(2) = Round(curNode[i](2));
		CalcParmOfCurveNode(origImg, backImg, tmpdata1, tmpR, tmpV);
		radius.push_back(tmpR);
		rav.push_back(tmpV);
	}
}

void WeakSWCFilter::CalcParmOfCurveNode(const SVolume &origImg, const SVolume &backImg,
											const Vec3d &curNode, double &radius, double &wet)
{
	int vx = backImg.x();
	int vy = backImg.y();
	int vz = backImg.z();

	int xMin, xMax, yMin, yMax, zMin, zMax;
	Get3DRegion(xMin, xMax, yMin, yMax, zMin, zMax, Round(curNode(0) ), Round(curNode(1) ), Round(curNode(2) ),
		5, 5, 5, 0, vx - 1, 0, vy - 1, 0, vz - 1);

	int xLen = xMax - xMin + 1;
	int yLen = yMax - yMin + 1;
	int zLen = zMax - zMin + 1;

	Vec3d center(curNode(0) - xMin, curNode(1) - yMin, curNode(2) - zMin);//SL
	Volume<double> procImg;
	procImg.SetSize(xLen, yLen, zLen);//MM1

	for (int i = xMin; i <= xMax; ++i){
		for (int j = yMin; j <= yMax; ++j){
			for (int ij = zMin; ij <= zMax; ++ij){
				procImg(i-xMin, j-yMin, ij-zMin) = double(origImg(i, j, ij)) - double(backImg(i, j, ij))
					- 3.0 * std::sqrt(double(backImg(i, j, ij)));
			}
		}
	}//for

	radius = 0.05;//r
	wet = 0.0;//v
	MatXd distWet(2, 4); distWet.setZero();
	double vv0 = 0.0;
	double vv1 = 0.0;

	for (double i = 0; i < xLen; ++i){
		for (double j = 0; j < yLen; ++j){
			for (double ij = 0; ij < zLen; ++ij){
				Vec3d dist(i - center(0), j - center(1), ij - center(2));
				double distNorm = dist.norm();
				if (distNorm <= 1.0){ //
					distWet(0, 0) += 1.0;
					if (procImg(i, j, ij) > 0.0){
						vv0 += procImg(i, j, ij);
						distWet(1, 0) += 1.0;
					}
				}//if

				if (distNorm <= 2.0){ //
					distWet(0, 1) += 1.0;
					if (procImg(i, j, ij) > 0){
						vv1 += procImg(i, j, ij);
						distWet(1, 1) += 1.0;
					}
				}

				if (distNorm <= 3.0){ //
					distWet(0, 2) += 1.0;
					if (procImg(i, j, ij) > 0)
						distWet(1, 2) += 1.0;
				}

				if (distNorm <= 4.0){ //
					distWet(0, 3) += 1.0;
					if (procImg(i, j, ij) > 0)
						distWet(1, 3) += 1.0;
				}
			}//for
		}
	}//for

	Vec4d procDistWet = distWet.row(1).array() / ( distWet.row(0).array() + 0.0001);
	for (int i = 1; i <= 4; ++i){
		if (procDistWet(4-i) > 0.5){
			radius = 5.0 - (double)i;
			break;
		}
	}

	if (radius > 2.1){//warning !!
		radius = radius / std::pow(distWet(0, int(radius + 0.5) - 1) / distWet(1, int(radius + 0.5) - 1), 1.0/3.0);
		wet = vv1 / distWet(1, 1);
	}

	if (radius < 2.1){
		if (distWet(1,1) > 0.0){
			radius = 2.0 / std::pow(distWet(0, 1) / distWet(1, 1), 1.0/3.0);
			wet = vv1 / distWet(1, 1);
		}
		if (std::abs(distWet(1, 1) - 0.0) < EXPO && distWet(1, 0) > 0.0 ){
			radius = 1.0 / std::pow(distWet(0, 0) / distWet(1, 0), 1.0/3.0);
			wet = vv0 / distWet(1, 0);
		}
	}
}

void WeakSWCFilter::RegiongrowFusedTraces( const VectorVec3i& filledPtSet, double thre )
{
    //
    VectorVec3i test(filledPtSet);
	VectorVec3i curPtSet;
	VectorVec3i initPtSet(filledPtSet);
	for (size_t i = 0; i < 10; ++i) {
		RegionInflationModifyV2(initPtSet, labelBlock, thre, curPtSet);
        std::copy(curPtSet.begin(), curPtSet.end(), std::back_inserter(test));//
		if (!curPtSet.empty()) {
			initPtSet.swap(curPtSet);
		}else{
			break;
		}
	}
}

void WeakSWCFilter::RegionInflationModifyV2(const VectorVec3i &curPoints,
                                              IVolume &growLabelMatrix, double threv,
                                              VectorVec3i &growPoints)
{
    //initial
    const SVolume &origImg = *origImgPointer;
    const SVolume &backImg = *backImgPointer;
    int xBdy = origImg.x() - 1;
    int yBdy = origImg.y() - 1;
    int zBdy = origImg.z() - 1;

    size_t nxx = curPoints.size();
    growPoints.clear();
    /*
     * area grow
    */
    bool flag1;
    bool flag2;
    int xMin, xMax, yMin, yMax, zMin, zMax;
    VectorVec3i tmpGrowPts;
    for(size_t i = 0; i < nxx; ++i){
        Get3DRegion(xMin, xMax, yMin, yMax, zMin, zMax, curPoints[i](0), curPoints[i](1), curPoints[i](2),
                    2,2,1, 0, xBdy, 0, yBdy, 0, zBdy);//2015-2-8
                tmpGrowPts.clear();
        tmpGrowPts.reserve(75);
        for(int ii = xMin; ii <= xMax; ++ii){
            for(int jj = yMin; jj <= yMax; ++jj){
                for(int ij = zMin; ij <= zMax; ++ij){
                    flag1 = double(origImg(ii,jj,ij)) > double(backImg(ii,jj,ij)) + threv * std::sqrt(double(backImg(ii,jj,ij)));
                    flag2 = double(origImg(ii,jj,ij)) > double(backImg(ii,jj,ij)) + 25;//2016-1-2
                    if(0 == growLabelMatrix(ii,jj,ij) && (flag1 || flag2 ))
                        tmpGrowPts.push_back(Vec3i(ii,jj,ij));
                }
            }
        }//for
        if(tmpGrowPts.size() > 2){
            std::copy(tmpGrowPts.begin(), tmpGrowPts.end(), std::back_inserter(growPoints));
            for(size_t j = 0; j < tmpGrowPts.size(); ++j)
                growLabelMatrix(tmpGrowPts[j](0), tmpGrowPts[j](1), tmpGrowPts[j](2)) = -1;//2016-1-2
        }
    }
}

void WeakSWCFilter::TreeStructureFusedDetection( const std::vector<std::vector<VectorVec5d> >& swcCell, const std::vector<std::vector<size_t> >& connectStruct,
												const std::vector<size_t>& candidateSWCId, std::vector<size_t>& connectionLabel, 
                                                std::vector<VectorVec5d>& addSwc)
{
    //addSwc mustnot clear
	size_t num = candidateSWCId.size();
	std::vector<size_t> candidateSWCIdLabel;
	int xBdy = labelBlock.x() - 1;
	int yBdy = labelBlock.y() - 1;
	int zBdy = labelBlock.z() - 1;
	int x,y,z;
    std::vector<size_t> mainSwcIndList;
    for (size_t i = 0; i < connectStruct.size(); ++i) {
        std::copy(connectStruct[i].begin(), connectStruct[i].end(), std::back_inserter(mainSwcIndList));
    }
	//
	VectorVec3i curPtSet;
	for (size_t i = 0; i < num; ++i) {
		const std::vector<VectorVec5d>& curSWC = swcCell[candidateSWCId[i]];//candidateSWCId[i] remain collide swc index
		int kk = 0;
        std::vector<VectorVec5d> tmpAddSwc;
		for (size_t j = 0; j < curSWC.size(); ++j) {//j remain collide swc curve index
			curPtSet.clear();
			curPtSet.resize(curSWC[j].size());
			for (size_t ij = 0; ij < curSWC[j].size(); ++ij) {
				x = std::min(std::max(Round(curSWC[j][ij](0)), 0), xBdy);
				y = std::min(std::max(Round(curSWC[j][ij](1)), 0), yBdy);
				z = std::min(std::max(Round(curSWC[j][ij](2)), 0), zBdy);
				curPtSet[ij]<<x,y,z;
			}
			int detectionNum = TreeStructureFusedDetectionSub(curPtSet);//collide main curve index
            if (detectionNum != 0) {
                ++kk;
                tmpAddSwc.push_back(VectorVec5d());
                VectorVec5d tmp;
                MakeNearestNode2SwcFromColldeCurves(swcCell, mainSwcIndList, detectionNum, candidateSWCId[i], j, tmp);
                tmpAddSwc.back().swap(tmp);
            }
		}
        if (kk > 0) {
            candidateSWCIdLabel.push_back(i);
            if (tmpAddSwc.size() == 1) {
                addSwc.push_back(tmpAddSwc[0]);
            }else{
                double minDist = 1000000.0;
                double tmpDist = 0.0;
                size_t finalind = 0;
                for (size_t k = 0; k < tmpAddSwc.size(); ++k) {
                    tmpDist = (tmpAddSwc[k][0].block(0,0,3,1) - tmpAddSwc[k][1].block(0,0,3,1)).norm();
                    if (tmpDist < minDist) {
                        minDist = tmpDist;
                        finalind = k;
                    }
                }
                addSwc.push_back(tmpAddSwc[finalind]);
            }
        }
	}
	connectionLabel.clear();
	for (size_t i = 0; i < candidateSWCIdLabel.size(); ++i) {
		connectionLabel.push_back(candidateSWCId[candidateSWCIdLabel[i] ] );
	}
}

void WeakSWCFilter::TransferVectorVec5d2VectorVec3d( const VectorVec5d& orig, VectorVec3d& dst )
{
	dst.clear();
	dst.resize(orig.size());
	for (size_t i = 0; i < orig.size(); ++i) {
		dst[i] = orig[i].block(0,0,3,1);
	}
}

int WeakSWCFilter::TreeStructureFusedDetectionSub( const VectorVec3i& curCurve )
{
	int detectionNum = 0;
	VectorVec3i flatPtSet;
	for (size_t i = 0; i < curCurve.size(); ++i) {
		if (labelBlock(curCurve[i](0), curCurve[i](1), curCurve[i](2)) != 0) {
            //printf("%d\n",labelBlock(curCurve[i](0), curCurve[i](1), curCurve[i](2)) );
			//DetectionNum=TreeStructureFused_Detection11(dataP,LabelBlock);
			VectorVec3i initPtSet;
            initPtSet.push_back((curCurve[i]));
			indexImg.SetZero();
			for (size_t k = 0; k < 10; ++k) {
				RegionFlationBinary(initPtSet, flatPtSet);
				for (size_t j = 0; j < flatPtSet.size(); ++j) {
					if (labelBlock(flatPtSet[j](0), flatPtSet[j](1), flatPtSet[j](2)) > 0) {
						detectionNum = labelBlock(flatPtSet[j](0), flatPtSet[j](1), flatPtSet[j](2));//collide main curve index
						break;
					}
				}
				if (detectionNum > 0 || flatPtSet.empty()) {
                    /*FILE* fp = fopen("E:/nimei.swc","w");
                    int id = 0;
                    for (size_t k = 0; k < flatPtSet.size(); ++k) {
                        fprintf(fp, "%d 1 %lf %lf %lf 1.0 -1\n",++id, double(flatPtSet[k](0)),double(flatPtSet[k](1)),double(flatPtSet[k](2)));
                    }
                    fclose(fp);
                    system("pause");*/
					break;
				} else initPtSet.swap(flatPtSet);
			}
			//
			if (detectionNum > 0) break;
		}
	}
	return detectionNum;
}

void WeakSWCFilter::RegionFlationBinary( const VectorVec3i& curPoints, VectorVec3i& growPoints )
{
	//initial
    const SVolume &origImg = *origImgPointer;
    const SVolume &backImg = *backImgPointer;
    int xBdy = origImg.x() - 1;
    int yBdy = origImg.y() - 1;
    int zBdy = origImg.z() - 1;

    size_t nxx = curPoints.size();
    growPoints.clear();
    /*
     * area grow
    */
    int xMin, xMax, yMin, yMax, zMin, zMax;
    VectorVec3i tmpGrowPts;
    for(size_t i = 0; i < nxx; ++i){
        Get3DRegion(xMin, xMax, yMin, yMax, zMin, zMax, curPoints[i](0), curPoints[i](1), curPoints[i](2),
                    2,2,1, 0, xBdy, 0, yBdy, 0, zBdy);//2015-2-8
                tmpGrowPts.clear();
        tmpGrowPts.reserve(75);
        for(int ii = xMin; ii <= xMax; ++ii){
            for(int jj = yMin; jj <= yMax; ++jj){
                for(int ij = zMin; ij <= zMax; ++ij){
					if(0 == indexImg(ii,jj,ij) && std::abs(labelBlock(ii,jj,ij)) > 0)
                        tmpGrowPts.push_back(Vec3i(ii,jj,ij));
                }
            }
        }//for
        if(tmpGrowPts.size() > 2){
            std::copy(tmpGrowPts.begin(), tmpGrowPts.end(), std::back_inserter(growPoints));
            for(size_t j = 0; j < tmpGrowPts.size(); ++j)
                indexImg(tmpGrowPts[j](0), tmpGrowPts[j](1), tmpGrowPts[j](2)) = 1;//2016-1-2
        }
    }
}

void WeakSWCFilter::MakeNearestNode2SwcFromColldeCurves( const std::vector<std::vector<VectorVec5d> >& swcCell, 
                                                    const std::vector<size_t>& mainSwcIndList, 
                                                    int detectionNum, int remainCollideSwcInd, int remainCollideCurveInd, 
                                                    VectorVec5d& addCurve )
{
    Vec5d collideNode1, collideNode2;
    const VectorVec5d& remainCollideCurve = swcCell[remainCollideSwcInd][remainCollideCurveInd];
    double minDist = 1000000.0;
    double tmpDist = 0.0;
    size_t mainSwcInd, mainNodeInd, remainCollideNodeInd;
    --detectionNum;//it is from 1, so minus 1
    for (size_t i = 0; i < mainSwcIndList.size(); ++i) {
        if(swcCell[mainSwcIndList[i]].size() < detectionNum + 1) continue;
        const VectorVec5d& curCurve = swcCell[mainSwcIndList[i]][detectionNum];
        for (size_t j = 0; j < curCurve.size(); ++j) {
            for (size_t ij = 0; ij < remainCollideCurve.size(); ++ij) {
                tmpDist = (curCurve[j].block(0,0,3,1) - remainCollideCurve[ij].block(0,0,3,1)).norm();
                if (tmpDist < minDist) {
                    mainSwcInd = mainSwcIndList[i];
                    mainNodeInd = j;
                    remainCollideNodeInd = ij;
                    minDist = tmpDist;
                }
            }
        }
    }
    collideNode1 = swcCell[mainSwcInd][detectionNum][mainNodeInd];
    collideNode2 = remainCollideCurve[remainCollideNodeInd];
    /*tmpDist = (collideNode1 - collideNode2).norm();
    Vec3d dir = (collideNode1.block(0,0,3,1) - collideNode2.block(0,0,3,1));
    dir.normalize();*/
    addCurve.clear();
    addCurve.push_back(collideNode1);
    addCurve.push_back(collideNode2);
}

