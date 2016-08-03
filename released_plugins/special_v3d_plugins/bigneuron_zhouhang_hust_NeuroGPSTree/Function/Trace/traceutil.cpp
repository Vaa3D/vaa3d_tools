#include "traceutil.h"
#include "../../ngtypes/volume.h"
#include <vector>
#include <deque>
#include <algorithm>
#include <numeric>
#include "Function/volumealgo.h"

void TraceUtil::GetGradientVectorFlowForTrace(const Volume<double> &sphereRayWet, Volume<double> &smoothRay)
{
    smoothRay.SetSize(sphereRayWet.x(), sphereRayWet.y(), sphereRayWet.z());
    std::vector<double> smoothOneRayWet;

    for (int i = 0; i < sphereRayWet.y(); ++i){
        for (int j = 0; j < sphereRayWet.z(); ++j){//角度2
            std::vector<double> tmpSphere;
            smoothOneRayWet.clear();
            for (int ij = 0; ij < sphereRayWet.x(); ++ij){//长度
                tmpSphere.push_back((double)sphereRayWet(ij,i,j));
            }
            SmoothGradientCurves(tmpSphere, smoothOneRayWet);
            for (std::vector<double>::size_type ij = 0; ij < smoothOneRayWet.size(); ++ij){
                smoothRay(ij,i,j) = smoothOneRayWet[ij];
            }
        }
    }
}

void TraceUtil::SmoothGradientCurves(const std::vector<double> &oneRayWet, std::vector<double> &smoothOneRayWet)
{
    //TODO:check this function compared to matlab code
    smoothOneRayWet.clear();
    std::vector<double> procOneRayWet(oneRayWet);
    int rayLength = int(procOneRayWet.size());

    for (int i = 0; i < rayLength; ++i){
        procOneRayWet[i] = procOneRayWet[i] < 600.0 ? procOneRayWet[i] : 600.0;
    }

    std::deque<double> tmpDiff;//原始梯度
    //diff_vector<double>(ttk, tmp_diff);//interp1
    std::adjacent_difference(procOneRayWet.begin(), procOneRayWet.end(), std::back_inserter(tmpDiff));
    tmpDiff.pop_front();//do not need first element
    std::vector<double> diffRay;

    //插值
    diffRay.resize(2 * tmpDiff.size() - 1);
    for(std::vector<double>::size_type i = 0; i < tmpDiff.size() - 1; ++i){
        diffRay[i * 2] = tmpDiff[i];
        diffRay[i * 2 + 1] = (tmpDiff[i] + tmpDiff[i + 1]) * 0.5;
    }
    diffRay[diffRay.size() - 1] = tmpDiff.back();
    //TODO:2014-3-17
    std::vector<double>::size_type diffRayLength = diffRay.size();

    std::vector<double> interplRay;
    //Interpl_2_Mean(ttk, ttks);
    interplRay.resize(2 * procOneRayWet.size() - 2);
    for(std::vector<double>::size_type i = 0; i < procOneRayWet.size() - 2; ++i){
        interplRay[i * 2] = procOneRayWet[i];
        interplRay[i * 2 + 1] = (procOneRayWet[i] + procOneRayWet[i + 1]) * 0.5;
    }
    //Change:2014-3-21-13-38
    interplRay[interplRay.size() - 2] = procOneRayWet[procOneRayWet.size() - 2];
    interplRay[interplRay.size() - 1] = procOneRayWet.back();
    //interplRay[diffRay.size() - 1] = procOneRayWet.back();

    std::vector<double> bufferRay(diffRay);
    //std::vector<double> yy1;// = yy;
    //yy1 = max(-yy, 0);
    double tmpYY;
    for (std::vector<double>::size_type i = 0; i < bufferRay.size(); ++i){
        tmpYY = -bufferRay[i] > 0.0 ? -bufferRay[i] : 0.0;
        tmpYY *= -100.0 / interplRay[1 + i];
        bufferRay[i] = tmpYY;
        //yy1.push_back( tmpYY );
    }

    /*平滑梯度*/
    for (int ddk = 1; ddk < 101; ++ddk){//只是循环100次
        for (std::vector<double>::size_type i =1; i < diffRayLength - 1; ++i){
            bufferRay[i] = ((std::abs)(diffRay[i]) * diffRay[i]
                            + 2.0 * (diffRay[i-1]+diffRay[i+1])) / ((std::abs)(diffRay[i]) + 4.0);
            //bufferRay = yy1;
            diffRay = bufferRay;
        }
    }

    std::vector<int> tts;
    //generate_n(back_inserter(tts), (int)(yy.size() / 2) + 1,GenArray<int>(0, 2));
    for(std::vector<double>::size_type i = 0; i <= bufferRay.size() / 2; ++i){
        tts.push_back(i * 2);
    }

    for (std::vector<int>::size_type i = 0; i < tts.size(); ++i){
        smoothOneRayWet.push_back(bufferRay[tts[i]]);
    }

    for (std::vector<double>::size_type i = 0; i < smoothOneRayWet.size(); ++i){
        smoothOneRayWet[i] = smoothOneRayWet[i] > 0.0? 0.0 : (-smoothOneRayWet[i]);
    }
}

void TraceUtil::ExtractSubRegionOP(const Vec3d &currentPoint, Volume<NGCHAR> &locIndexImg, VectorVec3d &extractedPtSet)
{
    ///------------------初始化-----------------
    int i = 0;

    //TODO:2014-3-18 15:33
    extractedPtSet.clear();//重复清除不要紧吧？
    VectorVec3d tempPos1;		//点集
    VectorVec3d tempPos2;		//临时存放
    tempPos2.push_back(currentPoint);			//为了第一次不空
    ///----------------中心点初始化-----------------
    locIndexImg( (int)(currentPoint(0)) , (int)(currentPoint(1)) ,  (int)(currentPoint(2)) ) = 0;
    extractedPtSet.push_back(currentPoint);//先放入中心点

    tempPos1.push_back(currentPoint);//用于扩张的基础区域点
    ///----------------开始-------------------------
    while(!tempPos2.empty() && i < 1280){
        ++i;
        //Change : 2014-3-19-9-24
        //ExtractSubRegion2(tempPos2, tmp_bSrcData, tempPos1, thre);
        ExtractSubRegionAboveThrev(tempPos1, tempPos2, locIndexImg);
        //必须是back_inserter,在尾部添加
        //std::copy(tempPos2.begin(), tempPos2.end(), back_inserter(extractedPtSet));
        VectorVec3d::size_type oSize = tempPos2.size();
        int preSize = int(extractedPtSet.size());
        extractedPtSet.resize(oSize + preSize);
        //Bug Fix:2014-3-21-15-16
        std::copy(tempPos2.begin(), tempPos2.end(), extractedPtSet.begin() + preSize);
        //tempPos1.clear();
        //std::copy(tempPos2.begin(), tempPos2.end(), back_inserter(tempPos1));
        tempPos1 = tempPos2;
    }
}

void TraceUtil::ExtractSubRegionAboveThrev(const VectorVec3d &currentPtSet, VectorVec3d &extractedPtSet, Volume<NGCHAR> &indexImg)
{
    ///-------------------------------------
    int i,j,ij,n;

    int nx	= indexImg.x();
    int ny	= indexImg.y();
    int nz	= indexImg.z();

    int nxx	= int(currentPtSet.size());
    int xMin,xMax;
    int yMin,yMax;
    int zMin,zMax;

    int windowVal;
    int threv = 100;
    extractedPtSet.clear();//必须清空
    ///-------------------------------------
    for (n = 0 ; n < nxx; ++n){

        int id1 = Round(currentPtSet[n](0));
        int id2 = Round(currentPtSet[n](1));
        int id3 = Round(currentPtSet[n](2));

        if (id1 > 1 && id1 < nx-2 &&id2 > 1 && id2 < ny-2 && id3 > 1 && id3 < nz-2){
            xMin = id1 -1; xMax = id1 +1;
            yMin = id2 -1; yMax = id2 +1;
            zMin = id3 -1; zMax = id3 +1;
            windowVal = 0;
            for (i = xMin; i <= xMax; ++i){
                for (ij = zMin; ij <= zMax; ++ij){
                    for (j = yMin; j <= yMax; ++j)
                        windowVal += indexImg(i , j , ij );
                }
            }//for
            windowVal /= 255;//warning
            Vec3d pt;
            if (windowVal > 4){
                for (i = xMin; i <= xMax; ++i){
                    for (j = yMin; j <= yMax; ++j){
                        for (ij = zMin; ij <= zMax; ++ij){
                            if (indexImg(i ,j , ij ) > threv){//only if greater than zero
                                indexImg(i ,j , ij ) = 0;
                                pt << i,j,ij;
                                extractedPtSet.push_back(pt);
                            }
                        }
                    }
                }
            }//if
        }//if
    }//for
}

int TraceUtil::sign(double a)
{
    if(a < 0) return -1;
    else if(a > 0) return 1;
    return 0;
}

void TraceUtil::GetPartVectorVec3d(const VectorVec3d &orig, int minId, int maxId, VectorVec3d &dst)
{
    if(minId > maxId || maxId >= (int)orig.size() || minId < 0) {
        dst.clear();
        return;
    }
    dst.resize(maxId - minId + 1);
    for(int i = minId; i <= maxId; ++i){
        dst[i-minId]=orig[i];
    }
}

void TraceUtil::GetPartVectorVec3d(const VectorVec5d &orig, int minId, int maxId, VectorVec3d &dst)
{
    if(minId > maxId || maxId >= (int)orig.size() || minId < 0) {
        dst.clear();
        return;
    }
    dst.resize(maxId - minId + 1);
    Vec3d tmp;
    for(int i = minId; i <= maxId; ++i){
        tmp << orig[i](0),orig[i](1),orig[i](2);
        dst[i-minId]=tmp;
    }
}

bool TraceUtil::IsEqualDouble(double a, double b)
{
    return std::abs(a - b) < 0.0001;
}

