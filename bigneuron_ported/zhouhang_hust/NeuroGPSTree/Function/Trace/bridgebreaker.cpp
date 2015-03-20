//
#include <algorithm>
#include <numeric>
#ifdef _WIN32
#include <ctime>
#include <omp.h>
#else
#include <sys/time.h>
#endif
#include <Eigen/Geometry>

#include "bridgebreaker.h"
#include "../../ngtypes/tree.h"
#include "../../ngtypes/volume.h"
#include "../../ngtypes/soma.h"
#include "../contourutil.h"
#include "./traceutil.h"

const double EXPO = 0.0001;

struct LessPair2{
    bool operator() (const std::pair<int, double>& lhs, const std::pair<int, double>& rhs){
        return lhs.second < rhs.second;
    }
};

BridgeBreaker::BridgeBreaker()
{
    identifyName = "BridgeBreaker";
    m_Source = std::shared_ptr<TreeConnect>(new TreeConnect(this));
}

BridgeBreaker::~BridgeBreaker()
{

}

bool BridgeBreaker::Update()
{
    if(!m_Input  ){
        printf("error occured in %s\n", identifyName.c_str());
        return false;
    }
    if(m_Input->GetProcessObject()){//|| !m_Soma
        if(!m_Input->GetProcessObject()->Update()){
            printf("error occured in %s\n", identifyName.c_str());
            return false;
        }
    }
    CTreeCurvePointer tmpTree = std::dynamic_pointer_cast<const TreeCurve>(m_Tree);
    CTreeConnectPointer tmpConnect = std::dynamic_pointer_cast<const TreeConnect>(m_Input);
    CSVolumePointer tmpOrigImg = std::dynamic_pointer_cast<const SVolume>(m_OrigImg);
	resultIndex.clear();
    VectorMat2i resultConnect;
	//2015-3-3 if neuroGPS-Tree get 0 or 1 soma, just skip breaking step and return;
	std::shared_ptr<const Soma > tmpSoma =
            std::dynamic_pointer_cast<const Soma>(m_Soma);
	if( tmpSoma->size() < 2){
        printf("there is one or no somas. skip break.\n");
		std::shared_ptr<TreeConnect> tmp_Source = std::dynamic_pointer_cast<TreeConnect>(m_Source);
		tmp_Source->SetConnect((*tmpConnect).GetConnect());
		//resultConnect = (*tmpConnect).GetConnect();
		//resultIndex.clear();
		return true;
	}
#ifdef _WIN32
    clock_t beg = clock();
#else
    timeval start1, end1;
    gettimeofday(&start1, 0);
#endif
    if(!DeleteErrorDendConnect((*tmpTree).GetCurve(), (*tmpConnect).GetConnect(),
                               (*tmpOrigImg), resultConnect, resultIndex))
        return false;
#ifdef _WIN32
    clock_t end = clock();
    printf("%d ms eclipsed in break. \n", int(end - beg));
#else
    gettimeofday(&end1, 0);
    double timeuse=1000000*(end1.tv_sec-start1.tv_sec)+end1.tv_usec-start1.tv_usec;
    timeuse/=1000000;
    printf("%lf s eclipsed in break.\n", timeuse);
#endif
    std::shared_ptr<TreeConnect> tmp_Source = std::dynamic_pointer_cast<TreeConnect>(m_Source);
    tmp_Source->Swap(resultConnect);
    return true;
}

ConstDataPointer BridgeBreaker::GetOutput()
{
    if(!m_Source)
        m_Source = std::shared_ptr<TreeConnect>(new TreeConnect(this));
    return m_Source;
}

DataPointer BridgeBreaker::ReleaseData()
{
    m_Source->ReleaseProcessObject();
    DataPointer tData(m_Source);
    m_Source.reset();
    return tData;
}

VectorVec4d BridgeBreaker::getResultIndex() const
{
    return resultIndex;
}

void BridgeBreaker::setResultIndex(const VectorVec4d &value)
{
    resultIndex = value;
}



bool BridgeBreaker::DeleteErrorDendConnect(const std::vector<VectorVec5d> &rawDendList,
                                           const VectorMat2i &rawDendConInfo,
                                           const Volume<unsigned short> &origImg,
                                           VectorMat2i &newDendConInfo, VectorVec4d &deletedDendId)
{
    int nxx = rawDendList.size();
    newDendConInfo = rawDendConInfo;
    int tmpCurConnectIndex = 0;
    int ddst = 0;
#ifdef _WIN32
#pragma omp parallel for
#endif
    for(int i = 0; i < nxx; ++i){
        //printf("cal %d\n",i);
        const Mat2i& currentDendConInfo = rawDendConInfo[i];
        Mat2i tmpDendConInfo;
//#ifdef _WIN32
//    clock_t beg = clock();
//#else
//    timeval start1, end1;
//    gettimeofday(&start1, 0);
//#endif
        if(currentDendConInfo(0,0) != 0 && currentDendConInfo(0,1) != 0 ){
            tmpDendConInfo = currentDendConInfo;
            ++ddst;
            const VectorVec5d& currentDendCurve = rawDendList[i];
            Vec2d connectFlag(0.5,0.5);//0.7 * Vec2d(std::abs(double(currentDendConInfo(0,0) ) ), std::abs(double(currentDendConInfo(0,1))) );
            //Vec2d tmpConnectStat; tmpConnectStat.setZero();
            MatXd initHeadRadWet(2,4), initTailRadWet(2,4);
            VectorVec5d tmpCurLine;
            for(VectorVec5d::size_type j = 3; j < currentDendCurve.size() -4; ++j){//Change : '<'
                tmpCurLine.push_back(currentDendCurve[j]);
            }
            CalcRayBurstRadWet(tmpCurLine, origImg, initHeadRadWet, initTailRadWet);
            int headTailFlag = 0;
            JudgeHeadTailForDendCurve(initHeadRadWet, initTailRadWet, connectFlag, headTailFlag);
            int headConjHeadTailFlag = 0, tailConjHeadTailFlag = 0;
            //2014-3-27-21-46
            if(currentDendConInfo(0,0) > 0){
                VectorVec5d conjHeadDendCurve;
                for(VectorVec5d::size_type j = 3; j < rawDendList[currentDendConInfo(0,0) - 1].size() -4; ++j){
                    conjHeadDendCurve.push_back(rawDendList[currentDendConInfo(0,0) - 1][j]);
                }
                CalcRayBurstRadWet(conjHeadDendCurve, origImg, initHeadRadWet, initTailRadWet);
                const Mat2i& conjHeadDendConInfo = rawDendConInfo[currentDendConInfo(0,0) - 1];
                //connectFlag=0.5*sign([abs(conjHeadDendConInfo(1,1)),abs(conjHeadDendConInfo(1,2))]);
                connectFlag = 0.5 * Vec2d( TraceUtil::sign(std::abs<double>(double(conjHeadDendConInfo(0,0) ) )),
                                          TraceUtil::sign(std::abs<double>(double(conjHeadDendConInfo(0,1)))) );
                JudgeHeadTailForDendCurve(initHeadRadWet, initTailRadWet, connectFlag, headConjHeadTailFlag);

            }

            if(currentDendConInfo(0,1) > 0){
                VectorVec5d conjHeadDendCurve;
                for(VectorVec5d::size_type j = 3; j < rawDendList[currentDendConInfo(0,1) - 1].size() -4; ++j){
                    conjHeadDendCurve.push_back(rawDendList[currentDendConInfo(0,1) - 1][j]);
                }
                CalcRayBurstRadWet(conjHeadDendCurve, origImg, initHeadRadWet, initTailRadWet);
                const Mat2i& conjTailDendConInfo = rawDendConInfo[currentDendConInfo(0,1) - 1];
                connectFlag = 0.5 * Vec2d(TraceUtil::sign(std::abs<double>(double(conjTailDendConInfo(0,0) ) )),
                                          TraceUtil::sign(std::abs<double>(double(conjTailDendConInfo(0,1)))) );
                JudgeHeadTailForDendCurve(initHeadRadWet, initTailRadWet, connectFlag, tailConjHeadTailFlag);

            }

            if( headTailFlag == 0 && currentDendConInfo(0,0) > 0 && currentDendConInfo(0,1) > 0){
                int nxxs = currentDendCurve.size();
                Vec3d rawHeadConjDir;
                rawHeadConjDir.setZero();
                for(int inxx = 0; inxx < std::min(nxxs, 8); ++inxx){
                    rawHeadConjDir += (Vec3d(currentDendCurve[inxx + 2](0), currentDendCurve[inxx + 2](1), currentDendCurve[inxx + 2](2)) -
                                   Vec3d(currentDendCurve[1](0), currentDendCurve[1](1), currentDendCurve[1](2))).normalized();
                }
                rawHeadConjDir.normalize();
                Vec3d rawTailConjDir;
                rawTailConjDir.setZero();
                for(int inxx = nxxs - 3; inxx >= std::max(nxxs - 9, 0); --inxx){
                    rawTailConjDir += (Vec3d(currentDendCurve[inxx](0), currentDendCurve[inxx](1), currentDendCurve[inxx](2)) -
                                   Vec3d(currentDendCurve[nxxs - 2](0), currentDendCurve[nxxs - 2](1), currentDendCurve[nxxs - 2](2))).normalized();
                }
                rawTailConjDir.normalize();
                Vec3d headConjDendCurveVector;
                Vec3d tailConjDendCurveVector;
                const VectorVec5d &headConnectLine = rawDendList[currentDendConInfo(0,0) - 1];
                const VectorVec5d &tailConnectLine = rawDendList[currentDendConInfo(0,1) - 1];
                CalcConnectKnotLocalDirection(headConnectLine, currentDendCurve[0], headConjHeadTailFlag, headConjDendCurveVector);
                CalcConnectKnotLocalDirection(tailConnectLine, currentDendCurve.back(), tailConjHeadTailFlag, tailConjDendCurveVector);

                double tmpvalue1 = (rawTailConjDir.transpose() * tailConjDendCurveVector);
                double tmpvalue2 = (rawHeadConjDir.transpose() * headConjDendCurveVector);
                if( (0.0 <= tmpvalue1 && tmpvalue2 <= 0.0) && std::abs(tmpvalue1 - tmpvalue2) > 0.1){
                    tmpDendConInfo << 0, currentDendConInfo(0,1), 0,0;
                    ++tmpCurConnectIndex;
                    deletedDendId.push_back(Vec4d(double(i), currentDendCurve[0](0), currentDendCurve[0](1), currentDendCurve[0](2)));
                }

                if( (0.0 >= tmpvalue1 && tmpvalue2 >= 0) &&
                        std::abs(tmpvalue1 - tmpvalue2) > 0.1){
                    tmpDendConInfo << currentDendConInfo(0,0),0, 0,0;
                    ++tmpCurConnectIndex;
                    int back = currentDendCurve.size() - 1;
                    deletedDendId.push_back(Vec4d(double(i), currentDendCurve[back](0), currentDendCurve[back](1), currentDendCurve[back](2)));
                }


            }

            //if connect to curve'middle part instead of soma, save head connection and delete tail connection
            //if connect to connected-curve's head and the direction is same,
            //then do nothing.
            if( headTailFlag == 1 && currentDendConInfo(0,1) > 0){
                int knotFlag;
                //knotFlag == 1 : delete ; other : do nothing
                KnotAnalysis(currentDendCurve, rawDendList[currentDendConInfo(0,1) - 1], 1, 0.85, tailConjHeadTailFlag, knotFlag);
                if(knotFlag == 1){
                    tmpDendConInfo << currentDendConInfo(0,0), 0, 0, 0;
                    ++tmpCurConnectIndex;
                    int back = currentDendCurve.size() - 1;
                    deletedDendId.push_back(Vec4d(double(i), currentDendCurve[back](0), currentDendCurve[back](1), currentDendCurve[back](2)));
                }

            }

            //headTailFlag== -1
            if( headTailFlag == -1 && currentDendConInfo(0,0) > 0){//pd == -1
                int knotFlag;
                KnotAnalysis(currentDendCurve, rawDendList[currentDendConInfo(0,0) - 1], 0, 0.85, headConjHeadTailFlag, knotFlag);
                if(knotFlag == 1 ){
                    tmpDendConInfo << 0, currentDendConInfo(0,1), 0, 0;
                    ++tmpCurConnectIndex;
                    deletedDendId.push_back(Vec4d(double(i), currentDendCurve[0](0), currentDendCurve[0](1), currentDendCurve[0](2)));
                }

            }

            newDendConInfo[i] = tmpDendConInfo;
        }

        if(currentDendConInfo(0,0) < 0 && currentDendConInfo(0,1) < 0){
            ++ddst;
            const VectorVec5d& currentDendCurve = rawDendList[i];
            VectorVec2d currentCurveRadWet;
            for(VectorVec5d::size_type j = 0; j < currentDendCurve.size(); ++j){
                currentCurveRadWet.push_back(Vec2d(currentDendCurve[j](3), currentDendCurve[j](4)));
            }
            int radWetLen = currentCurveRadWet.size();
            double headMeanRadius = 0.0, tailMeanRadius = 0.0;
            for(int j = 0; j < std::min(20, radWetLen); ++j){
                headMeanRadius += currentCurveRadWet[j](0);
            }
            headMeanRadius /= double(std::min(20, radWetLen));
            for(int j = std::max(radWetLen - 21, 0); j < radWetLen; ++j){
                tailMeanRadius += currentCurveRadWet[j](0);
            }
            tailMeanRadius /= double(radWetLen - std::max(radWetLen - 21, 0));

            if(headMeanRadius < tailMeanRadius){
                tmpDendConInfo << 0, currentDendConInfo(0,1), 0,0;
                ++tmpCurConnectIndex;
                deletedDendId.push_back(Vec4d(double(i), currentDendCurve[0](0), currentDendCurve[0](1), currentDendCurve[0](2)));
            }
            else{
                tmpDendConInfo << currentDendConInfo(0,0),0, 0,0;
                ++tmpCurConnectIndex;
                deletedDendId.push_back(Vec4d(double(i), currentDendCurve[0](0), currentDendCurve[0](1), currentDendCurve[0](2)));
            }
            newDendConInfo[i] = tmpDendConInfo;
        }
//#ifdef _WIN32
//    clock_t end = clock();
//    printf("%d ms eclipsed in %d break. \n", int(end - beg));
//#else
//    gettimeofday(&end1, 0);
//    double timeuse=1000000*(end1.tv_sec-start1.tv_sec)+end1.tv_usec-start1.tv_usec;
//    timeuse/=1000000;
//    printf("%lf s eclipsed in %d break.\n", timeuse, i);
//#endif
    }
//    for(size_t i = 0; i < newDendConInfo.size(); ++i){
//        printf("%d : %d %d %d %d\n",i, newDendConInfo[i](0), newDendConInfo[i](0,1),
//               newDendConInfo[i](1,0), newDendConInfo[i](1,1) );
//    }
    return true;
}

void BridgeBreaker::GetBoundaryBack(const std::vector<double> &outer_shell, const double threv, std::vector<double> &boundary_back)
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

void BridgeBreaker::GetRayLimit(const Volume<double> &sphere_ray_wet, const double constriction_threv, std::vector<std::vector<double> > &ray_limit)
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

void BridgeBreaker::GetGradientVectorFlowForTrace(const Volume<double> &sphere_ray_wet, Volume<double> &smooth_ray)
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

void BridgeBreaker::SmoothGradientCurvesForTrace(const std::vector<double> &init_one_ray_wet, std::vector<double> &smooth_one_ray_wet)
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

void BridgeBreaker::CalculateOneRayLimit(const std::vector<double> &ray, const double constriction_threv, int &one_ray_limit)
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

void BridgeBreaker::CalcRayBurstRadWet(const VectorVec5d &noHeadTailDendList, const Volume<unsigned short> &origImg,
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

void BridgeBreaker::CalcRayBurstOnePtRadWet(const Vec3d &curvePt, const Volume<unsigned short> &origImg, const Vec3d &preDirection, Vec2d &ptRadWet)
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

void BridgeBreaker::WeighRayValue(const VectorVec3d &rayNode, const Volume<unsigned short> &locOrigImg,
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

void BridgeBreaker::JudgeHeadTailForDendCurve(const MatXd &initHeadRadWet, const MatXd &initTailRadWet,
                                              const Vec2d &connectWet,
                                              int &headTailFlag)
{//matxd(2,4)
    headTailFlag = 0;
    double r0 = 0, r1 = 0, v0 = 0, v1 = 0;
    for(int i = 0; i < 4; ++i){
        r0 += initHeadRadWet(0, i);
        r1 += initTailRadWet(0, i);
        v0 += initHeadRadWet(1, i);
        v1 += initTailRadWet(1, i);
    }
    r0 /= 4.0;
    r1 /= 4.0;
    v0 /= 4.0;
    v1 /= 4.0;
    double v = std::max(std::min(v0, v1), 1.0);
    double F1 = r0 + 0.5 * v0 / v + connectWet(0);
    double F2 = r1 + 0.5 * v1 / v + connectWet(1);
    if(F1 > F2 + 0.5){
        headTailFlag = 1;
    }
    if(F1 + 0.5 < F2){
        headTailFlag = -1;
    }
}

void BridgeBreaker::KnotAnalysis(const VectorVec5d &currentDendCurve, const VectorVec5d &conjDendCurve,
                                 int directionFlag, double angleThrev, int conjHeadTailFlag,
                                 int &knotFlag)
{
    knotFlag = 0;
    Vec3d forwardDirection, backwardDirection, currentHeadDirection;
    Vec3d headCurrentCurvePt;
    Vec3d tmpConjDendCurvePt;
    Vec3d tmpCurrentDendCurvePt;
    int conjCurveLen = conjDendCurve.size();
    if(conjCurveLen == 1){
        knotFlag=0;
        directionFlag=-1;
    }

    if(directionFlag == 0){
        headCurrentCurvePt << currentDendCurve[0](0), currentDendCurve[0](1), currentDendCurve[0](2);
        std::vector<double> distList(conjCurveLen, 0);
        for(int i = 0; i < conjCurveLen; ++i){
            tmpConjDendCurvePt << conjDendCurve[i](0), conjDendCurve[i](1), conjDendCurve[i](2);
            distList[i] = (headCurrentCurvePt - tmpConjDendCurvePt).norm();
        }

        std::vector<double>::iterator minItem = std::min_element(distList.begin(), distList.end());
        int idexx = std::distance(distList.begin(), minItem);
        double tmpvalue1 = ( Vec3d(conjDendCurve[0](0), conjDendCurve[0](1), conjDendCurve[0](2))
                - Vec3d(conjDendCurve[idexx](0), conjDendCurve[idexx](1), conjDendCurve[idexx](2)) ).norm();
        double tmpvalue2 = ( Vec3d(conjDendCurve[0](0), conjDendCurve[0](1), conjDendCurve[0](2))
                - Vec3d(conjDendCurve[idexx](0), conjDendCurve[idexx](1), conjDendCurve[idexx](2)) ).norm();
        double minDistInEndPtAndConDist = std::min<double>(tmpvalue1, tmpvalue2);
        if(minDistInEndPtAndConDist < 5){
            if(0 == conjHeadTailFlag){
                std::vector<int> forwardIdx;
                for(int i = idexx; i <= std::min<int>(idexx+10,conjCurveLen-2); ++i ){
                    forwardIdx.push_back(i);
                }

                std::vector<int> backwardIdx;
                for(int i = idexx; i >= std::max<int>(idexx-10, 1); --i ){
                    backwardIdx.push_back(i);
                }

                //forwardDirection=CalcPrinDirAndDistList(conjDendCurve(1:3,forwardIdx));
                VectorVec3d conjDendCurveSub;
                for(VectorVec3d::size_type k = 0; k < forwardIdx.size(); ++k){
                    tmpConjDendCurvePt << conjDendCurve[forwardIdx[k]](0), conjDendCurve[forwardIdx[k]](1),
                            conjDendCurve[forwardIdx[k]](2);
                    conjDendCurveSub.push_back(tmpConjDendCurvePt);
                }
                CalcPrinDirAndDistList(conjDendCurveSub, forwardDirection);

                conjDendCurveSub.clear();
                for(VectorVec3d::size_type k = 0; k < backwardIdx.size(); ++k){
                    tmpConjDendCurvePt << conjDendCurve[backwardIdx[k]](0), conjDendCurve[backwardIdx[k]](1),
                            conjDendCurve[backwardIdx[k]](2);
                    conjDendCurveSub.push_back(tmpConjDendCurvePt);
                }
                CalcPrinDirAndDistList(conjDendCurveSub, backwardDirection);

                std::vector<int> currentHeadIdx;
                for(int i = 1; i <= std::min<int>(currentDendCurve.size() - 1, 9); ++i ){
                    currentHeadIdx.push_back(i);
                }

                //[currentHeadDirection,aa]=CalcPrinDirAndDistList(currentDendCurve(1:3,currentHeadIdx));
                VectorVec3d currentDendCurveSub;
                for(VectorVec3d::size_type k = 0; k < currentHeadIdx.size(); ++k){
                    tmpCurrentDendCurvePt << currentDendCurve[currentHeadIdx[k]](0), currentDendCurve[currentHeadIdx[k]](1),
                            currentDendCurve[currentHeadIdx[k]](2);
                    currentDendCurveSub.push_back(tmpCurrentDendCurvePt);
                }
                CalcPrinDirAndDistList(currentDendCurveSub, currentHeadDirection);

                double vd1=0, vd2 = 0, vd3 = 0;
                int idMax = std::min<int>(4, int(forwardIdx.size()) - 1);//5-1=4
                for(int k = 0; k <= idMax; ++k){
                    vd1 += conjDendCurve[forwardIdx[k] ](4);
                }
                vd1 /= idMax + 1;

                idMax = std::min<int>(4, int(backwardIdx.size()) - 1);//5-1=4
                for(int k = 0; k <= idMax; ++k){
                    vd2 += conjDendCurve[backwardIdx[k] ](4);
                }
                vd2 /= idMax + 1;

                idMax = std::min<int>(4, int(currentHeadIdx.size()) - 1);//5-1=4
                for(int k = 0; k <= idMax; ++k){
                    vd3 += currentDendCurve[currentHeadIdx[k] ](4);
                }
                vd3 /= idMax + 1;

                double vdMax = vd1 > vd2 ? (vd1 > vd3 ? vd1 : vd3) : (vd2 > vd3 ? vd2 : vd3);
                double vdMin = vd1 < vd2 ? (vd1 < vd3 ? vd1 : vd3) : (vd2 < vd3 ? vd2 : vd3);

                if(std::max( std::abs(currentHeadDirection.dot(backwardDirection)),
                             std::abs(currentHeadDirection.dot(forwardDirection))) < angleThrev
                        && vdMax > 1.35 * vdMin){
                    knotFlag = 1;
                }
            }
            else{
                //forwardIdx=max(idexx-8,2):min(idexx+8,conjCurveLen-1);
                std::vector<int> forwardIdx;
                for(int i = std::max(idexx - 8, 1); i <= std::min<int>(idexx+8,conjCurveLen-2); ++i ){
                    forwardIdx.push_back(i);
                }

                VectorVec3d conjDendCurveSub;
                for(VectorVec3d::size_type k = 0; k < forwardIdx.size(); ++k){
                    tmpConjDendCurvePt << conjDendCurve[forwardIdx[k]](0), conjDendCurve[forwardIdx[k]](1),
                            conjDendCurve[forwardIdx[k]](2);
                    conjDendCurveSub.push_back(tmpConjDendCurvePt);
                }
                CalcPrinDirAndDistList(conjDendCurveSub, forwardDirection);

                forwardDirection *= double(conjHeadTailFlag);

                std::vector<int> currentHeadIdx;
                for(int i = 1; i <= std::min<int>(currentDendCurve.size() - 1, 9); ++i ){//10-1=9
                    currentHeadIdx.push_back(i);
                }
                VectorVec3d currentDendCurveSub;
                for(VectorVec3d::size_type k = 0; k < currentHeadIdx.size(); ++k){
                    tmpCurrentDendCurvePt << currentDendCurve[currentHeadIdx[k]](0), currentDendCurve[currentHeadIdx[k]](1),
                            currentDendCurve[currentHeadIdx[k]](2);
                    currentDendCurveSub.push_back(tmpCurrentDendCurvePt);
                }
                CalcPrinDirAndDistList(currentDendCurveSub, currentHeadDirection);
                if(forwardDirection.dot(currentHeadDirection) < angleThrev){
                    knotFlag = 1;
                }
            }
        }

        if( minDistInEndPtAndConDist>5.0-0.0001){
            std::vector<int> forwardIdx;
            for(int i = idexx; i <= std::min<int>(idexx+15,conjCurveLen-1); ++i ){
                forwardIdx.push_back(i);
            }

            VectorVec3d conjDendCurveSub;
            for(VectorVec3d::size_type k = 0; k < forwardIdx.size(); ++k){
                tmpConjDendCurvePt << conjDendCurve[forwardIdx[k]](0), conjDendCurve[forwardIdx[k]](1),
                        conjDendCurve[forwardIdx[k]](2);
                conjDendCurveSub.push_back(tmpConjDendCurvePt);
            }
            CalcPrinDirAndDistList(conjDendCurveSub, forwardDirection);

            std::vector<int> backwardIdx;
            for(int i = idexx; i >= std::max<int>(idexx-15, 0); --i ){
                backwardIdx.push_back(i);
            }
            conjDendCurveSub.clear();
            for(VectorVec3d::size_type k = 0; k < backwardIdx.size(); ++k){
                tmpConjDendCurvePt << conjDendCurve[backwardIdx[k]](0), conjDendCurve[backwardIdx[k]](1),
                        conjDendCurve[backwardIdx[k]](2);
                conjDendCurveSub.push_back(tmpConjDendCurvePt);
            }
            CalcPrinDirAndDistList(conjDendCurveSub, backwardDirection);

            std::vector<int> currentHeadIdx;
            for(int i = 0; i <= std::min<int>(currentDendCurve.size() - 1,14); ++i ){
                currentHeadIdx.push_back(i);
            }

            VectorVec3d currentDendCurveSub;
            for(VectorVec3d::size_type k = 0; k < currentHeadIdx.size(); ++k){
                tmpCurrentDendCurvePt << currentDendCurve[currentHeadIdx[k]](0), currentDendCurve[currentHeadIdx[k]](1),
                        currentDendCurve[currentHeadIdx[k]](2);
                currentDendCurveSub.push_back(tmpCurrentDendCurvePt);
            }
            CalcPrinDirAndDistList(currentDendCurveSub, currentHeadDirection);
            if(forwardDirection.dot(backwardDirection) < -0.85
                    && std::max<double>( std::abs(currentHeadDirection.dot(backwardDirection)),
                                         std::abs(currentHeadDirection.dot(forwardDirection))) < angleThrev){
                knotFlag=1;
            }
        }
    }
    //2014-3-28
    if(directionFlag == 1){
        headCurrentCurvePt << currentDendCurve.back()(0), currentDendCurve.back()(1), currentDendCurve.back()(2);
        std::vector<double> distList(conjCurveLen,0);
        for(int i = 0; i < conjCurveLen; ++i){
            tmpConjDendCurvePt << conjDendCurve[i](0), conjDendCurve[i](1), conjDendCurve[i](2);
            distList[i] = (headCurrentCurvePt - tmpConjDendCurvePt).norm();
        }

        std::vector<double>::iterator minItem = std::min_element(distList.begin(), distList.end());
        int idexx = std::distance(distList.begin(), minItem);
        double tmpvalue1 = ( Vec3d(conjDendCurve[0](0), conjDendCurve[0](1), conjDendCurve[0](2))
                - Vec3d(conjDendCurve[idexx](0), conjDendCurve[idexx](1), conjDendCurve[idexx](2)) ).norm();
        double tmpvalue2 = ( Vec3d(conjDendCurve[conjCurveLen-1](0), conjDendCurve[conjCurveLen-1](1), conjDendCurve[conjCurveLen-1](2))
                - Vec3d(conjDendCurve[idexx](0), conjDendCurve[idexx](1), conjDendCurve[idexx](2)) ).norm();
        double minDistInEndPtAndConDist = std::min<double>(tmpvalue1, tmpvalue2);
        if(minDistInEndPtAndConDist < 5){
            if(0 == conjHeadTailFlag){
                std::vector<int> forwardIdx;
                for(int i = idexx; i <= std::min<int>(idexx+15,conjCurveLen-1); ++i ){
                    forwardIdx.push_back(i);
                }

                std::vector<int> backwardIdx;
                for(int i = idexx; i >= std::max<int>(idexx-15, 0); --i ){
                    backwardIdx.push_back(i);
                }

                //forwardDirection=CalcPrinDirAndDistList(conjDendCurve(1:3,forwardIdx));
                VectorVec3d conjDendCurveSub;
                for(VectorVec3d::size_type k = 0; k < forwardIdx.size(); ++k){
                    tmpConjDendCurvePt << conjDendCurve[forwardIdx[k]](0), conjDendCurve[forwardIdx[k]](1),
                            conjDendCurve[forwardIdx[k]](2);
                    conjDendCurveSub.push_back(tmpConjDendCurvePt);
                }
                CalcPrinDirAndDistList(conjDendCurveSub, forwardDirection);

                conjDendCurveSub.clear();
                for(VectorVec3d::size_type k = 0; k < backwardIdx.size(); ++k){
                    tmpConjDendCurvePt << conjDendCurve[backwardIdx[k]](0), conjDendCurve[backwardIdx[k]](1),
                            conjDendCurve[backwardIdx[k]](2);
                    conjDendCurveSub.push_back(tmpConjDendCurvePt);
                }
                CalcPrinDirAndDistList(conjDendCurveSub, backwardDirection);

                //currentHeadIdx=size(currentDendCurve,2):-1:max(size(currentDendCurve,2)-15,1);
                std::vector<int> currentHeadIdx;
                for(int i = currentDendCurve.size() - 2; i >= std::max<int>(currentDendCurve.size() - 16, 0); --i ){
                    currentHeadIdx.push_back(i);
                }

                //[currentHeadDirection,aa]=CalcPrinDirAndDistList(currentDendCurve(1:3,currentHeadIdx));
                VectorVec3d currentDendCurveSub;
                for(VectorVec3d::size_type k = 0; k < currentHeadIdx.size(); ++k){
                    tmpCurrentDendCurvePt << currentDendCurve[currentHeadIdx[k]](0), currentDendCurve[currentHeadIdx[k]](1),
                            currentDendCurve[currentHeadIdx[k]](2);
                    currentDendCurveSub.push_back(tmpCurrentDendCurvePt);
                }
                CalcPrinDirAndDistList(currentDendCurveSub, currentHeadDirection);

                double vd1=0, vd2 = 0, vd3 = 0;
                int idMax = std::min<int>(4, int(forwardIdx.size()) - 1);//5-1=4
                for(int k = 0; k <= idMax; ++k){
                    vd1 += conjDendCurve[forwardIdx[k] ](4);
                }
                vd1 /= idMax + 1;

                idMax = std::min<int>(4, int(backwardIdx.size()) - 1);//5-1=4
                for(int k = 0; k <= idMax; ++k){
                    vd2 += conjDendCurve[backwardIdx[k] ](4);
                }
                vd2 /= idMax + 1;

                idMax = std::min<int>(4, int(currentHeadIdx.size()) - 1);//5-1=4
                for(int k = 0; k <= idMax; ++k){
                    vd3 += currentDendCurve[currentHeadIdx[k] ](4);
                }
                vd3 /= idMax + 1;

                double vdMax = vd1 > vd2 ? (vd1 > vd3 ? vd1 : vd3) : (vd2 > vd3 ? vd2 : vd3);
                double vdMin = vd1 < vd2 ? (vd1 < vd3 ? vd1 : vd3) : (vd2 < vd3 ? vd2 : vd3);

                if(std::max( std::abs(currentHeadDirection.dot(backwardDirection)),
                             std::abs(currentHeadDirection.dot(forwardDirection))) < angleThrev
                        && vdMax > 1.5 * vdMin){
                    knotFlag = 1;
                }
            }
            else{
                //forwardIdx=max(idexx-8,2):min(idexx+8,conjCurveLen-1);
                std::vector<int> forwardIdx;
                for(int i = std::max(idexx - 8, 1); i <= std::min<int>(idexx+8,conjCurveLen-2); ++i ){
                    forwardIdx.push_back(i);
                }

                VectorVec3d conjDendCurveSub;
                for(VectorVec3d::size_type k = 0; k < forwardIdx.size(); ++k){
                    tmpConjDendCurvePt << conjDendCurve[forwardIdx[k]](0), conjDendCurve[forwardIdx[k]](1),
                            conjDendCurve[forwardIdx[k]](2);
                    conjDendCurveSub.push_back(tmpConjDendCurvePt);
                }
                CalcPrinDirAndDistList(conjDendCurveSub, forwardDirection);

                forwardDirection *= double(conjHeadTailFlag);

                std::vector<int> currentHeadIdx;
                for(int i = currentDendCurve.size() - 2; i >= std::max<int>(currentDendCurve.size() - 16, 0); --i ){
                    currentHeadIdx.push_back(i);
                }
                VectorVec3d currentDendCurveSub;
                for(VectorVec3d::size_type k = 0; k < currentHeadIdx.size(); ++k){
                    tmpCurrentDendCurvePt << currentDendCurve[currentHeadIdx[k]](0), currentDendCurve[currentHeadIdx[k]](1),
                            currentDendCurve[currentHeadIdx[k]](2);
                    currentDendCurveSub.push_back(tmpCurrentDendCurvePt);
                }
                CalcPrinDirAndDistList(currentDendCurveSub, currentHeadDirection);
                if(forwardDirection.dot(currentHeadDirection) < angleThrev){
                    knotFlag = 1;
                }
            }
        }

        if( minDistInEndPtAndConDist>5.0-0.0001){
            std::vector<int> forwardIdx;
            for(int i = idexx; i <= std::min<int>(idexx+15,conjCurveLen-1); ++i ){
                forwardIdx.push_back(i);
            }

            VectorVec3d conjDendCurveSub;
            for(VectorVec3d::size_type k = 0; k < forwardIdx.size(); ++k){
                tmpConjDendCurvePt << conjDendCurve[forwardIdx[k]](0), conjDendCurve[forwardIdx[k]](1),
                        conjDendCurve[forwardIdx[k]](2);
                conjDendCurveSub.push_back(tmpConjDendCurvePt);
            }
            CalcPrinDirAndDistList(conjDendCurveSub, forwardDirection);

            std::vector<int> backwardIdx;
            for(int i = idexx; i >= std::max<int>(idexx-15, 0); --i ){
                backwardIdx.push_back(i);
            }
            conjDendCurveSub.clear();
            for(VectorVec3d::size_type k = 0; k < backwardIdx.size(); ++k){
                tmpConjDendCurvePt << conjDendCurve[backwardIdx[k]](0), conjDendCurve[backwardIdx[k]](1),
                        conjDendCurve[backwardIdx[k]](2);
                conjDendCurveSub.push_back(tmpConjDendCurvePt);
            }
            CalcPrinDirAndDistList(conjDendCurveSub, backwardDirection);

            std::vector<int> currentHeadIdx;
            for(int i = currentDendCurve.size() - 1; i >= std::max<int>(currentDendCurve.size() - 16,0); --i ){
                currentHeadIdx.push_back(i);
            }

            VectorVec3d currentDendCurveSub;
            for(VectorVec3d::size_type k = 0; k < currentHeadIdx.size(); ++k){
                tmpCurrentDendCurvePt << currentDendCurve[currentHeadIdx[k]](0), currentDendCurve[currentHeadIdx[k]](1),
                        currentDendCurve[currentHeadIdx[k]](2);
                currentDendCurveSub.push_back(tmpCurrentDendCurvePt);
            }
            CalcPrinDirAndDistList(currentDendCurveSub, currentHeadDirection);
            if(forwardDirection.dot(backwardDirection) < -0.85
                    && std::max<double>( std::abs(currentHeadDirection.dot(backwardDirection)),
                                         std::abs(currentHeadDirection.dot(forwardDirection))) < angleThrev){
                knotFlag=1;
            }
        }
    }
}

void BridgeBreaker::CalcConnectKnotLocalDirection(const VectorVec5d &conjDendCurve, const Vec5d &currentDendPt,
                                                  int headTailFlag, Vec3d &conjDendCurveVector)
{
    int nxx = conjDendCurve.size();
    //minDist=norm(conjDendCurve(1:3,1)-currentDendPt(1:3))+1;
    double minDist = ( Vec3d(conjDendCurve[0](0), conjDendCurve[0](1), conjDendCurve[0](2))
            - Vec3d(currentDendPt(0), currentDendPt(1), currentDendPt(2)) ).norm() + 1.0;
    int idex = 0;
    conjDendCurveVector.setZero();
    double currentDist;
    Vec3d tmpConjDendCurvePt, tmpCurrentDendPt;
    for(int i = 0; i < nxx; ++i){
        tmpConjDendCurvePt << conjDendCurve[i](0), conjDendCurve[i](1), conjDendCurve[i](2);
        tmpCurrentDendPt << currentDendPt(0), currentDendPt(1), currentDendPt(2);
        currentDist = (tmpConjDendCurvePt - tmpCurrentDendPt).norm();
        if(currentDist < minDist){
            minDist = currentDist;
            idex = i;
        }
    }
    std::vector<int> backwardId;
    //warning here!
    for(int i = std::max<int>(idex - 6, 1); i <= idex; ++i ){
        backwardId.push_back(i);
    }
    std::vector<int> forwardId;
    for(int i = idex + 1; i <= std::min<int>(idex + 6, nxx - 1); ++i ){
        forwardId.push_back(i);
    }
    int mmk = 0;
    if(std::min<size_t>(backwardId.size(), forwardId.size()) < 3){
        mmk = 1;
    }

    if(std::min<int>( int(forwardId.size()), int(backwardId.size()) ) > 3){
        Vec3d backwardDirection;
        backwardDirection.setZero();
        Vec3d tmpConjDendCurvePt1;
        for(VectorVec3d::size_type i = 1; i < backwardId.size(); ++i){
            tmpConjDendCurvePt << conjDendCurve[backwardId[i] ](0), conjDendCurve[backwardId[i] ](1),
                    conjDendCurve[backwardId[i] ](2);
            tmpConjDendCurvePt1 << conjDendCurve[backwardId[0] ](0), conjDendCurve[backwardId[0] ](1),
                    conjDendCurve[backwardId[0] ](2);
            backwardDirection += (tmpConjDendCurvePt - tmpConjDendCurvePt1).normalized();
        }
        backwardDirection.normalize();

        Vec3d forwardDirection;
        forwardDirection.setZero();
        tmpConjDendCurvePt1.setZero();
        for(VectorVec3d::size_type i = 1; i < forwardId.size(); ++i){
            tmpConjDendCurvePt << conjDendCurve[forwardId[i] ](0), conjDendCurve[forwardId[i] ](1),
                    conjDendCurve[forwardId[i] ](2);
            tmpConjDendCurvePt1 << conjDendCurve[forwardId[0] ](0), conjDendCurve[forwardId[0] ](1),
                    conjDendCurve[forwardId[0] ](2);
            forwardDirection += (tmpConjDendCurvePt - tmpConjDendCurvePt1).normalized();
        }
        forwardDirection.normalize();
        //if abs(forwardDirection'*backwardDirection)>0.85
        if(std::abs(forwardDirection.dot(backwardDirection) ) > 0.85 ){
            mmk = 1;
        }
    }

    if(mmk == 1){
        std::vector<int> allID;
        allID.resize(forwardId.size() + backwardId.size());
        std::copy(backwardId.begin(), backwardId.end(), allID.begin());
        std::copy(forwardId.begin(), forwardId.end(), allID.begin() + backwardId.size());
        int allIDLen = allID.size();
        conjDendCurveVector.setZero();
        Vec3d tmpConjDendCurvePt1;
        tmpConjDendCurvePt1.setZero();
        for(int i = 1; i < allIDLen; ++i){
            tmpConjDendCurvePt << conjDendCurve[allID[i]](0), conjDendCurve[allID[i]](1), conjDendCurve[allID[i]](2);
            tmpConjDendCurvePt1 << conjDendCurve[allID[0]](0), conjDendCurve[allID[0]](1), conjDendCurve[allID[0]](2);
            conjDendCurveVector += (tmpConjDendCurvePt - tmpConjDendCurvePt1).normalized();
        }
        conjDendCurveVector.normalize();
    }

    conjDendCurveVector *= double(headTailFlag);
}

//void BridgeBreaker::RayBurstSampling(const Volume<double> &Sphere_XX, const double three_vs,
//                                   std::vector<std::vector<double> > &Uzz)
//{
//    int nx = Sphere_XX.x();
//    int ny = Sphere_XX.y();
//    int nz = Sphere_XX.z();
//    std::vector<double> LLs;
//    std::vector<double> tmpUzz;
//    LLs.clear();
//    int arra(0);

//    for (int i = 0; i < ny; ++i){
//        tmpUzz.clear();
//        for (int j = 0; j < nz; ++j){
//            for (int ij = 0; ij < nx; ++ij){
//                LLs.push_back(Sphere_XX(ij, i, j));
//            }
//            RayBurstSampling1(LLs, three_vs, arra);
//            tmpUzz.push_back(arra);
//        }
//        Uzz.push_back(tmpUzz);
//    }
//}


//void BridgeBreaker::RayBurstSampling1(const std::vector<double> &LLs, const double three_vs, int &arra)
//{
//    int nx = LLs.size();
//    arra = 0;

//    for (int i = 0; i < nx; ++i){
//        if (LLs[i] < three_vs){
//            break;
//        }
//        else{
//            ++arra;
//        }
//    }
//}

void BridgeBreaker::Principald(const VectorVec3d &dataL, Vec3d &x1)
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

//void BridgeBreaker::WeighRayValue(const VectorVec3d &dataL1, const Volume<unsigned char> &L_XX3, std::vector<double> &aa_data)
//{
//    typedef double mydist;
//    size_t nxss = dataL1.size();//
//    aa_data.clear();

//    int nx = L_XX3.x();
//    int ny = L_XX3.y();
//    int nz = L_XX3.z();

//    //坐标
//    typedef double spheredist;
//    spheredist x,y,z;

//    spheredist dd, ww;//, w1;

//    for (size_t i = 0; i < nxss; ++i)
//    {
//        x = dataL1[i](0);
//        y = dataL1[i](1);
//        z = dataL1[i](2);

//        dd = ww = 0.0;

//        ContourUtil::CalculateSphereOneNode(L_XX3, 0.05, x, y, z, dd, ww);

//        aa_data.push_back(dd / (ww + 0.0001));
//    }
//}

void BridgeBreaker::CalcPrinDirAndDistList(const VectorVec3d &ptLine, Vec3d &mainDirection)
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

void BridgeBreaker::CalcOrthoBasis(const Vec3d &vec1, Vec3d &vec2, Vec3d &vec3)
{
    vec2 = vec1;
    vec3.setZero();
    std::vector<std::pair<int, double> > tmp;
    tmp.push_back(std::pair<int, double>(0, std::abs(vec1(0))));
    tmp.push_back(std::pair<int, double>(1, std::abs(vec1(1))));
    tmp.push_back(std::pair<int, double>(2, std::abs(vec1(2))));
    /*[idxv,idexx]=sort(abs(vec1))*/

    //std::sort(tmp.begin(), tmp.end(),[](const std::pair<int, double>& lhs, const std::pair<int, double>& rhs){
    //    return lhs.second < rhs.second;
    //});
    std::sort(tmp.begin(), tmp.end(), LessPair2());
    double cdd = (tmp[0].second * tmp[0].second + tmp[1].second * tmp[1].second) / tmp[2].second;
    vec2(tmp[2].first) = - (double)TraceUtil::sign(vec1[tmp[2].first]) * cdd;

    if (std::abs(cdd - 0.0) > EXPO)
        vec2.normalize();//cdd != 0.0
    else vec2[tmp[1].first] = 1.0;

    /*第三主成分*/
    vec3(tmp[0].first) = -1.0;
    Mat2d denMat;
    denMat << vec1(tmp[1].first), vec1(tmp[2].first), vec2(tmp[1].first), vec2(tmp[2].first);

    //Matrix2f num1;
    Mat2d num1;
    num1 << vec1(tmp[0].first), vec1(tmp[2].first), vec2(tmp[0].first), vec2(tmp[2].first);

    //Matrix2f num2;
    Mat2d num2;
    num2 << vec1(tmp[1].first), vec1(tmp[0].first), vec2(tmp[1].first), vec2(tmp[0].first);

    double den = denMat.determinant();
    vec3(tmp[1].first) = num1.determinant() / den;
    vec3(tmp[2].first) = num2.determinant() / den;
    vec3.normalize();
}

