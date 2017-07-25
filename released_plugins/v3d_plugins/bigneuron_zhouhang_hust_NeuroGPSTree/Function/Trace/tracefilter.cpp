#include "tracefilter.h"
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
#include <iostream>

#ifdef _WIN32
#include <iterator>
#include <ctime>
#else
#include <sys/time.h>
#endif

#define M_E        2.71828182845904523536
#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923

const double EXPO = 0.000001;

TraceFilter::TraceFilter()
{
    identifyName = std::string("TraceFilter");
#ifdef _WIN32
	m_Source = std::tr1::shared_ptr<TreeCurve>(new TreeCurve(this));
	rawDendConInfoPointer = std::tr1::shared_ptr<TreeConnect>(new TreeConnect);
#else
	m_Source = std::shared_ptr<TreeCurve>(new TreeCurve(this));
	rawDendConInfoPointer = std::shared_ptr<TreeConnect>(new TreeConnect);
#endif
	fillThrev = 120.0;
	endThrev = 150.0;
	//threValue = 1020;
}

TraceFilter::~TraceFilter()
{

}

bool TraceFilter::Update()
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
	if(!m_Soma) m_Soma = std::tr1::shared_ptr<INeuronDataObject>(new Soma);
	origImgPointer =
		std::tr1::dynamic_pointer_cast<const Volume<unsigned short> >(m_Input);
	backImgPointer =
		std::tr1::dynamic_pointer_cast<const Volume<unsigned short> >(m_Back);
	binImgPointer =
		std::tr1::dynamic_pointer_cast<const Volume<NGCHAR> >(m_Bin);
	std::tr1::shared_ptr<const Soma > tmpSoma =
		std::tr1::dynamic_pointer_cast<const Soma>(m_Soma);
	std::tr1::shared_ptr<TreeCurve > tmpTree =
		std::tr1::dynamic_pointer_cast<TreeCurve>(m_Source);
#else
	if(!m_Soma) m_Soma = std::shared_ptr<INeuronDataObject>(new Soma);
	origImgPointer =
		std::dynamic_pointer_cast<const Volume<unsigned short> >(m_Input);
	backImgPointer =
		std::dynamic_pointer_cast<const Volume<unsigned short> >(m_Back);
	binImgPointer =
		std::dynamic_pointer_cast<const Volume<NGCHAR> >(m_Bin);
	std::shared_ptr<const Soma > tmpSoma =
		std::dynamic_pointer_cast<const Soma>(m_Soma);
	std::shared_ptr<TreeCurve > tmpTree =
		std::dynamic_pointer_cast<TreeCurve>(m_Source);
#endif

    if(!origImgPointer || !backImgPointer || !binImgPointer || !tmpSoma) {
        printf("error backnoise image!\n");
        printf("error occured in %s\n", identifyName.c_str());
        return false;
    }

    //complete initialize
    int vx = origImgPointer->x();
    int vy = origImgPointer->y();
    int vz = origImgPointer->z();

    Volume<int> indexImg;
    indexImg.SetSize(vx, vy, vz);//short just support 2^16 = 65536
    VectorVec3d somaList;
    Vec3d somaItem;
    for (int i = 0; i < (int)tmpSoma->size(); ++i){
        somaItem << tmpSoma->GetCell(i).x, tmpSoma->GetCell(i).y, tmpSoma->GetCell(i).z ;
        somaList.push_back(somaItem);
    }

    //std::vector<std::vector<int> > MM;
    //rawDendConInfoPointer = std::shared_ptr<TreeConnect>(new TreeConnect);
#ifdef _WIN32
	rawDendConInfoPointer = std::tr1::shared_ptr<TreeConnect>(new TreeConnect);
	std::tr1::shared_ptr<TreeConnect> tmpRawDendConInfoPointer = std::tr1::dynamic_pointer_cast<TreeConnect>(rawDendConInfoPointer);
#else
	rawDendConInfoPointer = std::shared_ptr<TreeConnect>(new TreeConnect);
	std::shared_ptr<TreeConnect> tmpRawDendConInfoPointer = std::dynamic_pointer_cast<TreeConnect>(rawDendConInfoPointer);
#endif
    std::vector<VectorVec5d> rawDendList;
    VectorMat2i rawDendConInfo;
#ifdef _WIN32
    clock_t beg = clock();
#else
    timeval start1, end1;
    gettimeofday(&start1, 0);
#endif
    TraceCurvesAndConInfo(indexImg, *origImgPointer, *binImgPointer, *backImgPointer, somaList, rawDendList, rawDendConInfo);
    tmpTree->Swap(rawDendList);
    tmpRawDendConInfoPointer->Swap(rawDendConInfo);
#ifdef _WIN32
    clock_t end = clock();
    printf("%d ms eclipsed in raw trace. \n", int(end - beg));
#else
    gettimeofday(&end1, 0);
    double timeuse=1000000*(end1.tv_sec-start1.tv_sec)+end1.tv_usec-start1.tv_usec;
    timeuse/=1000000;
    printf("%lf s eclipsed in raw trace\n", timeuse);
#endif
    return true;
}

ConstDataPointer TraceFilter::GetOutput()
{
    if(!m_Source)
#ifdef _WIN32
		m_Source = std::tr1::shared_ptr<TreeConnect>(new TreeConnect(this));
#else
		m_Source = std::shared_ptr<TreeConnect>(new TreeConnect(this));
#endif
    return m_Source;
}

DataPointer TraceFilter::GetConnect()
{
    return rawDendConInfoPointer;
}

DataPointer TraceFilter::ReleaseData()
{
    m_Source->ReleaseProcessObject();
    DataPointer tData(m_Source);
    m_Source.reset();
    return tData;
}

void TraceFilter::SetInputBack(ConstDataPointer p)
{
    m_Back = p;
}

void TraceFilter::SetInputBin(ConstDataPointer p)
{
    m_Bin = p;
}

void TraceFilter::SetSoma(ConstDataPointer p)
{
    m_Soma = p;
}

DataPointer &TraceFilter::GetDendConInfo()
{
    return rawDendConInfoPointer;
}

void TraceFilter::TraceCurvesAndConInfo(Volume<int> &resultIndexImg, const Volume<unsigned short> &origImg,
                                        const Volume<NGCHAR> &binImg,
                                        const Volume<unsigned short> &backImg,
                                        const VectorVec3d &somaList,
                                        std::vector<VectorVec5d > &rawDendList, VectorMat2i &rawDendConInfo)
{
    //finally
    std::vector<VectorVec5d > resultDendCurves;
    VectorMat2i resultDendConInfo;
    //all soma shape
    VectorMatXd allRayLen;
	ReconstructSomaShapeForTrace(somaList,origImg, resultIndexImg,allRayLen);
    //printf("ReconstructSomaShapeForTrace.\n");
    //all seed not inside soma
    VectorVec3d tmpTraceSeed;
    SelectSeedForTrace(binImg, origImg, resultIndexImg, tmpTraceSeed);
	//tmpTraceSeed.push_back(Vec3d(157,159,110));//2015-11-20
    //printf("SelectSeedForTrace.\n");
    VectorVec3d::size_type somaNum = somaList.size();
    VectorVec3d traceSeed(somaList);
    traceSeed.resize(somaNum + tmpTraceSeed.size());
    std::copy(tmpTraceSeed.begin(), tmpTraceSeed.end(), traceSeed.begin() + somaNum);
    //size_t somaNum= somaList.size();

    std::vector<VectorVec5d > tmpDendCurves;
    VectorMat2i tmpDendConInfo;
    //2014-4-19 : new
    VectorMat3d allThickDendInitDir(somaNum);
    VectorVec3d allThickDendInitPt(somaNum);
//#pragma omp parallel for
    for(int ii = 0; ii < int(somaNum); ++ii){
        const Vec3d& initialP = traceSeed[ii];
        Vec3d mainDir1,mainDir2,mainDir3,thickDendInitPt;
        FindThickDendDirFromSoma(allRayLen[ii], origImg, backImg, initialP,
                                 resultIndexImg, -(int(ii)+1),
                                 mainDir1,mainDir2,mainDir3,thickDendInitPt);

        if(mainDir1.array().abs().sum() > 0.0){
            Mat3d tmpMat;
            tmpMat.col(0) = mainDir1;
            tmpMat.col(1) = mainDir2;
            tmpMat.col(2) = mainDir3;
            allThickDendInitDir[ii]=(tmpMat);
            allThickDendInitPt[ii]=(thickDendInitPt);
        } else{
            allThickDendInitDir[ii].setZero();
            allThickDendInitPt[ii].setZero();
        }
    }

    //printf("FindThickDendDirFromSoma.\n");

    std::vector<int> largeAngleFlagList;
    if(!somaList.empty()){
        ForceModifyDirection(allThickDendInitDir, largeAngleFlagList);
    }

    for (VectorVec3d::size_type i = 0; i < somaNum; ++i){//TODO
        printf("                \r");
        printf("cell tracing : %d of %d\r", int(i) + 1, int(somaNum));
        Mat2i resultCurConInfo;
        resultCurConInfo.setZero();
        VectorVec5d resultCurve;
        //
        Vec3d initialPoint;//(traceSeed[i](0), traceSeed[i](1), traceSeed[i](2));
        if(largeAngleFlagList[i] == 0){
            initialPoint = allThickDendInitPt[i];
        } else{
            initialPoint = traceSeed[i];
        }
        //
        Mat3d& somaInitDir = allThickDendInitDir[i];
        if(somaInitDir.col(0).array().abs().sum() > 0.0){
            Vec3d somaInitMainDir = somaInitDir.col(0);
            VectorVec5d thickCurve;
            RayburstShapeTrack(initialPoint, origImg, somaInitMainDir,12,
                               thickCurve);

            if(!thickCurve.empty()){
                TraceCurvesFromSoma(origImg, backImg, resultIndexImg,thickCurve,  i+1,somaInitDir,
                                    somaList,
                                    resultCurve, resultCurConInfo);//注意序号！
            } else{
                resultCurve.clear();
            }
        }else{
            resultCurve.clear();
        }

        tmpDendCurves.push_back(resultCurve);
        if (!resultCurve.empty())
            tmpDendConInfo.push_back(resultCurConInfo);
        else{
            resultCurConInfo.setZero();
            tmpDendConInfo.push_back(resultCurConInfo);
        }

        //--------------test-----------
//        if(i == 2){
//        FILE *fp = fopen("/home/zhouhang/TIFF/compareCurve.txt","w");
//        for(size_t kk = 0; kk < resultCurve.size();++kk){
//            fprintf(fp, "%lf %lf %lf\n",resultCurve[kk](0),resultCurve[kk](1),resultCurve[kk](2));
//        }
//        fclose(fp);}
    }

    //TODO:2014-4-19 no comment
    printf("\n");
    int seedSeries = int(0.7 * traceSeed.size() +0.5);
    int uppersz = somaNum + seedSeries;//2015-11-20
    for (int i = somaNum; i < uppersz; ++i){
        printf("                \r");
        printf("seed tracing : %d of %d\r", int(i - somaNum + 1), seedSeries);
        Vec3d initialPoint(traceSeed[i](0), traceSeed[i](1), traceSeed[i](2));
        //int** dataPtr_ = resultIndexImg.GetPointer();
        if ( 0 == resultIndexImg(int(traceSeed[i](0)), int(traceSeed[i](1) ), int(traceSeed[i](2) )) ){
            Mat2i resultCurConInfo;
            resultCurConInfo.setZero();
            VectorVec5d resultCurve;
            TraceCurvesFromSeed(origImg, backImg, initialPoint, resultIndexImg, somaList, i + 1, resultCurve, resultCurConInfo);

            tmpDendCurves.push_back(resultCurve);
            if (!resultCurve.empty())  tmpDendConInfo.push_back(resultCurConInfo);
            else{
                resultCurConInfo.setZero();
                tmpDendConInfo.push_back(resultCurConInfo);
            }
        }//
        else{
            //though it is not needed ,but i am dan teng.
            VectorVec5d dataLLs;
            dataLLs.clear();
            tmpDendCurves.push_back(dataLLs);
            Mat2i resultCurConInfo;
            resultCurConInfo.setZero();
            tmpDendConInfo.push_back(resultCurConInfo);
        }
    }
    printf("\n");
    //test---------------------
//    FILE *fp = fopen("/home/zhouhang/TIFF/compareCNoClearDendInfo.txt","w");
//    for(size_t kk = 0; kk < tmpDendConInfo.size();++kk){
//        fprintf(fp, "%d %d\n",tmpDendConInfo[kk](0,0),tmpDendConInfo[kk](0,1));
//    }
//    fclose(fp);

    ClearShortCurvesAndInvalidConnection(tmpDendCurves, tmpDendConInfo, resultDendCurves, resultDendConInfo/*, mmc12*/);

    rawDendList.clear();
    rawDendConInfo.clear();
    AddCollideConnectNode(resultDendCurves, resultDendConInfo, somaList, rawDendList);
	std::swap(rawDendConInfo, resultDendConInfo);
	//2015-8-13
	if (origImg.XResolution() * origImg.YResolution() * origImg.ZResolution() > 1.5) {
	}else{
		std::vector<VectorVec5d> dendCurves;
		dendCurves.swap(rawDendList);
		VectorMat2i dendConInfo;
		dendConInfo.swap(rawDendConInfo);
		MyCrossDetection(dendCurves, dendConInfo, rawDendList, rawDendConInfo);
	}
	
    //test---------------------
	/*FILE *fp = fopen("D:/dd222.txt","w");
    for(size_t kk = 0; kk < rawDendConInfo.size();++kk){
        fprintf(fp, "%d %d\n",rawDendConInfo[kk](0,0),rawDendConInfo[kk](0,1));
    }
    fclose(fp);
	fp = fopen("D:/dd111.txt","w");
	int idx = 0;
	for(size_t kk = 0; kk < rawDendList.size();++kk){
		for (size_t ii = 0; ii < rawDendList[kk].size(); ++ii) {
			++idx;
			if (ii == 0) {
				fprintf(fp, "%d 1 %lf %lf %lf 1.0 %d\n", idx, rawDendList[kk][ii](0), rawDendList[kk][ii](1), rawDendList[kk][ii](2), -1);
			}else{
				fprintf(fp, "%d 1 %lf %lf %lf 1.0 %d\n", idx, rawDendList[kk][ii](0), rawDendList[kk][ii](1), rawDendList[kk][ii](2), idx - 1);
			}
			
		}
		
	}
	fclose(fp);
	system("pause");*/
}

void TraceFilter::SelectSeedForTrace(const Volume<NGCHAR> &binImg, const Volume<unsigned short> &origImg,
                                     const Volume<int>& indexImg,
                                     VectorVec3d &traceSeed)
{
    traceSeed.clear();

    VectorVec4d tmpTraceSeed;
    Vec4d tmpvec4;

    int nxx = binImg.x();
    int nyy = binImg.y();
    int nzz = binImg.z();
    //more memory occupied
    Volume<unsigned short> origImgCopy;
    origImgCopy.QuickCopy(origImg);
	int range = 0;
	if (origImg.XResolution() * origImg.YResolution() * origImg.ZResolution() > 1.5) {
		range = 5;
	}else{
		range = 7;
	}

	for (int i = 0; i < nxx; ++i)
	{
		for (int j = 0; j < nyy; ++j)
		{
			for (int ij = 0; ij < nzz; ++ij)
			{
				if (binImg(i,j,ij) > 0 && indexImg(i,j,ij) >=0
					&& IsAreaMaxValue<unsigned short>(origImgCopy, std::max<int>(i - range, 0), std::min<int>(i + range, nxx - 1),
					std::max<int>(j - range, 0), std::min<int>(j + range, nyy - 1),
					std::max<int>(ij - range, 0), std::min<int>(ij + range, nzz - 1),
					origImgCopy(i, j, ij) ) )
				{
					tmpvec4<< i, j, ij, origImgCopy(i, j, ij);
					tmpTraceSeed.push_back(tmpvec4);
					//printf("%d %d %d\n", i+1,j+1,ij+1);
					SetAreaValue<unsigned short>(origImgCopy, std::max<int>(i - range, 0), std::min<int>(i + range, nxx - 1),
						std::max<int>(j - range, 0), std::min<int>(j + range, nyy - 1),
						std::max<int>(ij - range, 0), std::min<int>(ij + range, nzz - 1),
						0 );
				}

			}
		}
	}//for

#ifdef __linux
    std::sort(tmpTraceSeed.begin(), tmpTraceSeed.end(), [](const Vec4d& lhs, const Vec4d& rhs){//Node_v_x_y_z_DES
        if(lhs(3) != rhs(3)) return lhs(3) > rhs(3);
        else if(lhs(0) != rhs(0)) return lhs(0) < rhs(0);
        else if(lhs(1) != rhs(1)) return lhs(1) < rhs(1);
        return lhs(2) < rhs(2);
    });
#else
    std::sort(tmpTraceSeed.begin(), tmpTraceSeed.end(), Vec4d_3_great_012less());
#endif

    size_t numLimit = std::min<size_t>(tmpTraceSeed.size(), 10000);
	traceSeed.resize(numLimit);
    for (VectorVec4d::size_type i = 0; i < numLimit; ++i )
        traceSeed[i] = (Vec3d(tmpTraceSeed[i](0), tmpTraceSeed[i](1), tmpTraceSeed[i](2) ));
}

void TraceFilter::CalcNeighborSignal(const Volume<unsigned short> &origImg,
                                            const Vec3d &curSeedNode, const Vec3d &initVec, const double threv,
                                            VectorVec3d &neighborPtSet, std::vector<double> &neighborWet,
                                            Vec3d &firDir, Vec3d &secDir)
{
    neighborPtSet.clear();
    neighborWet.clear();
    firDir.setZero();
    secDir.setZero();
    int vx = origImg.x();
    int vy = origImg.y();
    int vz = origImg.z();

    /*初始化*/
	//2015-8-13
	double xExtractRegion, yExtractRegion, zExtractRegion;
	xExtractRegion = yExtractRegion = 7.0;
	zExtractRegion = 6.0;
	
    int xMin = std::max(Round(curSeedNode(0) - xExtractRegion ), 0);
    int xMax = std::min(Round(curSeedNode(0) + xExtractRegion), vx - 1);
    int yMin = std::max(Round(curSeedNode(1) - yExtractRegion ), 0);
    int yMax = std::min(Round(curSeedNode(1) + yExtractRegion ), vy - 1);
    int zMin = std::max(Round(curSeedNode(2) - zExtractRegion ), 0);
    int zMax = std::min(Round(curSeedNode(2) + zExtractRegion ), vz - 1);

    Vec3d curCenter(curSeedNode(0) - (double)xMin,
        curSeedNode(1) - (double)yMin, curSeedNode(2) - (double)zMin);//ML1,中心点
    /*提取区域*/
    Volume<unsigned short> locOrigImg;
    //Change : 2014-3-19-10-16
    locOrigImg.SetSize(xMax - xMin + 1, yMax - yMin + 1, zMax - zMin + 1);
    //const Volume<unsigned short> &pptr = origImg;//局部区域
    for (int i = xMin; i <= xMax; ++i){
        for (int j = yMin; j <= yMax; ++j){
            for (int ij = zMin; ij <= zMax; ++ij){
                locOrigImg(i - xMin, j - yMin, ij - zMin) = origImg(i, j, ij);
            }
        }
    }
    /*局部大小*/
    int sx = xMax - xMin + 1;
    int sy = yMax - yMin + 1;
    int sz = zMax - zMin + 1;
    //Volumn subVol(xMax - xMin + 1, yMax - yMin + 1, vol.z);//nx1,ny1,nz1 zMax - zMin + 1
    /*最小边长*/
    int minLen = (sx) < (sy) ?
        ((sx) < (sz) ? (sx) : (sz)) :
        ((sy) < (sz) ? (sy) : (sz));

    double ii(0.0);
    double jj(0.0);
    double angleDiff(0.0);

    double angleThrev1=-0.95;
    double angleThrev2= 0.95;
	
	//2015-8-13
	if (minLen > 3 && vz > 3){
		/*AAj = zeros(4,216)*/
		VectorVec4d backwardNodeSet;//AAj
		for (int i = 0; i < 36; ++i){
			for ( int j = 0; j < 18; ++j){
				ii = (double)i * M_PI / 18.0;
				jj = (double)j * M_PI / 18.0;
				Vec3d polarPosition((std::sin(jj)) * (std::cos(ii)), (std::sin(ii)) * (std::sin(jj)), (std::cos(jj)));
				angleDiff = polarPosition.dot(initVec);/*jd*/
				if (angleDiff < angleThrev1) //if jd < -0.85
				{
					VectorVec3d rayNode;
					rayNode.push_back(curCenter + polarPosition);
					rayNode.push_back(curCenter + 2 * polarPosition);
					rayNode.push_back(curCenter + 3 * polarPosition);
					rayNode.push_back(curCenter + 4 * polarPosition);
					rayNode.push_back(curCenter + 5 * polarPosition);
					std::vector<double> rayNodeWet;
					WeighRayValue(rayNode, locOrigImg, rayNodeWet);
					double meanRayNodeWet = std::accumulate(rayNodeWet.begin(), rayNodeWet.begin() + 3, 0.0);
					meanRayNodeWet /= 3.0;//akmmv.size();
					backwardNodeSet.push_back(Vec4d(polarPosition(0), polarPosition(1), polarPosition(2), meanRayNodeWet));
				}//if
			}
		}//for
		/*异常处理*/
		if (backwardNodeSet.size() < 2) firDir =  - initVec;
		else{
#ifdef __linux
            VectorVec4d::iterator maxItem = std::max_element(backwardNodeSet.begin(), backwardNodeSet.end(),
                [](const Vec4d& lhs, const Vec4d& rhs){
                    return lhs(3) < rhs(3);
            });//Node_4TH_MAX

#else
            VectorVec4d::iterator maxItem = std::max_element(backwardNodeSet.begin(), backwardNodeSet.end(),
                Vec4d_3th_less());//Node_4TH_MAX
#endif
			Vec3d maxVec((*maxItem)(0), (*maxItem)(1), (*maxItem)(2));//xss
			firDir = maxVec.normalized();//x10 =
		}
	}
	else firDir = - initVec;

	if (minLen > 3 && vz > 3){
		/*AAj2 = zeros(4,216)*/
		VectorVec4d forwardNodeSet;//AAj2
		for (int i = 0; i < 36; ++i){
			for ( int j = 0; j < 18; ++j){
				ii = (double)i * M_PI / 18.0;
				jj = (double)j * M_PI / 18.0;
				Vec3d ds1((std::sin(jj)) * (std::cos(ii)), (std::sin(ii)) * (std::sin(jj)) , (std::cos(jj)));//ds1
				angleDiff = ds1.dot(initVec);/*jd*/
				if (angleDiff > angleThrev2){ //if jd > -0.85
					VectorVec3d rayNode;
					rayNode.push_back(curCenter + ds1);
					rayNode.push_back(curCenter + 2 * ds1);
					rayNode.push_back(curCenter + 3 * ds1);
					rayNode.push_back(curCenter + 4 * ds1);
					rayNode.push_back(curCenter + 5 * ds1);
					rayNode.push_back(curCenter + 6 * ds1);
					std::vector<double> rayNodeWet;
					WeighRayValue(rayNode, locOrigImg, rayNodeWet);
					double meanRayNodeWet = accumulate(rayNodeWet.begin(), rayNodeWet.begin()+3, 0.0);
					meanRayNodeWet /= 3.0;//akmmv.size();
					forwardNodeSet.push_back(Vec4d(ds1[0], ds1[1], ds1[2], meanRayNodeWet));
				}
			}
		}//for
		/*异常处理*/
		if (forwardNodeSet.size() < 2) secDir = initVec;
		else{
#ifdef __linux
            VectorVec4d::iterator maxItem = std::max_element(forwardNodeSet.begin(), forwardNodeSet.end(),
                [](const Vec4d& lhs, const Vec4d& rhs){
                    return lhs(3) < rhs(3);
            });//Node_4TH_MAX
#else
            VectorVec4d::iterator maxItem = std::max_element(forwardNodeSet.begin(), forwardNodeSet.end(),
                Vec4d_3th_less());//Node_4TH_MAX

#endif
			Vec3d maxVec((*maxItem).x(), (*maxItem).y(), (*maxItem).z());//xss
			secDir = maxVec.normalized();
		}
	}
	else secDir = initVec;

	/*dataSS = zeros(4,8*125);//存放射线区域信息*/
	VectorVec4d rayArea;//dataSS,dataS
	for (int i = 0; i < 3; ++i){//2014-4-21
		//Vec3d data10(curSeedNode(0), curSeedNode(1), curSeedNode(2));
		//Vec3d ray = i * firDir + data10; //data10 + i * x10
		Vec3d rayNode = i * firDir + curSeedNode;
		int minX = std::max(std::min(Round(rayNode(0) - 2.0), vx - 1), 0);
		int maxX = std::min(std::max(Round(rayNode[0] + 2.0), 0), vx - 1);
		int minY = std::max(std::min(Round(rayNode[1] - 2.0), vy - 1), 0);
		int maxY = std::min(std::max(Round(rayNode[1] + 2.0), 0), vy - 1);
		int minZ = std::max(std::min(Round(rayNode[2] - 2.0), vz - 1), 0);
		int maxZ = std::min(std::max(Round(rayNode[2] + 2.0), 0), vz - 1);
		//Volumn localVol(maxX - minX + 1, maxY - minY + 1, maxZ - minZ + 1);
		int lx = maxX - minX + 1;
		int ly = maxY - minY + 1;
		int lz = maxZ - minZ + 1;
		/*存储射线区域*/
		Vec4d tmpNode;
		for (int ix = 0; ix < lx; ++ix){
			for (int iy = 0; iy < ly; ++iy){
				for (int iz = 0; iz < lz; ++iz){
					int newX = minX + ix;
					int newY = minY + iy;
					int newZ = minZ + iz;
					tmpNode << newX, newY , newZ , std::max(double(origImg(newX, newY, newZ)) - threv, 0.0) ;
					rayArea.push_back(tmpNode);
				}
			}
		}
	}
	/*第二条射线区域*/
	for (int i = 1; i < 3; ++i){
		//Vec3d data10(curSeedNode(0), curSeedNode(1), curSeedNode(2));
		//Vec3d ray = i * secDir + data10; //data10 + i * x11
		Vec3d rayNode = i * secDir + curSeedNode;
		int minX = std::max(std::min(int(rayNode[0] - 2.0 + 0.5), vx - 1), 0);
		int maxX = std::min(std::max(int(rayNode[0] + 2.0 + 0.5), 0), vx - 1);
		int minY = std::max(std::min(int(rayNode[1] - 2.0 + 0.5), vy - 1), 0);
		int maxY = std::min(std::max(int(rayNode[1] + 2.0 + 0.5), 0), vy - 1);
		int minZ = std::max(std::min(int(rayNode[2] - 2.0 + 0.5), vz - 1), 0);
		int maxZ = std::min(std::max(int(rayNode[2] + 2.0 + 0.5), 0), vz - 1);
		//Volumn localVol(maxX - minX + 1, maxY - minY + 1, maxZ - minZ + 1);
		int lx = maxX - minX + 1;
		int ly = maxY - minY + 1;
		int lz = maxZ - minZ + 1;
		/*存储射线区域*/
		Vec4d tmpNode;
		for (int ix = 0; ix < lx; ++ix){
			for (int iy = 0; iy < ly; ++iy){
				for (int iz = 0; iz < lz; ++iz){
					int newX  = minX + ix;
					int newY = minY + iy;
					int newZ = minZ + iz;
					tmpNode << newX, newY , newZ , (std::max)((double)origImg(newX, newY, newZ) - threv, 0.0) ;
					rayArea.push_back(tmpNode);
				}
			}
		}
	}
	if (!rayArea.empty()){
		/*清除冗余rayArea*/
		//sort from x-y-z-v
#ifdef __linux
        std::sort(rayArea.begin(), rayArea.end(), [](const Vec4d& lhs, const Vec4d& rhs){
            if(lhs(0) != rhs(0)) return lhs(0) < rhs(0);
            else if(lhs(1) != rhs(1)) return lhs(1) < rhs(1);
            else if(lhs(2) != rhs(2)) return lhs(2) < rhs(2);
            return lhs(3) < rhs(3);
        });//Node_v_x_y_z_MAX
#else
        std::sort(rayArea.begin(), rayArea.end(), Vec4d_0123less());//Node_v_x_y_z_MAX
#endif
		rayArea.erase(std::unique(rayArea.begin(), rayArea.end()), rayArea.end());
		Vec3d tmp;
		for (VectorVec4d::size_type i = 0; i < rayArea.size(); ++i){
			neighborWet.push_back(rayArea[i](3));
			//tmp(0) = (rayArea[i](0), rayArea[i](1), rayArea[i](2));
			tmp << rayArea[i](0), rayArea[i](1), rayArea[i](2);
			neighborPtSet.push_back(tmp);
		}
	}
	else{
		neighborWet.clear();
		neighborPtSet.clear();
	}

}

void TraceFilter::CalcNeighborSignalV2(const Volume<unsigned short> &origImg,
									 const Vec3d &curSeedNode, const Vec3d &initVec, const double threv,
									 VectorVec3d &neighborPtSet, std::vector<double> &neighborWet,
									 Vec3d &firDir, Vec3d &secDir)
{
	neighborPtSet.clear();
	neighborWet.clear();
	firDir.setZero();
	secDir.setZero();
	int vx = origImg.x();
	int vy = origImg.y();
	int vz = origImg.z();

	/*初始化*/
	//2015-8-13
	double xExtractRegion, yExtractRegion, zExtractRegion;
	xExtractRegion = yExtractRegion = 9.0;
	zExtractRegion = 9.0;
	int xMin = std::max(Round(curSeedNode(0) - xExtractRegion ), 0);
	int xMax = std::min(Round(curSeedNode(0) + xExtractRegion), vx - 1);
	int yMin = std::max(Round(curSeedNode(1) - yExtractRegion ), 0);
	int yMax = std::min(Round(curSeedNode(1) + yExtractRegion ), vy - 1);
	int zMin = std::max(Round(curSeedNode(2) - zExtractRegion ), 0);
	int zMax = std::min(Round(curSeedNode(2) + zExtractRegion ), vz - 1);

	Vec3d curCenter(curSeedNode(0) - (double)xMin,
		curSeedNode(1) - (double)yMin, curSeedNode(2) - (double)zMin);//ML1,中心点
	/*提取区域*/
	Volume<unsigned short> locOrigImg;
	//Change : 2014-3-19-10-16
	locOrigImg.SetSize(xMax - xMin + 1, yMax - yMin + 1, zMax - zMin + 1);
	//const Volume<unsigned short> &pptr = origImg;//局部区域
	for (int i = xMin; i <= xMax; ++i){
		for (int j = yMin; j <= yMax; ++j){
			for (int ij = zMin; ij <= zMax; ++ij){
				locOrigImg(i - xMin, j - yMin, ij - zMin) = origImg(i, j, ij);
			}
		}
	}
	/*局部大小*/
	int sx = xMax - xMin + 1;
	int sy = yMax - yMin + 1;
	int sz = zMax - zMin + 1;
	//Volumn subVol(xMax - xMin + 1, yMax - yMin + 1, vol.z);//nx1,ny1,nz1 zMax - zMin + 1
	/*最小边长*/
	int minLen = (sx) < (sy) ?
		((sx) < (sz) ? (sx) : (sz)) :
		((sy) < (sz) ? (sy) : (sz));

	double ii(0.0);
	double jj(0.0);
	double angleDiff(0.0);

	double angleThrev1=-0.95;
	double angleThrev2= 0.95;

	//2015-8-13
	if (minLen > 3 && vz > 3){
		VectorVec4d backwardNodeSet;//AAj
		for (int i = 0; i < 36; ++i){
			for ( int j = 0; j < 18; ++j){
				ii = (double)i * M_PI / 18.0;
				jj = (double)j * M_PI / 18.0;
				Vec3d polarPosition((std::sin(jj)) * (std::cos(ii)), (std::sin(ii)) * (std::sin(jj)), (std::cos(jj)));
				angleDiff = polarPosition.dot(initVec);/*jd*/
				if (angleDiff < angleThrev1) //if jd < -0.85
				{
					VectorVec3d rayNode;
					rayNode.push_back(curCenter + polarPosition);
					rayNode.push_back(curCenter + 3 * polarPosition);
					rayNode.push_back(curCenter + 5 * polarPosition);
					rayNode.push_back(curCenter + 7 * polarPosition);
					rayNode.push_back(curCenter + 8 * polarPosition);//2015-8-13
					std::vector<double> rayNodeWet;
					WeighRayValue(rayNode, locOrigImg, rayNodeWet);
					double meanRayNodeWet = std::accumulate(rayNodeWet.begin(), rayNodeWet.begin() + 3, 0.0);
					meanRayNodeWet /= 3.0;//akmmv.size();
					backwardNodeSet.push_back(Vec4d(polarPosition(0), polarPosition(1), polarPosition(2), meanRayNodeWet));
				}//if
			}
		}//for
		/*异常处理*/
		if (backwardNodeSet.size() < 2) firDir =  - initVec;
		else{
#ifdef __linux
            VectorVec4d::iterator maxItem = std::max_element(backwardNodeSet.begin(), backwardNodeSet.end(),
                [](const Vec4d& lhs, const Vec4d& rhs){
                    return lhs(3) < rhs(3);
            });//Node_4TH_MAX

#else
            VectorVec4d::iterator maxItem = std::max_element(backwardNodeSet.begin(), backwardNodeSet.end(),
                Vec4d_3th_less());//Node_4TH_MAX
#endif
			Vec3d maxVec((*maxItem)(0), (*maxItem)(1), (*maxItem)(2));//xss
			firDir = maxVec.normalized();//x10 =
		}
	}
	else firDir = - initVec;

	if (minLen > 3 && vz > 3){
		/*AAj2 = zeros(4,216)*/
		VectorVec4d forwardNodeSet;//AAj2
		for (int i = 0; i < 36; ++i){
			for ( int j = 0; j < 18; ++j){
				ii = (double)i * M_PI / 18.0;
				jj = (double)j * M_PI / 18.0;
				Vec3d ds1((std::sin(jj)) * (std::cos(ii)), (std::sin(ii)) * (std::sin(jj)) , (std::cos(jj)));//ds1
				angleDiff = ds1.dot(initVec);/*jd*/
				if (angleDiff > angleThrev2){ //if jd > -0.85
					VectorVec3d rayNode;
					rayNode.push_back(curCenter + ds1);
					rayNode.push_back(curCenter + 3 * ds1);
					rayNode.push_back(curCenter + 5 * ds1);
					rayNode.push_back(curCenter + 7 * ds1);
					rayNode.push_back(curCenter + 8 * ds1);//2015-8-13
					std::vector<double> rayNodeWet;
					WeighRayValue(rayNode, locOrigImg, rayNodeWet);
					double meanRayNodeWet = accumulate(rayNodeWet.begin(), rayNodeWet.begin()+3, 0.0);
					meanRayNodeWet /= 3.0;//akmmv.size();
					forwardNodeSet.push_back(Vec4d(ds1[0], ds1[1], ds1[2], meanRayNodeWet));
				}
			}
		}//for
		/*异常处理*/
		if (forwardNodeSet.size() < 2) secDir = initVec;
		else{
#ifdef __linux
            VectorVec4d::iterator maxItem = std::max_element(forwardNodeSet.begin(), forwardNodeSet.end(),
                [](const Vec4d& lhs, const Vec4d& rhs){
                    return lhs(3) < rhs(3);
            });//Node_4TH_MAX

#else
            VectorVec4d::iterator maxItem = std::max_element(forwardNodeSet.begin(), forwardNodeSet.end(),
                Vec4d_3th_less());//Node_4TH_MAX
#endif
			Vec3d maxVec((*maxItem).x(), (*maxItem).y(), (*maxItem).z());//xss
			secDir = maxVec.normalized();
		}
	}
	else secDir = initVec;

	VectorVec4d rayArea;
	for (int i = 0; i < 5; ++i){//2015-8-13
		Vec3d rayNode = 1.5 * i * firDir + curSeedNode;//2015-8-13
		int minX = std::max(std::min(Round(rayNode(0) - 3.0), vx - 1), 0);
		int maxX = std::min(std::max(Round(rayNode[0] + 3.0), 0), vx - 1);
		int minY = std::max(std::min(Round(rayNode[1] - 3.0), vy - 1), 0);
		int maxY = std::min(std::max(Round(rayNode[1] + 3.0), 0), vy - 1);
		int minZ = std::max(std::min(Round(rayNode[2] - 2.0), vz - 1), 0);
		int maxZ = std::min(std::max(Round(rayNode[2] + 2.0), 0), vz - 1);
		//Volumn localVol(maxX - minX + 1, maxY - minY + 1, maxZ - minZ + 1);
		int lx = maxX - minX + 1;
		int ly = maxY - minY + 1;
		int lz = maxZ - minZ + 1;
		/*存储射线区域*/
		Vec4d tmpNode;
		for (int ix = 0; ix < lx; ++ix){
			for (int iy = 0; iy < ly; ++iy){
				for (int iz = 0; iz < lz; ++iz){
					int newX = minX + ix;
					int newY = minY + iy;
					int newZ = minZ + iz;
					tmpNode << newX, newY , newZ , std::max(double(origImg(newX, newY, newZ)) - threv, 0.0) ;
					rayArea.push_back(tmpNode);
				}
			}
		}
	}
	/*第二条射线区域*/
	for (int i = 1; i < 5; ++i){//2015-8-13
		Vec3d rayNode = 1.5 * i * secDir + curSeedNode;//2015-8-13
		int minX = std::max(std::min(int(rayNode[0] - 2.0 + 0.5), vx - 1), 0);
		int maxX = std::min(std::max(int(rayNode[0] + 2.0 + 0.5), 0), vx - 1);
		int minY = std::max(std::min(int(rayNode[1] - 2.0 + 0.5), vy - 1), 0);
		int maxY = std::min(std::max(int(rayNode[1] + 2.0 + 0.5), 0), vy - 1);
		int minZ = std::max(std::min(int(rayNode[2] - 2.0 + 0.5), vz - 1), 0);
		int maxZ = std::min(std::max(int(rayNode[2] + 2.0 + 0.5), 0), vz - 1);
		//Volumn localVol(maxX - minX + 1, maxY - minY + 1, maxZ - minZ + 1);
		int lx = maxX - minX + 1;
		int ly = maxY - minY + 1;
		int lz = maxZ - minZ + 1;
		/*存储射线区域*/
		Vec4d tmpNode;
		for (int ix = 0; ix < lx; ++ix){
			for (int iy = 0; iy < ly; ++iy){
				for (int iz = 0; iz < lz; ++iz){
					int newX  = minX + ix;
					int newY = minY + iy;
					int newZ = minZ + iz;
					tmpNode << newX, newY , newZ , (std::max)((double)origImg(newX, newY, newZ) - threv, 0.0) ;
					rayArea.push_back(tmpNode);
				}
			}
		}
	}
	if (!rayArea.empty()){
		/*清除冗余rayArea*/
		//sort from x-y-z-v
#ifdef __linux
        std::sort(rayArea.begin(), rayArea.end(), [](const Vec4d& lhs, const Vec4d& rhs){
            if(lhs(0) != rhs(0)) return lhs(0) < rhs(0);
            else if(lhs(1) != rhs(1)) return lhs(1) < rhs(1);
            else if(lhs(2) != rhs(2)) return lhs(2) < rhs(2);
            return lhs(3) < rhs(3);
        });//Node_v_x_y_z_MAX
#else
        std::sort(rayArea.begin(), rayArea.end(), Vec4d_0123less());//Node_v_x_y_z_MAX

#endif
		rayArea.erase(std::unique(rayArea.begin(), rayArea.end()), rayArea.end());
		Vec3d tmp;
		for (VectorVec4d::size_type i = 0; i < rayArea.size(); ++i){
			neighborWet.push_back(rayArea[i](3));
			//tmp(0) = (rayArea[i](0), rayArea[i](1), rayArea[i](2));
			tmp << rayArea[i](0), rayArea[i](1), rayArea[i](2);
			neighborPtSet.push_back(tmp);
		}
	}
	else{
		neighborWet.clear();
		neighborPtSet.clear();
	}
}

void TraceFilter::WeighRayValue(const VectorVec3d &rayNode, const Volume<unsigned short> &locOrigImg,
                                std::vector<double> &rayNodeWet)
{
    typedef double spheredist;
    int nxss = int(rayNode.size());
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

void TraceFilter::TraceCurvesFromSeed(const Volume<unsigned short> &origImg, const Volume<unsigned short> &backImg,
                                         const Vec3d &initialPoint, Volume<int> &indexImg,
                                      const VectorVec3d &somaList, const int seedId,
                                         VectorVec5d &resultCurve, Mat2i &resultCurConInfo)
{
    int nxx = origImg.x();
    int nyy = origImg.y();
    int nzz = origImg.z();

    resultCurve.clear();
    VectorVec5d rawSeedCurve;
    rawSeedCurve.clear();
    resultCurConInfo.setZero();
    Vec3d roundInitPt;
    Vec3d initDirection;//, x2, x3;

    if ( 0 == indexImg((int)initialPoint(0), (int)initialPoint(1), (int)initialPoint(2)) ){
		if (origImg.XResolution() * origImg.YResolution() * origImg.ZResolution() > 1.5) {
			CalcInitDirectionOnTraceSeed(origImg, backImg, initialPoint, 8.0, initDirection);//2015-8-13
		}else{
			CalcInitDirectionOnTraceSeed(origImg, backImg, initialPoint, 12.0, initDirection);//2015-8-13
		}
        roundInitPt = Vec3d( Round(initialPoint(0) ), Round(initialPoint(1)), Round(initialPoint(2) )  );

        int xMin, xMax, yMin, yMax, zMin, zMax;
        Get3DRegion(xMin, xMax, yMin, yMax, zMin, zMax, (int)initialPoint(0), (int)initialPoint(1), (int)initialPoint(2),
            2, 2, 2, 0, (int)nxx - 1, 0, (int)nyy - 1, 0, (int)nzz - 1);

        //test
        //int** dataptr_ = indexImg.GetPointer();
        //
        int collideCount = 0;
        for (int i = xMin; i <= xMax; ++i){
            for (int j = yMin; j <= yMax; ++j){
                for (int ij = zMin; ij <= zMax; ++ij){
                    if (  0 != indexImg(i, j, ij)  ) ++collideCount;
                }
            }
        }

        if (collideCount > 83.3333 /*250 / 3*/) initDirection.setZero();
    }
    else initDirection.setZero();

    if (!TraceUtil::IsEqualDouble(initDirection(0) , 0.0)
            || !TraceUtil::IsEqualDouble(initDirection(1) , 0.0)
            || !TraceUtil::IsEqualDouble(initDirection(2) , 0.0)){
        //initDirection.array().abs().sum() > 0.0 2014-4-21
        //Vec3d data2(roundInitPt);
        //std::vector<double> mmsm, mmsm1;
        Vec2i tailCurConInfo, headCurConInfo;
        VectorVec5d backwardCurve;
        VectorVec5d forwardCurve;

        TraceCurvesForwardFromSeed(-initDirection, roundInitPt, indexImg, somaList, backImg, origImg,
            backwardCurve, tailCurConInfo);

        TraceCurvesForwardFromSeed(initDirection, roundInitPt, indexImg, somaList, backImg, origImg,
            forwardCurve, headCurConInfo);

        int nx = forwardCurve.size();

        resultCurConInfo.col(0) = headCurConInfo;
        resultCurConInfo.col(1) = tailCurConInfo;

        if (nx > 2){
            for (int n = nx - 2; n > -1; --n)
                rawSeedCurve.push_back(forwardCurve[n]);
            for (VectorVec5d::size_type n = 0; n < backwardCurve.size(); ++n)
                rawSeedCurve.push_back(backwardCurve[n]);
        }//if
        else rawSeedCurve = backwardCurve;
    }

    VectorVec5d resultCurveCopy;
    if (rawSeedCurve.size() > 4){
        size_t nxxs = rawSeedCurve.size();
        VectorVec3d rawSeedCurvePtList;
        Vec3d tmp;
        for (size_t i = 0; i < nxxs; ++i){
            tmp << rawSeedCurve[i](0), rawSeedCurve[i](1), rawSeedCurve[i](2);
            rawSeedCurvePtList.push_back(tmp);
        }
        std::vector<double> radius;
        std::vector<double> rav;
        CalcParmOfCurveNodeList(origImg, backImg, rawSeedCurvePtList, radius, rav);

        for (size_t i = 0; i < nxxs; ++i){
            rawSeedCurve[i](3) = radius[i];
            rawSeedCurve[i](4) = rav[i];
        }

        Vec5d half;
        //要插值
        for (size_t i = 0; i < nxxs - 1; ++i){
            resultCurveCopy.push_back(rawSeedCurve[i]);
            half = 0.5 * (rawSeedCurve[i] + rawSeedCurve[i + 1]);
            resultCurveCopy.push_back(half);
        }
        resultCurveCopy.push_back(rawSeedCurve[nxxs - 1]);
        //2015-8-13
        std::vector<double> three;
		if(origImg.XResolution() * origImg.YResolution() * origImg.ZResolution() > 1.5){
			for (VectorVec5d::size_type i = 0; i < resultCurveCopy.size(); ++i)
				three.push_back(std::min(2.0,
										 double(std::max(1.0, double(Round(resultCurveCopy[i](3) + 0.5) )))) );//这里是1
		}else{
			for (VectorVec5d::size_type i = 0; i < resultCurveCopy.size(); ++i)
				three.push_back(std::min(5.0,
				double(std::max(1.0, double(Round(resultCurveCopy[i](3) + 2.5) )))) );
		}

        int id1(0), id2(0), id3(0);
        int xMin, xMax, yMin, yMax, zMin, zMax;
		//2015-8-13
		if(origImg.XResolution() * origImg.YResolution() * origImg.ZResolution() > 1.5){
			//Change:2014-3-21-21-42
			for (VectorVec5d::size_type ik = 1; ik < 2*nxxs - 2; ++ik){
				id1 = Round(resultCurveCopy[ik](0) );
				id2 = Round(resultCurveCopy[ik](1) );
				id3 = Round(resultCurveCopy[ik](2) );

				Get3DRegion(xMin, xMax, yMin, yMax, zMin, zMax, id1, id2, id3,
					std::max(0, (int)three[ik]), std::max(0, (int)three[ik]), std::max(0, (int)three[ik]),
					0, (int)nxx - 1, 0, (int)nyy - 1, 0, (int)nzz - 1);

				for (int ii = xMin; ii <= xMax; ++ii){
					for (int jj = yMin; jj <= yMax; ++jj){
						for (int kk = zMin; kk <= zMax; ++kk){
							if (indexImg(ii, jj, kk) == 0) indexImg(ii, jj, kk) = seedId;
						}
					}//for
				}//for
			}
		}else{
			//Change:2015-8-13
			for (VectorVec5d::size_type ik = 1; ik < 2*nxxs - 2; ++ik){
				id1 = Round(resultCurveCopy[ik](0) );
				id2 = Round(resultCurveCopy[ik](1) );
				id3 = Round(resultCurveCopy[ik](2) );

				Get3DRegion(xMin, xMax, yMin, yMax, zMin, zMax, id1, id2, id3,
					std::max(0, (int)three[ik]), std::max(0, (int)three[ik]), std::max(0, (int)three[ik]),
					0, (int)nxx - 1, 0, (int)nyy - 1, 0, (int)nzz - 1);

				for (int ii = xMin; ii <= xMax; ++ii){
					for (int jj = yMin; jj <= yMax; ++jj){
						for (int kk = zMin; kk <= zMax; ++kk){
							double dist = (Vec3d( double(ii), double(jj), double(kk)) - Vec3d(double(id1), double(id2), double(id3))).norm();
							if (indexImg(ii, jj, kk) == 0 && dist < 2.0) indexImg(ii, jj, kk) = seedId;
							if(indexImg(ii, jj, kk) == 0 && dist >= 2.0
								&& double(origImg(ii,jj,kk) - backImg(ii,jj,kk)) >  std::max(fillThrev, 4.5* std::sqrt(double(backImg(ii,jj,kk))))) 
								indexImg(ii, jj, kk) = seedId;
						}
					}//for
				}//for
			}
		}
        
        resultCurve.swap(resultCurveCopy);
    }
    else{
        resultCurve.clear();
        resultCurConInfo.setZero();
    }
}

void TraceFilter::CalcInitDirectionOnTraceSeed(const Volume<unsigned short> &origImg, const Volume<unsigned short> &backImg,
                                         const Vec3d &initPoint, const double windowSize,
                                         Vec3d &vec1/*, Vec3d &vec2, Vec3d &vec3*/)
{
    int nx = origImg.x();
    int ny = origImg.y();
    int nz = origImg.z();

    vec1.setZero();
    //vec2.setZero();
    //vec3.setZero();

    int xMin, xMax, yMin, yMax, zMin, zMax;
    Get3DRegion(xMin, xMax, yMin, yMax, zMin, zMax, (int)initPoint(0), (int)initPoint(1), (int)initPoint(2),
                (int)windowSize, (int)windowSize, (int)windowSize,
                0, nx - 1, 0, ny - 1, 0, nz - 1);

    Vec3d locCenter(initPoint(0) - (double)xMin, initPoint(1) - (double)yMin,
        initPoint(2) - (double)zMin);//ML1,中心点
    /*局部大小*/

    int sx = xMax - xMin + 1;
    int sy = yMax - yMin + 1;
    int sz = zMax - zMin + 1;

    Volume<unsigned short> locOrigImg;
    Volume<unsigned short> locBackImg;
    ExtractArea(origImg, xMin, xMax, yMin, yMax, zMin, zMax, locOrigImg);
    ExtractArea(backImg, xMin, xMax, yMin, yMax, zMin, zMax, locBackImg);
    const Volume<unsigned short> &pptr = locOrigImg;//XXv
    const Volume<unsigned short> &bptr = locBackImg;//XXb

    VectorVec4d rayWet; //AAj,Node
    //Vec4d maxRay;	//L11
    //std::vector<double> MMk;//不知道是什么

    /*射线选择*/
    double ii(0.0), jj(0.0);
    double rayNodeWet(0.0);//akmv
    Vec3d aRayNode;//mms
    int minX(0), maxX(0), minY(0), maxY(0), minZ(0), maxZ(0);
    double tmpDense(0.0);//akmv的临时
    for (int i = 0; i < 36; ++i){
        for (int j = 0; j < 12; ++j){
            jj = (double)j * M_PI / 24.0;
            ii = (double)i * M_PI / 18.0;
            Vec3d spaceAngle((std::sin(jj)) * (std::cos(ii)), (std::sin(ii)) * (std::sin(jj)) , (std::cos(jj)));//ds1,射线方位
            rayNodeWet = 0.0;
            for (int tt = -8; tt <= 8; ++tt){
                aRayNode = locCenter + tt * spaceAngle; // mms = ML1 + tt * ds1
                /*该方向上不同长度的强度*/
                minX = std::max(std::min(Round(aRayNode[0] - 1.0 ), sx - 1), 0);
                maxX = std::min(std::max(Round(aRayNode[0] + 1.0 ), 0), sx - 1);
                minY = std::max(std::min(Round(aRayNode[1] - 1.0 ), sy - 1), 0);
                maxY = std::min(std::max(Round(aRayNode[1] + 1.0 ), 0), sy - 1);
                minZ = std::max(std::min(Round(aRayNode[2] - 1.0 ), sz - 1), 0);
                maxZ = std::min(std::max(Round(aRayNode[2] + 1.0 ), 0), sz - 1);
                tmpDense = 0.0; //akmmv2
                /*求强度*/
                for (int lx = minX; lx <= maxX; ++lx){
                    for (int ly = minY; ly <= maxY; ++ly){
                        for (int lz = minZ; lz <= maxZ; ++lz){
                            tmpDense += (double)pptr(lx, ly, lz ) -
                                bptr(lx , ly , lz);//SSv
                        }
                    }
                }
                tmpDense /= (maxX - minX + 1)*(maxY - minY + 1)*(maxZ - minZ + 1);
                rayNodeWet += tmpDense;
            }
            rayNodeWet /= 13.0;
            Vec4d tmpRay(spaceAngle(0), spaceAngle(1), spaceAngle(2), rayNodeWet);
            rayWet.push_back(tmpRay);
        }
    }

    aRayNode << Round(locCenter(0) ), Round(locCenter(1) ), Round(locCenter(2) ) ;

    minX = std::max(std::min(Round(aRayNode[0] - 1.0 ), sx - 1), 0);
    maxX = std::min(std::max(Round(aRayNode[0] + 1.0 ), 0), sx - 1);
    minY = std::max(std::min(Round(aRayNode[1] - 1.0 ), sy - 1), 0);
    maxY = std::min(std::max(Round(aRayNode[1] + 1.0 ), 0), sy - 1);
    minZ = std::max(std::min(Round(aRayNode[2] - 1.0 ), sz - 1), 0);
    maxZ = std::min(std::max(Round(aRayNode[2] + 1.0 ), 0), sz - 1);

    double backValue(0.0);

    for (int lx = minX; lx <= maxX; ++lx){
        for (int ly = minY; ly <= maxY; ++ly){
            for (int lz = minZ; lz <= maxZ; ++lz){
                backValue += (double)bptr(lx, ly, lz);//SSv
            }
        }
    }

    backValue /= (maxX - minX + 1)*(maxY - minY + 1)*(maxZ - minZ + 1);

    /*排序,依次是强度，x,y,z*/
#ifdef __linux
    VectorVec4d::iterator maxItem = std::max_element(rayWet.begin(), rayWet.end(), [](const Vec4d& lhs, const Vec4d& rhs){
        if(lhs(3) != rhs(3)) return lhs(3) < rhs(3);
        else if(lhs(0) != rhs(0)) return lhs(0) < rhs(0);
        else if(lhs(1) != rhs(1)) return lhs(1) < rhs(1);
        return lhs(2) < rhs(2);
    });//Node_v_x_y_z_MAX
#else
    VectorVec4d::iterator maxItem = std::max_element(rayWet.begin(), rayWet.end(), Vec4d_3012less());//Node_v_x_y_z_MAX

#endif
    //maxRay << (*maxItem)(0), (*maxItem)(1), (*maxItem)(2), (*maxItem)(3);
    Vec3d maxTransItem((*maxItem)(0), (*maxItem)(1), (*maxItem)(2));//L11(1:3)'

    /*求取std(AAj(4,:))*/
    double meanRayWet = 0.0;
    double sumRayWet = 0.0;
    for (VectorVec4d::size_type i = 0; i < rayWet.size(); ++i){
        meanRayWet += rayWet[i](3);
    }
    meanRayWet /= rayWet.size();
    for (VectorVec4d::size_type i = 0; i < rayWet.size(); ++i){
        sumRayWet += std::pow(rayWet[i](3) - meanRayWet, 2.0);
    }
    double stdRayWet = std::sqrt(sumRayWet / (double)(rayWet.size() - 1));

    /*if max_v>min(2.5*sqrt(bc),5*std(AAj(4,:)))*/
    if ((*maxItem)(3) > (std::min)(2.5 * std::sqrt(backValue), 5.0 * stdRayWet)){
        vec1 = maxTransItem;
        vec1.normalize();
        //CalcOrthoBasis(vec1, vec2, vec3);//directionc
    }
    else{
        vec1.setZero();
        //vec2.setZero();
        //vec3.setZero();
    }
}

void TraceFilter::TraceCurvesForwardFromSeed(const Vec3d &seedInitDir,
                                           const Vec3d &initPt, const Volume<int> &indexImg, const VectorVec3d &somaList,
                                           const Volume<unsigned short> &backImg, const Volume<unsigned short> &origImg,
                                           VectorVec5d &resultSeedCurve, Vec2i &curConInfo)
{
    resultSeedCurve.clear();
    Vec5d tmp1; tmp1.setZero();
    tmp1(0) = initPt(0);
    tmp1(1) = initPt(1);
    tmp1(2) = initPt(2);
    resultSeedCurve.push_back(tmp1);
    curConInfo.setZero();

    int isNextNodeValid(1);
    int i = 0;

    int nxx = origImg.x();
    int nyy = origImg.y();
    int nzz = origImg.z();

    Vec3d nextDendDir(seedInitDir);
    Vec3d fixDendDir(seedInitDir);
    VectorVec3d saveDirection;
    saveDirection.reserve(3000);
    saveDirection.push_back(nextDendDir);
    std::vector<int> CC_infor(3000,0);
    //2015-8-13
	double xExtractRegion, yExtractRegion, zExtractRegion;
	if (origImg.XResolution() * origImg.YResolution() * origImg.ZResolution() > 1.5) {
		xExtractRegion = yExtractRegion = 10;
		zExtractRegion = 7;
	}else{
		xExtractRegion = yExtractRegion = 15;
		zExtractRegion = 10;
	}

	//2015-8-13
	bool isHighResolution = false;
	double endThrev = 2.0;
	typedef void (TraceFilter::*MyTraceNextCurveNode)( const Volume<unsigned short> &, const Vec3d &,const double &, const Vec3d &,
		Vec3d &, Vec3d &,int & );
	typedef void (TraceFilter::*MyDetectCollision)(const VectorVec5d &, const VectorVec3d &,
		const Vec2i &, const Volume<int> &,
		const Volume<unsigned short> &, const Volume<unsigned short> &,
		const int, const int, const int,
		VectorVec5d &, int &,
		Vec2i &, int &, int& );
	MyTraceNextCurveNode curTraceNextCurveNode;
	MyDetectCollision curDetectCollision;
	if (origImg.XResolution() * origImg.YResolution() * origImg.ZResolution() > 1.5) {
		curTraceNextCurveNode = &TraceFilter::TraceNextCurveNode;
		curDetectCollision = &TraceFilter::DetectCollision;
		isHighResolution = false;
		endThrev = 2.0;
	}else{
		//printf("TraceFilter::TraceNextCurveNodeV2\n");
		curTraceNextCurveNode = &TraceFilter::TraceNextCurveNodeV2;
		curDetectCollision = &TraceFilter::DetectCollisionV2;
		isHighResolution = true;
		endThrev = 4.0;
	}

    while(isNextNodeValid == 1 && i < 3000) {
        int xMin, xMax, yMin, yMax, zMin, zMax;

        Get3DRegion(xMin, xMax, yMin, yMax, zMin, zMax,
            Round(resultSeedCurve[i](0)), Round(resultSeedCurve[i](1)), Round(resultSeedCurve[i](2) ),
            xExtractRegion, yExtractRegion, zExtractRegion, 0, nxx - 1, 0, nyy - 1, 0, nzz - 1);//2015-8-13

        Volume<unsigned short> locOrigImg;//(xMax - xMin + 1, yMax - yMin + 1, zMax - zMin + 1);
        ExtractArea(origImg, xMin, xMax, yMin, yMax, zMin, zMax, locOrigImg);
        Vec3d lefttopAxis(xMin, yMin, zMin);
        int rdrvx = std::min(std::max(Round(resultSeedCurve[i](0) ), 0), nxx - 1);
        //int rdrvy = std::min(std::max((int)(resultSeedCurve[i](1) - 3.0 + 0.5), 0), nyy - 1);
      //2014-4-22
        int rdrvy = std::min(std::max(Round(resultSeedCurve[i](1) ), 0), nyy - 1);
        int rdrvz = std::min(std::max(Round(resultSeedCurve[i](2) ), 0), nzz - 1);
 //double threv = backImg(rdrvx, rdrvy, (int)((double)(zMin + zMax) / 2.0 + 0.5));
        double threv = backImg(rdrvx, rdrvy, rdrvz);
        Vec3d tmpCurrentCurveNode(resultSeedCurve[i](0) - xMin, resultSeedCurve[i](1) - yMin, resultSeedCurve[i](2) - zMin);

        Vec3d nextCurveNode;
        nextCurveNode.setZero();
        //Vec3d vmmk; vmmk.setZero(); vmmk = tmpx1
        Vec3d nextDendDirCopy;//, tmpx2, tmpx3;
		(this->*curTraceNextCurveNode)(locOrigImg, tmpCurrentCurveNode, threv, nextDendDir, nextCurveNode, nextDendDirCopy,
            isNextNodeValid);
        nextDendDir = nextDendDirCopy;
        Vec3d vmmk = nextDendDirCopy;

        //TODO:2014-3-19-19-00

        if ( !nextCurveNode.isZero() ){
            nextCurveNode += lefttopAxis;
            resultSeedCurve[i].block(0,0,3,1)=nextCurveNode;
        }

        Vec3d tmpCurCurveNode = resultSeedCurve.back().block(0,0,3,1);
        if(tmpCurCurveNode.minCoeff() < -1.0 || tmpCurCurveNode(0) > nxx
           || tmpCurCurveNode(1) > nyy
           || tmpCurCurveNode(2) > nzz){
            isNextNodeValid = 0;
        }

        if (i < 4){
            nextDendDir = 0.5* nextDendDir + 0.5* fixDendDir;
            nextDendDir.normalize();
//            VectorVec3d dataPrinci;
//            for (int n = std::max(i - 6, 0) ; n <= i; ++n  ){
//                dataPrinci.push_back(Vec3d(resultSeedCurve[n](0), resultSeedCurve[n](1), resultSeedCurve[n](2)));;
//            }
//            Vec3d sx1;//, sx2, sx3;
//            CalcPtCurveDirection(dataPrinci, sx1);
//            nextDendDir = sx1;
        }

        if ( i > 3 ){//&& i < 3000
            int isNextNodeValidCp;
            Vec2i curConInfoCp;
            int iCp;
            int C_infor;
            VectorVec3d tmpCurve;
            VectorVec5d modifiedResultSeedCurve;
            TraceUtil::GetPartVectorVec3d(resultSeedCurve, std::max<int>(i-10,0), i-1, tmpCurve);
            CalcPtCurveDirection(tmpCurve, nextDendDir);
            (this->*curDetectCollision)(resultSeedCurve, somaList, curConInfo, indexImg, origImg, backImg,
                            i,0,isNextNodeValid, modifiedResultSeedCurve, isNextNodeValidCp,
                            curConInfoCp, iCp, C_infor);
            resultSeedCurve.swap(modifiedResultSeedCurve);
            isNextNodeValid = isNextNodeValidCp;
            curConInfo = curConInfoCp;
            i = iCp;
            if( int(CC_infor.size()) < i + 1)
                CC_infor.resize(i+1);
            CC_infor[i]=C_infor;
            if(C_infor > 0){
                if(int(saveDirection.size()) < i+1)
                    saveDirection.resize(i+1);
                saveDirection[i-3]=nextDendDir;
                saveDirection[i-2]=nextDendDir;
                saveDirection[i-1]=nextDendDir;
                saveDirection[i]=nextDendDir;
            }
        }

        if ( i > 3 ){//matlab is 4
            Vec3d nearNodeDist; nearNodeDist.setZero();
            for (int iij = 1; iij < 4; ++iij){
                Vec3d tmp11(resultSeedCurve[i - iij](0), resultSeedCurve[i - iij](1), resultSeedCurve[i - iij](2));
                Vec3d tmp22(resultSeedCurve[i - iij + 1](0), resultSeedCurve[i - iij + 1](1), resultSeedCurve[i - iij + 1](2));
                nearNodeDist(iij - 1) = (tmp22 - tmp11).norm();
            }
            VectorVec3d dataPrincipald;
            Vec3d tmpDataPrin;
            for ( int n = std::max(i - 10, 0) ; n <= i - 1; ++n ){
                tmpDataPrin(0) = resultSeedCurve[n](0);
                tmpDataPrin(1) = resultSeedCurve[n](1);
                tmpDataPrin(2) = resultSeedCurve[n](2);
                dataPrincipald.push_back(tmpDataPrin);
            }
            Vec3d dir1;//, sx2, sx3;
            CalcPtCurveDirection(dataPrincipald, dir1);
            nextDendDir = dir1;//Change 2014-3-20-15-33
            //
            Vec3d corr1 = saveDirection[std::max<int>(i-6,0)];
            Vec3d corr2 = resultSeedCurve[i-1].block(0,0,3,1) - resultSeedCurve[std::max<int>(i-6,0)].block(0,0,3,1);
            corr2(2)*=2.0;//warning here!
            Vec3d corr3 = resultSeedCurve[i-2].block(0,0,3,1) - resultSeedCurve[std::max<int>(i-6,0)].block(0,0,3,1);
            corr3(2)*=2.0;
            Vec3d corr4 = resultSeedCurve[i-2].block(0,0,3,1) - resultSeedCurve[std::max<int>(i-7,0)].block(0,0,3,1);
            corr4(2)*=2.0;
            Vec3d corr5 = resultSeedCurve[i-1].block(0,0,3,1) - resultSeedCurve[std::max<int>(i-7,0)].block(0,0,3,1);
            corr5(2)*= 2.0;
            std::vector<double> corrList(4,0.0);
            //double corr12
            corrList[0] = corr1.dot(corr2) / (corr1.norm() * corr2.norm() + 0.001);
            corrList[1] = corr1.dot(corr3) / (corr1.norm() * corr3.norm() + 0.001);
            corrList[2] = corr1.dot(corr4) / (corr1.norm() * corr4.norm() + 0.001);
            corrList[3] = corr1.dot(corr5) / (corr1.norm() * corr5.norm() + 0.001);

            if (nearNodeDist.minCoeff() < 0.15){//0.25
                isNextNodeValid = 0;
                --i;
            }

            if(MaxValueInVector(corrList) < 0.5 || MinValueInVector(corrList) < -0.5){
                isNextNodeValid = 0;
                i -= 3;
            }

            if (isNextNodeValid>0){//
                VectorVec3d tmpCurve;
                TraceUtil::GetPartVectorVec3d(resultSeedCurve, i-1,i, tmpCurve);
                std::vector<double> origWet, backWet, origbackWet;
                WeighRayValue(tmpCurve, origImg, origWet);
                WeighRayValue(tmpCurve, backImg, backWet);
                for(size_t k = 0; k < origWet.size(); ++k){
                    origbackWet.push_back(origWet[k] - backWet[k]-endThrev
                                          * std::sqrt(backWet[k]));
                }
                if(MaxValueInVector(origbackWet) < 0.0){
					if (isHighResolution) {
						isNextNodeValid = 0;
					}else{
						VectorVec5d tryForwardCurve;
						RayburstShapeTrack(resultSeedCurve[i-3].block(0,0,3,1),origImg,
							nextDendDir, 5, tryForwardCurve);
						if(tryForwardCurve.size() > 2){
							for(int k = i-2; k <= i; ++k)
								resultSeedCurve[k].block(0,0,3,1)=
								tryForwardCurve[k - i + 2].block(0,0,3,1);
							VectorVec3d tmpCurve1;
							TraceUtil::GetPartVectorVec3d(resultSeedCurve, std::max<int>(i - 9, 0),
								i, tmpCurve1);
							CalcPtCurveDirection(tmpCurve1, nextDendDir);
						} else{
							isNextNodeValid = 0;
						}
					} 
                }
            }
        }

        if(isNextNodeValid == 1 && CC_infor[i]==0){
            if(int(saveDirection.size()) < i+1)
                saveDirection.resize(i+1);
            saveDirection[i] = nextDendDir;
            ++i;
            Vec3d modifiedCurveNode;
            nextCurveNode = resultSeedCurve[i-1].block(0,0,3,1);
            PushNodeUseRayBurst(nextCurveNode, origImg, threv, vmmk,modifiedCurveNode);
            if(int(resultSeedCurve.size()) < i+1)
                resultSeedCurve.resize(i+1);
            resultSeedCurve[i].block(0,0,3,1)=modifiedCurveNode;
        }
    }
    if(i+1 < int(resultSeedCurve.size())){
        resultSeedCurve.erase(resultSeedCurve.begin() + i + 1, resultSeedCurve.end());
    }
}
//2014-4-20
void TraceFilter::IsCurvesCollide(const Volume<int> &indexImg,
                                  const int xMin, const int xMax,
                                  const int yMin, const int yMax,
                                  const int zMin, const int zMax,
                                  int &conInfo, double &curveCollideFlag,
                                  double &somaCollideFlag)
{
    conInfo = 0;
    curveCollideFlag = 0;
    somaCollideFlag = 1;
    std::vector<int> indexWindowVal;
    int kkk = 0;
    for (int i = xMin; i <= xMax; ++i){
        for (int j = yMin; j <= yMax; ++j){
            for (int k = zMin; k <= zMax; ++k){
                if ( 0 != indexImg(i, j, k) ) indexWindowVal.push_back( indexImg(i, j, k) );
                if ( 0 > indexImg(i, j, k)  ) ++kkk;
            }
        }
    }//for
    //2014-4-24
    if ( indexWindowVal.size() >  75 || kkk > 60 ){
        std::sort(indexWindowVal.begin(), indexWindowVal.end());
        std::vector<int>::const_iterator maxIt = std::max_element(indexWindowVal.begin(), indexWindowVal.begin() + 60);
        if ( *maxIt < 0 ){
            somaCollideFlag = -1;
            conInfo=indexWindowVal[0];
//            Vec3d seedSomaDir(somaList[ -indexWindowVal[0] - 1 ]);
//            seedSomaDir -= currentCurNode;
//            seedSomaDir.normalize();
//            double collideAngle = seedSomaDir.transpose() * mainDir;
//            if ( collideAngle > -0.5  ){
//                conInfo = indexWindowVal[0];
//                somaCollideFlag = -collideAngle;
//            }
        }
        else{
            //jumpIndexVal=diff(indexWindowVal);
            std::deque<int> jumpIndexVal;
            //diff_vector(L, Lk);
            std::adjacent_difference(indexWindowVal.begin(), indexWindowVal.end(), std::back_inserter(jumpIndexVal));
            jumpIndexVal.pop_front();//do not need first element
            // jumpIndexList=[indexWindowVal((jumpIndexVal~=0)), indexWindowVal(end)];
            std::deque<int> jumpIndexList;
            for (std::deque<int>::size_type ii = 0; ii < jumpIndexVal.size(); ++ii){
                if ( 0 != jumpIndexVal[ii] )
                    jumpIndexList.push_back(indexWindowVal[ii]);
            }
            jumpIndexList.push_back( indexWindowVal[indexWindowVal.size() - 1] );//end
            //collideSets=diff([0,find(jumpIndexVal~=0),length(indexWindowVal)]);
            //std::deque<int>::size_type nxrs = jumpIndexList.size();
            std::deque<int> collideSets;
            std::vector<int> tmprs1;
            //[0,find(jumpIndexVal~=0),length(indexWindowVal)]
            tmprs1.push_back(0);
            for (std::deque<int>::size_type ii = 0; ii < jumpIndexVal.size(); ++ii){
                if ( 0 != jumpIndexVal[ii]  ) tmprs1.push_back(ii);
            }
            tmprs1.push_back(indexWindowVal.size());
            //diff_vector(tmprs1, rs1);
            std::adjacent_difference(tmprs1.begin(), tmprs1.end(), std::back_inserter(collideSets));
            collideSets.pop_front();//do not need first element
            // [idexv,idexx]=max(collideSets);
            std::deque<int>::const_iterator maxIt = std::max_element(collideSets.begin(), collideSets.end());
            conInfo = jumpIndexList[maxIt - collideSets.begin()];
            curveCollideFlag = double(*maxIt) / 125.0;
        }
    }
}

//2015-8-13
void TraceFilter::IsCurvesCollideV2(const Volume<int> &indexImg,
								  const int xMin, const int xMax,
								  const int yMin, const int yMax,
								  const int zMin, const int zMax,
								  int &conInfo, double &curveCollideFlag,
								  double &somaCollideFlag)
{
	conInfo = 0;
	curveCollideFlag = 0;
	somaCollideFlag = 1;
	std::vector<int> indexWindowVal;
	int kkk = 0;
	for (int i = xMin; i <= xMax; ++i){
		for (int j = yMin; j <= yMax; ++j){
			for (int k = zMin; k <= zMax; ++k){
				if ( 0 != indexImg(i, j, k) ) indexWindowVal.push_back( indexImg(i, j, k) );
				if ( 0 > indexImg(i, j, k)  ) ++kkk;
			}
		}
	}//for
	//2014-4-24
	if ( indexWindowVal.size() >  60 || kkk > 60 ){
		std::sort(indexWindowVal.begin(), indexWindowVal.end());
		std::vector<int>::const_iterator maxIt = std::max_element(indexWindowVal.begin(), indexWindowVal.begin() + 50);
		if ( *maxIt < 0 ){
			somaCollideFlag = -1;
			conInfo=indexWindowVal[0];
		}
		else{
			//jumpIndexVal=diff(indexWindowVal);
			std::deque<int> jumpIndexVal;
			//diff_vector(L, Lk);
			std::adjacent_difference(indexWindowVal.begin(), indexWindowVal.end(), std::back_inserter(jumpIndexVal));
			jumpIndexVal.pop_front();//do not need first element
			// jumpIndexList=[indexWindowVal((jumpIndexVal~=0)), indexWindowVal(end)];
			std::deque<int> jumpIndexList;
			for (std::deque<int>::size_type ii = 0; ii < jumpIndexVal.size(); ++ii){
				if ( 0 != jumpIndexVal[ii] )
					jumpIndexList.push_back(indexWindowVal[ii]);
			}
			jumpIndexList.push_back( indexWindowVal[indexWindowVal.size() - 1] );//end
			//collideSets=diff([0,find(jumpIndexVal~=0),length(indexWindowVal)]);
			//std::deque<int>::size_type nxrs = jumpIndexList.size();
			std::deque<int> collideSets;
			std::vector<int> tmprs1;
			//[0,find(jumpIndexVal~=0),length(indexWindowVal)]
			tmprs1.push_back(0);
			for (std::deque<int>::size_type ii = 0; ii < jumpIndexVal.size(); ++ii){
				if ( 0 != jumpIndexVal[ii]  ) tmprs1.push_back(ii);
			}
			tmprs1.push_back(indexWindowVal.size());
			//diff_vector(tmprs1, rs1);
			std::adjacent_difference(tmprs1.begin(), tmprs1.end(), std::back_inserter(collideSets));
			collideSets.pop_front();//do not need first element
			// [idexv,idexx]=max(collideSets);
			std::deque<int>::const_iterator maxIt = std::max_element(collideSets.begin(), collideSets.end());
			conInfo = jumpIndexList[maxIt - collideSets.begin()];
			curveCollideFlag = double(*maxIt) / 125.0;
		}
	}
}

void TraceFilter::TraceCurvesFromSoma(const Volume<unsigned short> &origImg, const Volume<unsigned short> &backImg,
                                      Volume<int> &resultIndexImg, const VectorVec5d &thickNodeList, const int somaId,
                                      const Mat3d& somaInitDir, const VectorVec3d& somaList,
                                      VectorVec5d &resultCurve, Mat2i &resultCurConInfo)
{

    int nxx = origImg.x();
    int nyy = origImg.y();
    int nzz = origImg.z();

    VectorVec3d thickCurve;
    for(size_t i = 0; i < thickNodeList.size();++i){
        thickCurve.push_back(Vec3d(thickNodeList[i](0), thickNodeList[i](1),
                                   thickNodeList[i](2) ));
    }

    resultCurve.clear();
    VectorVec5d rawSomaCurve;
    rawSomaCurve.clear();
    resultCurConInfo.setZero();

    rawSomaCurve.clear();
    Vec3d initPt = thickCurve.back();

    VectorVec5d resultSomaCurve;
    TraceCurveForwardFromSoma(somaInitDir.col(0),
                              initPt, resultIndexImg,backImg, origImg,somaList, resultSomaCurve);

    int nx = int(resultSomaCurve.size());

    //Vec2i Lnots1(-somaId, 0);
    resultCurConInfo.col(0) = Vec2i(-somaId, 0);//Lnots1;
    resultCurConInfo.col(1) = Vec2i(0, 0);

    if (nx > 2 && thickCurve.size() ==1){
        Vec5d tmp;
        tmp.setZero();
        tmp.block(0,0,3,1) = initPt;
        rawSomaCurve.push_back(tmp);
        for (VectorVec5d::size_type n = 0; n < resultSomaCurve.size(); ++n)
            rawSomaCurve.push_back(resultSomaCurve[n]);
    }//if
    //}
    if(thickCurve.size() > 1 && nx > 1){
        Vec5d tmp;
        for(size_t k = 0; k < thickCurve.size();++k){
            tmp << thickCurve[k](0), thickCurve[k](1),thickCurve[k](2),
                   2.5,0.0;
            rawSomaCurve.push_back(tmp);
        }
        for (VectorVec5d::size_type n = 0; n < resultSomaCurve.size(); ++n)
            rawSomaCurve.push_back(resultSomaCurve[n]);
    }

    VectorVec5d resultCurveCopy;//some change compared to matlab
    if (rawSomaCurve.size() > 4){
        double distNorm = (rawSomaCurve[1].block(0,0,3,1)
                - rawSomaCurve[0].block(0,0,3,1)).norm();
        if(distNorm > 3.0){
            Vec5d node1 = rawSomaCurve[0];
            Vec5d node2 = rawSomaCurve[1];
            node1(3) =node2(3)+1.0;
            std::vector<double> origWet, backWet;
            VectorVec3d tmp;
            tmp.push_back(node1.block(0,0,3,1));
            WeighRayValue(tmp, origImg, origWet);
            WeighRayValue(tmp, backImg, backWet);
            node1(4)=origWet[0] - backWet[0];

            //2014-4-20
            VectorVec5d aaKKs;
            for(double ij = 0; ij <=distNorm; ij+=3.0){
                Vec5d tmp;
                tmp = ij/distNorm*node1+(1-ij/distNorm)*node2;
                aaKKs.push_back(tmp);
            }
            VectorVec5d tmpRawSomaCurve;
            std::copy(aaKKs.begin(), aaKKs.end(),
                      std::back_inserter(tmpRawSomaCurve));
            std::copy(rawSomaCurve.begin()+1, rawSomaCurve.end(),
                      std::back_inserter(tmpRawSomaCurve));
            rawSomaCurve.swap(tmpRawSomaCurve);
        }

        int rawSomaCurveNodeSum = rawSomaCurve.size();
        VectorVec3d tmpPtCurve;
        Vec3d tmp;
        for (int i = 0; i < rawSomaCurveNodeSum; ++i){
            tmp(0) = rawSomaCurve[i](0);
            tmp(1) = rawSomaCurve[i](1);
            tmp(2) = rawSomaCurve[i](2);
            tmpPtCurve.push_back(tmp);
        }
        std::vector<double> radius;//here is list
        std::vector<double> rav;
        CalcParmOfCurveNodeList(origImg, backImg, tmpPtCurve, radius, rav);

        for (int i = 0; i < rawSomaCurveNodeSum; ++i){
            rawSomaCurve[i](3) = radius[i];
            rawSomaCurve[i](4) = rav[i];
        }

        Vec5d half;
        //要插值
        for (int i = 0; i < rawSomaCurveNodeSum - 1; ++i){
            resultCurveCopy.push_back(rawSomaCurve[i]);
            half = 0.5 * (rawSomaCurve[i] + rawSomaCurve[i + 1]);
            resultCurveCopy.push_back(half);
        }
        resultCurveCopy.push_back(rawSomaCurve[rawSomaCurveNodeSum - 1]);

        //2014-4-23
        std::vector<double> three;
        for (VectorVec5d::size_type i = 0; i < resultCurveCopy.size(); ++i){
            //three.push_back((double)std::max(3, int(resultCurveCopy[i](3) + 1.0) ) );
            //2014-4-20
            three.push_back(std::min<double>(2.0, double(Round(resultCurveCopy[i](3) + 0.5) ) ) );
        }
        int id1(0), id2(0), id3(0);
        int xMin, xMax, yMin, yMax, zMin, zMax;
        for (int ik = 1; ik < 2*rawSomaCurveNodeSum-2; ++ik){
            id1 = Round(resultCurveCopy[ik](0) );
            id2 = Round(resultCurveCopy[ik](1) );
            id3 = Round(resultCurveCopy[ik](2) );

            xMin = std::max(0, id1 - std::max(0, (int)three[ik]));
            xMax = std::min(nxx - 1, id1 + std::max(0, (int)three[ik]));
            yMin = std::max(0, id2 - std::max(0, (int)three[ik]));
            yMax = std::min(nyy - 1, id2 + std::max(0, (int)three[ik]));
            zMin = std::max(0, id3 - std::max(0, (int)three[ik]));
            zMax = std::min(nzz - 1, id3 + std::max(0, (int)three[ik]));

            for (int ii = xMin; ii <= xMax; ++ii){
                for (int jj = yMin; jj <= yMax; ++jj){
                    for (int kk = zMin; kk <= zMax; ++kk){
                        if (resultIndexImg(ii, jj, kk) == 0)
                            resultIndexImg(ii, jj, kk) = somaId;
                    }
                }
            }//for
        }//for

        VectorVec3d innerSomaPt;
        DVolume smoothRay;
        RayBurstShape(rawSomaCurve[0].block(0,0,3,1),origImg, innerSomaPt,smoothRay);
        int innerSomaPtNum = innerSomaPt.size();
        for(int k = 0; k < innerSomaPtNum;++k){
            if(0 == resultIndexImg(int(innerSomaPt[k](0)), int(innerSomaPt[k](1)),
                int(innerSomaPt[k](2)) ) ){
                resultIndexImg(int(innerSomaPt[k](0)), int(innerSomaPt[k](1)),
                                int(innerSomaPt[k](2)) ) = somaId;
            }
        }
        resultCurve.swap(resultCurveCopy);
    }
    else{
        resultCurve.clear();
        resultCurConInfo.setZero();
    }
}

void TraceFilter::TraceCurveForwardFromSoma(const Vec3d &dendSomaInitDir, const Vec3d &initPt, const Volume<int>& indexImg,
                                               const Volume<unsigned short> &backImg, const Volume<unsigned short> &origImg,
                                            const VectorVec3d& somaList,
                                               VectorVec5d &resultSomaCurve)
{
    resultSomaCurve.clear();
    Vec5d tmp1;
    tmp1 << initPt(0), initPt(1), initPt(2), 0, 0;
    resultSomaCurve.push_back(tmp1);

    Vec2i collideFlag;
    collideFlag.setZero();
    int isNextNodeValid(1);
    int i = 0;

    int nxx = origImg.x();
    int nyy = origImg.y();
    int nzz = origImg.z();

    Vec3d nextDendDir(dendSomaInitDir);
    Vec3d fixInitDendDir(dendSomaInitDir);

	//2015-8-13
	int xExtractRegion, yExtractRegion, zExtractRegion;
	typedef void (TraceFilter::*MyTraceNextCurveNode)( const Volume<unsigned short> &, const Vec3d &,const double &, const Vec3d &,
		Vec3d &, Vec3d &,int & );
	typedef void (TraceFilter::*MyDetectCollision)(const VectorVec5d &, const VectorVec3d &,
		const Vec2i &, const Volume<int> &,
		const Volume<unsigned short> &, const Volume<unsigned short> &,
		const int, const int, const int,
		VectorVec5d &, int &,
		Vec2i &, int &, int& );
	MyTraceNextCurveNode curTraceNextCurveNode;
	MyDetectCollision curDetectCollision;
	if (origImg.XResolution() * origImg.YResolution() * origImg.ZResolution() > 1.5) {
		xExtractRegion = yExtractRegion = 10;
		zExtractRegion = 7;
		curTraceNextCurveNode = &TraceFilter::TraceNextCurveNode;
		curDetectCollision = &TraceFilter::DetectCollision;
	}else{
		xExtractRegion = yExtractRegion = 15;
		zExtractRegion = 10;
		curTraceNextCurveNode = &TraceFilter::TraceNextCurveNodeV2;
		curDetectCollision = &TraceFilter::DetectCollisionV2;
	}

    while(isNextNodeValid == 1 && i < 3000){
        int xMin, xMax, yMin, yMax, zMin, zMax;
        Get3DRegion(xMin, xMax, yMin, yMax, zMin, zMax,
            Round(resultSomaCurve[i](0) ), Round(resultSomaCurve[i](1)), Round(resultSomaCurve[i](2)), xExtractRegion, yExtractRegion, zExtractRegion,
            0, nxx - 1, 0, nyy - 1, 0, nzz - 1);

        Volume<unsigned short> locOrigImg;
        ExtractArea(origImg, xMin, xMax, yMin, yMax, zMin, zMax, locOrigImg);

        if((xMax - xMin) * (yMax - yMin) * (zMax - zMin) == 0){
            isNextNodeValid = 0;
            break;
        }
        Vec3d lefttopAxis(xMin, yMin, zMin);
        //2014-4-22
        int rdrvx = std::min(std::max(Round(resultSomaCurve[i](0) - 1.0 ), 0), nxx - 1);
        int rdrvy = std::min(std::max(Round(resultSomaCurve[i](1) - 1.0 ), 0), nyy - 1);
        int rdrvz = std::min(std::max(Round(resultSomaCurve[i](2) - 1.0 ), 0), nzz - 1);
        double threv = backImg(rdrvx, rdrvy, rdrvz);
        Vec3d tmpCurrentCurveNode(resultSomaCurve[i](0) - xMin, resultSomaCurve[i](1) - yMin, resultSomaCurve[i](2) - zMin);

        Vec3d nextCurveNode;
        nextCurveNode.setZero();
        Vec3d nextDenDirCopy;
        nextDenDirCopy.setZero();
        Vec3d tmpx1;
		//2015-8-13
        (this->*curTraceNextCurveNode)(locOrigImg, tmpCurrentCurveNode, threv, nextDendDir,nextCurveNode, tmpx1,
            isNextNodeValid);
		/*TraceNextCurveNode(locOrigImg, tmpCurrentCurveNode, threv, nextDendDir,nextCurveNode, tmpx1,
			isNextNodeValid);*/
        nextDendDir = tmpx1;
        nextDenDirCopy = nextDendDir;

        if ( nextCurveNode(0) != 0.0 && nextCurveNode(1) != 0.0 && nextCurveNode(2) != 0.0 )
            nextCurveNode += lefttopAxis;

        if (isNextNodeValid == 1){
            // in fact it is not nextCurveNode but corrected current node according to forward direction.
            resultSomaCurve[i](0) = nextCurveNode(0);
            resultSomaCurve[i](1) = nextCurveNode(1);
            resultSomaCurve[i](2) = nextCurveNode(2);

            double radius(0.0);
            double rav(0.0);

            CalcParmOfCurveNode(origImg, backImg, nextCurveNode, radius, rav);
            resultSomaCurve[i](3) = radius;
            if (radius > 1.0){
                ++i;//warning!!
                Vec3d rayNode2 = nextCurveNode + 2.0 * nextDenDirCopy;
                Vec3d rayNode1 = nextCurveNode + nextDenDirCopy;
                VectorVec3d tmpRayNode;
                tmpRayNode.push_back(rayNode1);
                tmpRayNode.push_back(rayNode2);
                std::vector<double> rayValue;
                WeighRayValue(tmpRayNode, origImg, rayValue);

                if ( rayValue[0] > rayValue[1] + 3.5 * std::sqrt(threv) ){
                    Vec3d tmpVec = nextCurveNode + nextDenDirCopy;
                    Vec5d tmp; tmp.setZero();
                    tmp(0) = tmpVec(0);
                    tmp(1) = tmpVec(1);
                    tmp(2) = tmpVec(2);
                    tmp(4) = rayValue[0];
                    resultSomaCurve.push_back(tmp);
                }
                else{
                    Vec3d tmpVec = nextCurveNode + 2.0 * nextDenDirCopy;
                    Vec5d tmp; tmp.setZero();
                    tmp(0) = tmpVec(0);
                    tmp(1) = tmpVec(1);
                    tmp(2) = tmpVec(2);
                    tmp(4) = rayValue[1];
                    resultSomaCurve.push_back(tmp);
                }//if
            }
            else isNextNodeValid = 0;
        }

        Vec3d tmpCurCurveNode(resultSomaCurve[i](0), resultSomaCurve[i](1), resultSomaCurve[i](2));
        if ( tmpCurCurveNode.minCoeff() < -1.0 || tmpCurCurveNode(0) > nxx
            || tmpCurCurveNode(1) > nyy
            || tmpCurCurveNode(2) > nzz  ) {
            isNextNodeValid = 0;
        }

        if (i > 3){//matlab is 4
            VectorVec5d modifiedCurve;
            int isNextNodeValidCp;
            Vec2i collideFlagCp;
            int iCp;
            int C_infor;
            DetectCollision(resultSomaCurve, somaList, collideFlag, indexImg, origImg,backImg,
                            i,1,isNextNodeValid,modifiedCurve,isNextNodeValidCp,
                            collideFlagCp, iCp, C_infor);
            resultSomaCurve.swap(modifiedCurve);
            isNextNodeValid = isNextNodeValidCp;
            collideFlag=collideFlagCp;
            i = iCp;
            Vec3d nearNodeDist;
            nearNodeDist.setZero();
            for (int iij = 1; iij < 4; ++iij){
                Vec3d tmp11(resultSomaCurve[i - iij](0), resultSomaCurve[i - iij](1), resultSomaCurve[i - iij](2));
                Vec3d tmp22(resultSomaCurve[i - iij + 1](0), resultSomaCurve[i - iij + 1](1), resultSomaCurve[i - iij + 1](2));
                nearNodeDist(iij - 1) = (tmp22 - tmp11).norm();
            }

            VectorVec3d dataPrincipald;
            Vec3d tmpDataPrin;
            for ( int n = std::max(i - 10, 0) ; n <= i - 2; ++n ){
                tmpDataPrin(0) = resultSomaCurve[n](0);
                tmpDataPrin(1) = resultSomaCurve[n](1);
                tmpDataPrin(2) = resultSomaCurve[n](2);
                dataPrincipald.push_back(tmpDataPrin);
            }

            Vec3d sx1;//, sx2, sx3;
            CalcPtCurveDirection(dataPrincipald, sx1);
            nextDendDir = sx1;
            //x2 = sx2;
            //x3 = sx3;

            Vec3d forwardAngle(sx1);
            Vec3d dataLLi, dataLLi_1;
            dataLLi(0) = resultSomaCurve[i](0);
            dataLLi(1) = resultSomaCurve[i](1);
            dataLLi(2) = resultSomaCurve[i](2);

            dataLLi_1(0) = resultSomaCurve[i - 1](0);
            dataLLi_1(1) = resultSomaCurve[i - 1](1);
            dataLLi_1(2) = resultSomaCurve[i - 1](2);

            Vec3d backwardAngle = dataLLi - dataLLi_1;

            double crossAngle = forwardAngle.transpose() * backwardAngle;
            crossAngle /=  forwardAngle.norm() * backwardAngle.norm();

            if (nearNodeDist.minCoeff() < 0.25 || crossAngle < 0.65 )
                isNextNodeValid = 0;
        }
        if(i < 4){
            nextDendDir=fixInitDendDir;
            nextDendDir.normalize();
        }
    }
}

void TraceFilter::ReconstructShapeForTrace(const Vec3d &intialPoint, const Volume<unsigned short> &origImg,
                                              //const Volume<unsigned short> &backImg,
                                              //Vec3d &dendSomaInitDirection, /*Vec3d &xx2, Vec3d &xx3,*/
                                           MatXd& resultRayLength,
                                           //Vec3d &globalDendInitCenter,
                                           VectorVec3d &innerSomaPts)
{
    innerSomaPts.clear();
    Volume<unsigned short> locOrigImg;
    Vec3d locPoint;
    const int Theta = 40;
    const int Phi = 20;
    ExtractLocalDomain(intialPoint, origImg, locOrigImg, locPoint);//为了L_XX3

    double slice = 0.5;//(double)(minLen) / 82.0;
    const int blocksum = 41;//41;

    std::vector<double> raySlice;
    //generate_n(back_inserter(r0), blocksum, GenArray<double>(0.0, slice));//41个
    for(int i = 0; i < blocksum; ++i){
        raySlice.push_back(slice * double(i));
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

    for (std::vector<double>::size_type k = 0; k < raySlice.size(); ++k){
        for (int i = 1; i <= Theta; ++i){
            for (int j = 1; j <= Phi; ++j){
                x = raySlice[k] * std::sin(b * (double)j * PI_180) * std::cos(a * (double)i * PI_180) + locPoint(0);
                y = raySlice[k] * std::sin(b * (double)j * PI_180) * std::sin(a * (double)i * PI_180) + locPoint(1);
                z = raySlice[k] * std::cos(b * (double)j * PI_180) + locPoint(2);
                rayNodeDense = rayNodeWet = 0.0;

                ContourUtil::CalculateSphereOneNode(locOrigImg, 1.0, x, y, z, rayNodeDense, rayNodeWet);

                sphereRayWet(k, i-1, j-1) = (double)(rayNodeDense / (rayNodeWet + 0.0001));
            }
        }
    }//for

    Volume<double> sphereRayGradient;

    TraceUtil::GetGradientVectorFlowForTrace(sphereRayWet, sphereRayGradient);

    double gradThrev(0.0);//求Uz最大值
    GetAreaMaxValue(sphereRayGradient, 0,  sphereRayGradient.x() - 1, 0, sphereRayGradient.y() - 1, 0, sphereRayGradient.z() - 1, gradThrev);
    gradThrev *= 0.1;

    resultRayLength = 5.0 * MatXd::Ones(Theta + 2, Phi + 2);//射线长度

    std::vector<double> raySliceIndex;//分段长度
    //generate_n(back_inserter(idexxS), blocksum, GenArray<double>(1.0, 1.0));//注意这里要同步
    for(int i = 0; i < blocksum; ++i){
        raySliceIndex.push_back(1.0 + double(i));
    }

    double currentRayLength(0);
    std::vector<double> currentRayGrad(sphereRayGradient.x());
    std::vector<double> rayGradWet(sphereRayGradient.x());

    double pullParm[2];
    pullParm[0] = 0.8;
    pullParm[1] = 1.0 - pullParm[0];

    int repeat = 50;
    double rayGradWetSum;

    for (int jj = 0; jj < repeat;++jj){
        for (int i = 1; i <= Theta; ++i){
            for (int j = 1; j <= Phi; ++j){
                currentRayLength = resultRayLength(i, j);
                for (int ij = 0; ij < sphereRayGradient.x(); ++ij){
                    currentRayGrad[ij] = sphereRayGradient(ij, i - 1, j - 1);
                    rayGradWet[ij] = currentRayGrad[ij]
                            * std::exp( -0.05 *std::pow( raySliceIndex[ij] - currentRayLength, 2.0));
                }
                //rayGradWetSum = std::accumulate(rayGradWet.begin(), rayGradWet.end(), 0.0);
                rayGradWetSum = 0.0;
                for(std::vector<double>::size_type k = 0; k < rayGradWet.size(); ++k){
                    rayGradWetSum += rayGradWet[k];
                }
                //if (std::abs(rayGradWetSum) > 0.001){
                resultRayLength(i,j) += pullParm[0] * ( ( (std::inner_product(raySliceIndex.begin(), raySliceIndex.end(), rayGradWet.begin(), 0.0)) /
                                                          (rayGradWetSum+0.001 ) ) - resultRayLength(i,j) )
                        - pullParm[1] * (4.0 * resultRayLength(i,j)
                                         - resultRayLength(i-1,j) - resultRayLength(i+1,j) - resultRayLength(i,j-1) - resultRayLength(i,j+1));
                //}

            }
            resultRayLength = (resultRayLength.array() > 0.0).select(resultRayLength, 0.0);
            resultRayLength.row(0) = resultRayLength.row(2);
            resultRayLength.row(Theta + 1) = resultRayLength.row(Theta - 1);
            resultRayLength.col(0) = resultRayLength.col(2);
            resultRayLength.col(Phi + 1) = resultRayLength.col(Phi - 1);
        }
    }//for

    for (int jj = 0; jj < repeat;++jj){
        for (int i = 1; i <= Theta; ++i){
            for (int j = 1; j <= Phi; ++j){
                currentRayLength = resultRayLength(i, j);
                for (int ij = 0; ij < sphereRayGradient.x(); ++ij){
                    currentRayGrad[ij] = sphereRayGradient(ij, i - 1, j - 1);
                    rayGradWet[ij] = currentRayGrad[ij] * std::exp( -0.05 * std::pow( raySliceIndex[ij] - currentRayLength, 2.0));
                }

                //rayGradWetSum = std::accumulate(rayGradWet.begin(), rayGradWet.end(), 0.0);
                rayGradWetSum = 0.0;
                for(std::vector<double>::size_type k = 0; k < rayGradWet.size(); ++k){
                    rayGradWetSum += rayGradWet[k];
                }

                //if (std::abs(rayGradWetSum) > 0.001){
                    if ( sphereRayGradient( (std::min)(Round(currentRayLength + 0.5) -1, (int)raySlice.size() - 1), i - 1, j - 1 ) >  gradThrev ){
                        resultRayLength(i,j) += pullParm[0] * ( ( (inner_product(raySliceIndex.begin(), raySliceIndex.end(), rayGradWet.begin(), 0.0)) /
                            (rayGradWetSum+0.001 ) ) - resultRayLength(i,j) )
                            - pullParm[1] * (4.0 * resultRayLength(i,j) - resultRayLength(i-1,j) - resultRayLength(i+1,j) - resultRayLength(i,j-1) - resultRayLength(i,j+1));
                    }
                    else{
                        resultRayLength(i,j) += (-pullParm[1]) * (4.0 * resultRayLength(i,j) - resultRayLength(i-1,j) - resultRayLength(i+1,j) - resultRayLength(i,j-1) - resultRayLength(i,j+1));
                    }
                //}
            }
            resultRayLength = (resultRayLength.array() > 0.0).select(resultRayLength, 0.0);
            resultRayLength.row(0) = resultRayLength.row(2);
            resultRayLength.row(Theta + 1) = resultRayLength.row(Theta - 1);
            resultRayLength.col(0) = resultRayLength.col(2);
            resultRayLength.col(Phi + 1) = resultRayLength.col(Phi - 1);
        }
    }//for

    double xx(0.0), yy(0.0), zz(0.0);
    Vec3d tmp;
    //double k(0.0);

    for (int i = 2; i <= Theta + 1;++i){
        for (int j = 2; j <= Phi  +1; ++j){
            if (resultRayLength(i-1, j - 1)  > 0.0){
                for (double k = 1.0; k <= 0.5 * resultRayLength(i - 1, j - 1); ++k){//warning!!
                    xx = (k * std::sin(b * (double)(j-1) * PI_180) * std::cos(a * (double)(i-1) * PI_180) + locPoint(0));
                    yy = (k * std::sin(b * (double)(j-1) * PI_180) * std::sin(a * (double)(i-1) * PI_180) + locPoint(1));
                    zz = (k * std::cos(b * (double)(j-1) * PI_180) + locPoint(2));
                    //2014-4-22
                    xx -= (locPoint(0)-intialPoint(0));
                    yy -= (locPoint(1)-intialPoint(1));
                    zz -= (locPoint(2)-intialPoint(2));
                    tmp(0) = Round(xx );
                    tmp(1) = Round(yy );
                    tmp(2) = Round(zz );
                    innerSomaPts.push_back(tmp);//轮廓内的区域也要
                }
            }
        }
    }

    //innerSomaPts.swap(VectorVec3d(innerSomaPts));
    /*去除重复*/
    //lambda function
#ifdef _linux
    std::sort(innerSomaPts.begin(), innerSomaPts.end(), [](const Vec3d& lhs, const Vec3d &rhs){
        if (lhs(0) != rhs(0))  return lhs(0) < rhs(0);
        else if (lhs(1) != rhs(1))  return lhs(1) < rhs(1);
        else if (lhs(2) != rhs(2))  return lhs(2) < rhs(2);
        return false;
    });
#else
    std::sort(innerSomaPts.begin(), innerSomaPts.end(), Vec3d_less());

#endif
    innerSomaPts.erase(std::unique(innerSomaPts.begin(), innerSomaPts.end()), innerSomaPts.end());
    //2014-4-19
    VectorVec3d tmpInnder;
    double nxx = origImg.x();
    double nyy = origImg.y();
    double nzz = origImg.z();
    for(size_t i = 0; i < innerSomaPts.size(); ++i){
        Vec3d& currentSomaPt = innerSomaPts[i];
        if( currentSomaPt.minCoeff() > -1.0 && currentSomaPt(0) < double(nxx)
                && currentSomaPt(1) < double(nyy)
                && currentSomaPt(2) < double(nzz) ){
            tmpInnder.push_back(currentSomaPt);
        }
    }
    innerSomaPts.swap(tmpInnder);
}

void TraceFilter::InflateBoundary(const VectorVec3d &innerSomaPts, const Volume<unsigned short> &locOrigImg,
                                  const double inflateThrev,
                                  VectorVec3d &inflatedArea)
{
    int nx = locOrigImg.x();
    int ny = locOrigImg.y();
    int nz = locOrigImg.z();

    inflatedArea.clear();

    VectorVec3d::size_type innerSomaPtNum = innerSomaPts.size();
    //Ima = XX;
    Volume<unsigned short> locIndexImg;
    //Ima.SetSize(XX.x(), XX.y(), XX.z());
    //SafeCopyVoxel(XX, Ima);
    locIndexImg.QuickCopy(locOrigImg);

    for (VectorVec3d::size_type i = 0; i < innerSomaPtNum; ++i){
        if (innerSomaPts[i](0) < nx - 1 && innerSomaPts[i](1) < ny - 1 && innerSomaPts[i](2) < nz - 1
            ){//&& innerSomaPts[i](0) > -1 && innerSomaPts[i](1) > -1 && innerSomaPts[i](2) > -1

            locIndexImg(std::max<int>((int)innerSomaPts[i](0),0),
                                      std::max<int>((int)innerSomaPts[i](1),0),
                                                    std::max<int>((int)innerSomaPts[i](2),0)) = 1025;//标记
        }
    }

    VectorVec3d boundaryPts;
    std::vector<double> boundaryPtsValue;

    Vec3d currentSomaPt;
    for (VectorVec3d::size_type i = 0; i < innerSomaPtNum; ++i){
        currentSomaPt = innerSomaPts[i];
        int xMin, xMax, yMin, yMax, zMin, zMax;

        Get3DRegion(xMin, xMax, yMin, yMax, zMin, zMax, Round(currentSomaPt(0)), Round(currentSomaPt(1)), Round(currentSomaPt(2)),
                    1, 1, 1, 0, nx - 1, 0, ny - 1, 0, nz - 1);
        //in matlab code there is MM/ptWindow,but C++ donot need it.
        unsigned short windowMinVal(10000);
        GetAreaMinValue(locIndexImg, xMin, xMax, yMin, yMax, zMin, zMax, windowMinVal);

        if (windowMinVal < 1025 && currentSomaPt(0) < nx - 1 && currentSomaPt(1) < ny - 1
                && currentSomaPt(2) < nz - 1 && currentSomaPt.minCoeff() > -0.5){//here matlab is 0.5, but C++ is -0.5
            int xLen = xMax - xMin + 1;
            int yLen = yMax - yMin + 1;
            int zLen = zMax - zMin + 1;

            std::vector<double> windowValList;

            for (int itt1 = 0; itt1 < xLen; ++itt1){
                for (int itt2 = 0; itt2 < yLen; ++itt2){
                    for (int itt3 = 0; itt3 < zLen; ++itt3)
                        windowValList.push_back(locIndexImg(xMin + itt1, yMin + itt2, zMin + itt3));
                }
            }//for

            std::sort(windowValList.begin(), windowValList.end());
            //int n = (int)round<double>((double)(nt1 * nt2 * nt3) / 2.0 - 0.5);
            //warning here different from matlab 2014-5-6
            if (windowValList[Round((double)(xLen * yLen * zLen) / 2.0 -0.5)] < 1025){
                boundaryPts.push_back(currentSomaPt);
                boundaryPtsValue.push_back(locOrigImg((int)currentSomaPt(0), (int)currentSomaPt(1), (int)currentSomaPt(2)));
            }
        }
    }//for

    //double thre1 = std::accumulate(boundaryPtsValue.begin(), boundaryPtsValue.end(), 0.0);
    double thre1 = 0.0;
    for(std::vector<double>::size_type i = 0; i < boundaryPtsValue.size(); ++i){
        thre1 += boundaryPtsValue[i];
    }
    thre1 /= double(boundaryPtsValue.size());
    double thre = std::min(std::max(thre1, inflateThrev), 500.0);
    int mm(0);
    VectorVec3d recursivePts1 (boundaryPts);
    inflatedArea = recursivePts1;
    VectorVec3d recursivePts2;
    recursivePts2.push_back(Vec3d(0,0,0));

    while (!recursivePts2.empty() && mm < 10){
        recursivePts2.clear();
        ++mm;
        InflateMarginalAreaAboveThrev(recursivePts1, locIndexImg, thre,
            nx, ny, nz, recursivePts2);
        int sz = inflatedArea.size();
        inflatedArea.resize(sz + recursivePts2.size());
        std::copy(recursivePts2.begin(), recursivePts2.end(), inflatedArea.begin() + sz);
        recursivePts1 = recursivePts2;
    }
}

void TraceFilter::InflateMarginalAreaAboveThrev(const VectorVec3d &initPoints, Volume<unsigned short> &locIndexImg,
                                                const double thre,
                                  const int nx, const int ny, const int nz,
                                                VectorVec3d &resultInflatedArea)
{
    int nxx = initPoints.size();
    resultInflatedArea.clear();

    unsigned short mm;
    Vec3d dist;
    Vec3d tmp;
    for ( int i = 0; i < nxx; ++i){
        int xMin, xMax, yMin, yMax, zMin, zMax;

        Get3DRegion(xMin, xMax, yMin, yMax, zMin, zMax, Round(initPoints[i](0) ), Round(initPoints[i](1) ), Round(initPoints[i](2) ),
                    1, 1, 1, 0, nx - 1, 0, ny - 1, 0, nz - 1);

        int xLen = xMax - xMin + 1;
        int yLen = yMax - yMin + 1;
        int zLen = zMax - zMin + 1;

        for (int ii = 0; ii < xLen; ++ii){
            for (int jj = 0; jj < yLen; ++jj){
                for (int ij = 0; ij < zLen; ++ij){
                    mm = locIndexImg(xMin + ii, yMin + jj, zMin + ij);
                    dist = Vec3d(xMin + ii, yMin + jj, zMin + ij) - initPoints[i];
                    if (mm < 1025 && dist.norm() < 1.1 && mm > thre){
                        tmp(0) = xMin + ii;
                        tmp(1) = yMin + jj;
                        tmp(2) = zMin + ij;
                        resultInflatedArea.push_back(tmp);
                        locIndexImg(xMin + ii, yMin + jj, zMin + ij) = 1025;
                    }
                }
            }
        }
    }
}

void TraceFilter::DetectCellSignalModify(const Volume<NGCHAR> &locIndexImg, const VectorVec3d &inflatedPoints,
                                         const int threv, VectorVec3d &cxDomainSet, std::vector<int> &cxDomainNumList)
{
    ///------------------初始化---------------------------------------------------------------
    //int n, i, j ,ij ;
    int xMin,xMax;
    int yMin,yMax;
    int zMin,zMax;

    cxDomainSet.clear();
    cxDomainNumList.clear();

    int ax		= locIndexImg.x();
    int ay		= locIndexImg.y();
    int az		= locIndexImg.z();
    //int aWH	= ax * ay;

    Volume<NGCHAR> tmpLocIndexImg;//(XX1.x(), XX1.y(), XX1.z());
    //QuickCopyVoxel(XX1, Ima);
    tmpLocIndexImg.QuickCopy(locIndexImg);
    //memcpy(Ima.Data(), XX1.Data(), sizeof(UChar) * XX1.x() * XX1.y() * XX1.z());

    //Vec3d pt;
    int nxx = inflatedPoints.size();
    int locWinWet(0);
    ///----------------开始-----------------
    for (int n = 0; n < nxx; ++n){

        const Vec3d& pt = inflatedPoints[n];

        if (pt(0) > 1 && pt(0) < ax-1 && pt(1) > 1 && pt(1) < ay-1 && pt(2) > 1 && pt(2) < az-1)
            //if id1>2 & id1<ax & id2>2 & id2<ay & id3>2 & id3<az ///
        {
            Get3DRegion(xMin, xMax, yMin, yMax, zMin, zMax, Round(pt(0)), Round(pt(1) )
                        , Round(pt(2) ), 1, 1, 1);

            locWinWet = 0;
            GetAreaSum(tmpLocIndexImg, xMin, xMax, yMin, yMax, zMin, zMax, locWinWet);
            locWinWet /= 255;
        }
        else locWinWet = 0;

        VectorVec3d resultPoint;
        resultPoint.clear();//一定要注意
        ///-----------------提取-----------------------
        if (locWinWet > threv){
            /*提取区域*/
            TraceUtil::ExtractSubRegionOP(pt, tmpLocIndexImg, resultPoint);
            /*不判断是否是连通域*/
            VectorVec3d::size_type sz = cxDomainSet.size();
            cxDomainSet.resize(sz + resultPoint.size());
            std::copy(resultPoint.begin(), resultPoint.end(), cxDomainSet.begin() + sz);
            cxDomainNumList.push_back(resultPoint.size());
            resultPoint.clear();
        }//if
    }//for
}

void TraceFilter::CalcOrthoBasis(const Vec3d &vec1, Vec3d &vec2, Vec3d &vec3)
{
    vec2 = vec1;
    vec3.setZero();
    std::vector<std::pair<int, double> > tmp;
    tmp.push_back(std::pair<int, double>(0, std::abs(vec1(0))));
    tmp.push_back(std::pair<int, double>(1, std::abs(vec1(1))));
    tmp.push_back(std::pair<int, double>(2, std::abs(vec1(2))));
    /*[idxv,idexx]=sort(abs(vec1))*/
#ifdef __linux
    std::sort(tmp.begin(), tmp.end(),[](const std::pair<int, double>& lhs, const std::pair<int, double>& rhs){
        return lhs.second < rhs.second;
    });
#else
    std::sort(tmp.begin(), tmp.end(),Pair_less());

#endif
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

void TraceFilter::CalcConstraintPCA(const VectorVec3d &neighborPtSet, const Vec3d &curSeedNode, const Mat3d &convMat,
                           const std::vector<double> &neighborWet,
                           double &P, Mat3d &sigmaH, Vec3d &mdata3)
{
    Vec3d g;//g
    Mat3d H;//H就是HH
    g.setZero();
    H.setZero();

    VectorVec3d::size_type length = neighborPtSet.size();//nx
    Vec3d X(curSeedNode(0), curSeedNode(1), curSeedNode(2)); //X
    std::vector<double> C;//C
    VectorVec3d U;//U

    Mat3d MMs;//MMs
    MMs.setZero();
    Vec3d MMs1;//MMs1
    MMs1.setZero();
    Vec3d dd(0.0, 0.0, 0.0);//dd
    //vec3d tmpCenter;//data1的一个元素
    for (VectorVec3d::size_type i = 0; i < length; ++i){
        dd = X - neighborPtSet[i];//dd=X-data1(:,i)
        dd(2) *= 2.0;
        if (dd.norm() < 3.5){
            C.push_back(neighborWet[i] * std::exp(-0.5 * dd.transpose() * convMat * dd));
        }
        else{
            C.push_back(0.0);//C(:,i)
        }

        U.push_back(convMat * dd);//U(:,i) = data2 *dd
        g += C[i] * U[i];//g
        H += C[i] * (U[i] * U[i].transpose() - convMat);//HH
        MMs += C[i] * convMat;//MMs
        MMs1 += (C[i] * convMat) * dd;//MMs1
    }

    //P = std::accumulate(C.begin(), C.end(), 0.0);//P = sum(C)
    P = 0.0;
    for(std::vector<double>::size_type i = 0; i < C.size(); ++i){
        P += C[i];
    }
    g *= -1.0;

    /*sigmaH = -H./P + (P^(-2)) * g * g'*/
    sigmaH = - H / P + std::pow(P, -2.0) * (g * g.transpose());
    if (std::abs(MMs.determinant()) > 0.1){
        Mat3d tmpcorrection = MMs.inverse();
        mdata3 = tmpcorrection * MMs1;//inv(MMs)*MMs1
    } else{
        mdata3 = MMs1;
    }
}

void TraceFilter::CalcConstraintPCAv2(const VectorVec3d &neighborPtSet, const Vec3d &curSeedNode, const Mat3d &convMat,
									const std::vector<double> &neighborWet,
									double &P, Mat3d &sigmaH, Vec3d &mdata3)
{
	Vec3d g;//g
	Mat3d H;//H就是HH
	g.setZero();
	H.setZero();

	VectorVec3d::size_type length = neighborPtSet.size();//nx
	Vec3d X(curSeedNode(0), curSeedNode(1), curSeedNode(2)); //X
	std::vector<double> C;//C
	VectorVec3d U;//U

	Mat3d MMs;//MMs
	MMs.setZero();
	Vec3d MMs1;//MMs1
	MMs1.setZero();
	Vec3d dd(0.0, 0.0, 0.0);//dd
	//vec3d tmpCenter;//data1的一个元素
	for (VectorVec3d::size_type i = 0; i < length; ++i){
		dd = X - neighborPtSet[i];//dd=X-data1(:,i)
		//dd(2) *= 2.0;
		if (dd.norm() < 6.0){
			C.push_back(neighborWet[i] * std::exp(-0.5 * dd.transpose() * convMat * dd));
		}
		else{
			C.push_back(0.0);//C(:,i)
		}

		U.push_back(convMat * dd);//U(:,i) = data2 *dd
		g += C[i] * U[i];//g
		H += C[i] * (U[i] * U[i].transpose() - convMat);//HH
		MMs += C[i] * convMat;//MMs
		MMs1 += (C[i] * convMat) * dd;//MMs1
	}

	//P = std::accumulate(C.begin(), C.end(), 0.0);//P = sum(C)
	P = 0.0;
	for(std::vector<double>::size_type i = 0; i < C.size(); ++i){
		P += C[i];
	}
	g *= -1.0;

	/*sigmaH = -H./P + (P^(-2)) * g * g'*/
	sigmaH = - H / P + std::pow(P, -2.0) * (g * g.transpose());
	if (std::abs(MMs.determinant()) > 0.1){
		Mat3d tmpcorrection = MMs.inverse();
		mdata3 = tmpcorrection * MMs1;//inv(MMs)*MMs1
	} else{
		mdata3 = MMs1;
	}
}

void TraceFilter::CalcPCADirections(const Mat3d &sigmaH, const Vec3d &initVec, const Vec3d &T2, const double threv,
                                    Vec3d &vec1)
{
    Vec3d vec0 = initVec;//x0 = T1
    for (int i = 1; i < 16; ++i){
        CalcPCAMainDirection(vec0, sigmaH, initVec, T2, threv, vec1);//gproPCAA
        vec0 = vec1;
        double angleDiff = vec0.transpose() * initVec;
        if (angleDiff < 0.7) break;
    }
    //directionc(vec1, vec2, vec3);//
}

void TraceFilter::CalcPCAMainDirection(const Vec3d &x0, const Mat3d &sigmaH,
                                       const Vec3d &T1, const Vec3d &T2, const double threv, Vec3d &x1)
{
    double init = 0.5;//a
    Vec3d g = sigmaH * x0;//g
    Mat3d A;
    A.col(0) = x0; A.col(1) = -T1; A.col(2) = -T2;

    Mat3d H;
    H.setZero();

    H(0, 0) = x0.transpose() * x0 - 1.0;//x0' * x0 - 1
    H(1, 1) = - T1.transpose() * x0 + threv;
    H(2, 2) = - T2.transpose() * x0 + threv;

    Mat3d ss1 = (A.transpose() * A - H);
    Mat3d ss = ss1.inverse();
    Mat3d B = ss * A.transpose();
    Vec3d U = B * g;
    Mat3d P = Mat3d::Identity() - (A * ss) * A.transpose();
    if ( ((P * g).cwiseAbs()).sum() <0.001 &&
        U.minCoeff()  > -0.0001){
        x1 = x0;
    }
    else{
        Vec3d v; v.setZero();
        for (int n = 0; n < 3; ++n){//原来是2
            if (U(n) < -0.0001) {
                v(n) = U(n);
            }
            else{
                v(n) = -H(n, n);
            }
        }//for

        Vec3d S = P * g + B.transpose() * v;
        /*p1 = (1-a) * g' * S / (abs(U' * ones(2,1)) + 1)*/
        double tp1 = (1.0 - init) * g.transpose() * S;
        double tp2 = std::abs(U.transpose() * Vec3d::Ones()) + 1.0;
        double p1 = tp1 / tp2;
        Vec3d d = S - p1 * (B.transpose() * Vec3d::Ones());
        d /= d.cwiseAbs().maxCoeff();

        int i = 1;
        for (i = 1; i <= 20; ++i){
            x1 = x0 + pow(0.5, (double)i) * d;
            x1.normalize();
            double tLs1 = x1.transpose() * sigmaH * x1;
            double tLs2 =  x0.transpose() * sigmaH * x0;
            double Ls = tLs1 - tLs2;
            double tdkkss1 = -T1.transpose() * x1;
            tdkkss1 /= x1.norm();
            double dkkss1 = tdkkss1 + threv + 0.001;
            double dkkss2 = pow(0.5, double(i+1)) * g.transpose() * d;
            double tdkkss3 = -T2.transpose() * x1;
            tdkkss3 /= x1.norm();
            double dkkss3 = tdkkss3 + threv + 0.001;
            if (Ls > dkkss2 && x1.norm() < 1.0001 && x1.norm() > 0.999 && dkkss1 < 0.0 &&
                 dkkss3 < 0.0) {
                break;
            }
        }
        if ( i == 20)  x1 = x0;
    }//else
}

void TraceFilter::CalcParmOfCurveNode(const Volume<unsigned short> &origImg, const Volume<unsigned short> &backImg,
                                      const Vec3d &curNode, double &radius, double &wet)
{
    int vx = backImg.x();
    int vy = backImg.y();
    int vz = backImg.z();

    int xMin, xMax, yMin, yMax, zMin, zMax;
	//2015-8-13
	if (origImg.XResolution() * origImg.YResolution() * origImg.ZResolution() > 1.5) {
		Get3DRegion(xMin, xMax, yMin, yMax, zMin, zMax, Round(curNode(0) ), Round(curNode(1) ), Round(curNode(2) ),
			5, 5, 5, 0, vx - 1, 0, vy - 1, 0, vz - 1);
	}else{
		Get3DRegion(xMin, xMax, yMin, yMax, zMin, zMax, Round(curNode(0) ), Round(curNode(1) ), Round(curNode(2) ),
			8, 8, 8, 0, vx - 1, 0, vy - 1, 0, vz - 1);
	}
    

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

	//2015-8-13
	Vec4d level;
	if (origImg.XResolution() * origImg.YResolution() * origImg.ZResolution() > 1.5) {
		level << 1.0, 2.0, 3.0, 4.0;
	}else{
		level << 1.5, 3.0, 4.5, 6.0;
	}

    for (double i = 0; i < xLen; ++i){
        for (double j = 0; j < yLen; ++j){
            for (double ij = 0; ij < zLen; ++ij){
                Vec3d dist(i - center(0), j - center(1), ij - center(2));
                double distNorm = dist.norm();
                if (distNorm <= level(0)){ //距离中心为1
                    distWet(0, 0) += 1.0;
                    if (procImg(i, j, ij) > 0.0){
                        vv0 += procImg(i, j, ij);
                        distWet(1, 0) += 1.0;
                    }
                }//if

                if (distNorm <= level(1)){ //距离中心为2
                    distWet(0, 1) += 1.0;
                    if (procImg(i, j, ij) > 0){
                        vv1 += procImg(i, j, ij);
                        distWet(1, 1) += 1.0;
                    }
                }

                if (distNorm <= level(2)){ //距离中心为3
                    distWet(0, 2) += 1.0;
                    if (procImg(i, j, ij) > 0)
                        distWet(1, 2) += 1.0;
                }

                if (distNorm <= level(3)){ //距离中心为4
                    distWet(0, 3) += 1.0;
                    if (procImg(i, j, ij) > 0)
                        distWet(1, 3) += 1.0;
                }
            }//for
        }
    }//for

	//2015-8-13
    Vec4d procDistWet = distWet.row(1).array() / ( distWet.row(0).array() + 0.0001);
	if (origImg.XResolution() * origImg.YResolution() * origImg.ZResolution() > 1.5) {
		for (int i = 1; i <= 4; ++i){
			if (procDistWet(4-i) > 0.5){
				radius = 5.0 - (double)i;
				break;
			}
		}
	}else{
		for (int i = 1; i <= 4; ++i){
			if (procDistWet(4-i) > 0.5){
				radius = (5.0 - (double)i) * 1.5;
				break;
			}
		}
	}

    if (radius > 2.1){//注意这里！！
		if (origImg.XResolution() * origImg.YResolution() * origImg.ZResolution() > 1.5) {
			radius = radius / std::pow(distWet(0, int(radius + 0.5) - 1) / distWet(1, int(radius + 0.5) - 1), 1.0/3.0);
		}else{
			radius = radius / std::pow(distWet(0, int(radius/1.5 + 0.5) - 1) / distWet(1, int(radius / 1.5 + 0.5) - 1), 1.0/3.0);
		}
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

void TraceFilter::CalcParmOfCurveNodeList(const Volume<unsigned short> &origImg, const Volume<unsigned short> &backImg,
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

void TraceFilter::CalcPtCurveDirection(const VectorVec3d &ptCurve, Vec3d &fir)
{
    int nx = ptCurve.size();
    Vec3d tmpFir;
    tmpFir.setZero();
    double tmp(0.0);
    Vec3d tmpVec;
    for (int i = 0; i < nx - 1; ++i){
        tmpVec = ptCurve[i + 1] - ptCurve[i];
        tmp = tmpVec.norm();
        tmpFir += tmp * tmpVec;
    }
    fir = tmpFir.normalized();
    //CalcOrthoBasis(fir, x2, x3);
}

void TraceFilter::TraceNextCurveNode(const Volume<unsigned short> &locOrigImg, const Vec3d &curSeedNode,
                                         const double &threv, const Vec3d &initDir,
                                         Vec3d &nextCurveNode, Vec3d &nextDenDir,  int &isNextNodeValid)
{
    nextCurveNode.setZero();
    isNextNodeValid = 1;
    nextDenDir = initDir;

    VectorVec3d neighborPtSet;//
    std::vector<double> neighborWet;//W

    Vec3d x10;//x10
    x10.setZero();
    Vec3d x11;//x11
    x11.setZero();

	CalcNeighborSignal(locOrigImg, curSeedNode, nextDenDir, threv, neighborPtSet, neighborWet, x10, x11);//2015-8-13
	/*if (locOrigImg.XResolution() * locOrigImg.YResolution() * locOrigImg.ZResolution() > 1.5) {
		CalcNeighborSignal(locOrigImg, curSeedNode, nextDenDir, threv, neighborPtSet, neighborWet, x10, x11);
	}else{
		CalcNeighborSignalV2(locOrigImg, curSeedNode, nextDenDir, threv, neighborPtSet, neighborWet, x10, x11);
	}*/

    size_t neighborWetNum = neighborPtSet.size();
    /*W1 = sort(W, 'desend')*/
    std::vector<double> sortedNeighborWet;//W1
    sortedNeighborWet.assign(neighborWet.begin(), neighborWet.end());
    std::sort(sortedNeighborWet.begin(), sortedNeighborWet.end(), std::greater<double>());

    size_t dsw = (std::min)((size_t)20, sortedNeighborWet.size());
    /*包含*/
    double thrdk(0.0);
    if (dsw > 10){
        //thrdk = std::accumulate(sortedNeighborWet.begin(), sortedNeighborWet.begin() + dsw, 0.0) / dsw;
        thrdk = 0.0;
        for(int i = 0; i < int(dsw); ++i){
            thrdk += sortedNeighborWet[i];
        }
        thrdk /= double(dsw);
    }
    else thrdk = 0.0;

    Mat3d sigmaH;//sigmaH
    sigmaH.setZero();
    Vec3d kk;//kk
    kk.setZero();

    double P = 0.0;
    //2014-4-22
    if (neighborWetNum > std::max(0.1 * threv, 5.0) && thrdk > std::max(endThrev, std::max(0.3 * threv, 4.5 * std::sqrt(threv)))){
        /*Change : conflict between old and new version, choose new version*/
        CalcConstraintPCA(neighborPtSet, curSeedNode, 0.150 * Eigen::Matrix3d::Identity(), neighborWet, P, sigmaH, kk);
        if (P>0){
            if ( std::abs(sigmaH.determinant() / ((sigmaH.inverse()).determinant()) + 0.0001) < 10.0 ) {
                CalcPCADirections(TraceUtil::sign(sigmaH.determinant()) * sigmaH.inverse(), initDir.normalized(),
                              x11, 0.9, nextDenDir);//0.85-0.9
            }
        } else{
            isNextNodeValid=0;
        }
        Mat3d tmpResultDirections;
        Vec3d x2, x3;
        CalcOrthoBasis(nextDenDir, x2, x3);
        tmpResultDirections.col(0) = x3;
        tmpResultDirections.col(1) = x2;
        tmpResultDirections.col(2) = nextDenDir;
        //vmmk = nextDenDir;
        nextCurveNode = - (tmpResultDirections.col(0) * tmpResultDirections.col(0).transpose() + tmpResultDirections.col(1) * tmpResultDirections.col(1).transpose()) * kk + curSeedNode;
    }
    else isNextNodeValid = 0;
}

void TraceFilter::TraceNextCurveNodeV2(const Volume<unsigned short> &locOrigImg, const Vec3d &curSeedNode,
									 const double &threv, const Vec3d &initDir,
									 Vec3d &nextCurveNode, Vec3d &nextDenDir,  int &isNextNodeValid)
{
	nextCurveNode.setZero();
	isNextNodeValid = 1;
	nextDenDir = initDir;

	VectorVec3d neighborPtSet;//
	std::vector<double> neighborWet;//W

	Vec3d x10;//x10
	x10.setZero();
	Vec3d x11;//x11
	x11.setZero();

	CalcNeighborSignalV2(locOrigImg, curSeedNode, nextDenDir, threv, neighborPtSet, neighborWet, x10, x11);

	size_t neighborWetNum = neighborPtSet.size();
	/*W1 = sort(W, 'desend')*/
	std::vector<double> sortedNeighborWet;//W1
	sortedNeighborWet.assign(neighborWet.begin(), neighborWet.end());
	std::sort(sortedNeighborWet.begin(), sortedNeighborWet.end(), std::greater<double>());

	size_t dsw = (std::min)((size_t)20, sortedNeighborWet.size());//2015-8-13
	/*包含*/
	double thrdk(0.0);
	if (dsw > 15){
		//thrdk = std::accumulate(sortedNeighborWet.begin(), sortedNeighborWet.begin() + dsw, 0.0) / dsw;
		thrdk = 0.0;
		for(int i = 0; i < int(dsw); ++i){
			thrdk += sortedNeighborWet[i];
		}
		thrdk /= double(dsw);
	}
	else thrdk = 0.0;

	Mat3d sigmaH;//sigmaH
	sigmaH.setZero();
	Vec3d kk;//kk
	kk.setZero();

	double P = 0.0;
	//2014-4-22
	if (neighborWetNum > std::max(0.1 * threv, 15.0) && thrdk > std::max(endThrev, std::max(0.3 * threv, 4.5 * std::sqrt(threv)))){//2015-8-13
		/*Change : conflict between old and new version, choose new version*/
		CalcConstraintPCAv2(neighborPtSet, curSeedNode, 0.150 * Eigen::Matrix3d::Identity(), neighborWet, P, sigmaH, kk);
		if (P>0){
			if ( std::abs(sigmaH.determinant() / ((sigmaH.inverse()).determinant()) + 0.0001) < 10.0 ) {
				CalcPCADirections(TraceUtil::sign(sigmaH.determinant()) * sigmaH.inverse(), initDir.normalized(),
					x11, 0.9, nextDenDir);//0.85-0.9
			}
		} else{
			isNextNodeValid=0;
		}
		Mat3d tmpResultDirections;
		Vec3d x2, x3;
		CalcOrthoBasis(nextDenDir, x2, x3);
		tmpResultDirections.col(0) = x3;
		tmpResultDirections.col(1) = x2;
		tmpResultDirections.col(2) = nextDenDir;
		//vmmk = nextDenDir;
		nextCurveNode = - (tmpResultDirections.col(0) * tmpResultDirections.col(0).transpose() + tmpResultDirections.col(1) * tmpResultDirections.col(1).transpose()) * kk + curSeedNode;
	}
	else isNextNodeValid = 0;
}

void TraceFilter::ClearShortCurvesAndInvalidConnection(const std::vector<VectorVec5d> &rawDendCurves,
                                                       const VectorMat2i &rawDendConInfo,
                                                       std::vector<VectorVec5d> &resultDendCurves,
                                                       VectorMat2i &resultDendConInfo/*, std::vector<int> &denNums*/)
{
    std::vector<VectorVec5d>::size_type nxx = rawDendCurves.size();

    resultDendCurves.clear();
    resultDendConInfo.clear();
    //denNums.clear();

    std::vector<int> hasConCurveIdList;

    for (std::vector<VectorVec5d>::size_type i = 0; i < nxx; ++i){
        const VectorVec5d& currentCurve = rawDendCurves[i];
        const Mat2i &currentConInfo  = rawDendConInfo[i];
        if (currentCurve.size() > 15 || currentConInfo.array().abs().sum() > 0.0){
            hasConCurveIdList.push_back(i);
        }
    }
//    FILE* fp = fopen("/home/zhouhang/hasConCurveID.txt","w");
//    for(size_t k = 0; k < hasConCurveIdList.size();++k){
//        fprintf(fp, "%d\n",hasConCurveIdList[k]);
//    }
//    fclose(fp);

    for (std::vector<int>::size_type i = 0; i < hasConCurveIdList.size(); ++i){
        resultDendCurves.push_back(rawDendCurves[ hasConCurveIdList[i] ] );
    }

    for (std::vector<int>::size_type j = 0; j < hasConCurveIdList.size(); ++j){
        Mat2i currentConInfo = rawDendConInfo[ hasConCurveIdList[j] ];
        //Change:2014-3-21-10-23
        if (currentConInfo(0,0) > 0.0 ){
            int conInfoList = 0;
            for (std::vector<int>::size_type k = 0; k < hasConCurveIdList.size(); ++k){
                //saa.push_back(denIndex[k] - cons(0,0))
                if ( (hasConCurveIdList[k] + 1 - (int)currentConInfo(0,0) ) == 0 ){
                    conInfoList = k + 1;
                    break;
                }
            }//for
            currentConInfo(0,0) = conInfoList;
        }//if

        if (currentConInfo(0,1) > 0){
            int conInfoList = 0;
            for (std::vector<int>::size_type k = 0; k < hasConCurveIdList.size(); ++k){
                //saa.push_back(denIndex[k] - cons(0,0))
                if ( (hasConCurveIdList[k] + 1 - (int)currentConInfo(0,1) ) == 0 ){
                    conInfoList = k + 1;
                    break;
                }
            }//for
            currentConInfo(0,1) = conInfoList;
        }//if
        resultDendConInfo.push_back(currentConInfo);
    }
}

void TraceFilter::AddCollideConnectNode(const std::vector<VectorVec5d> &dendCurves, const VectorMat2i &dendConInfo,
                                        const VectorVec3d &somaList, std::vector<VectorVec5d> &resultDendList)
{
    std::vector<VectorVec5d>::size_type kk = dendCurves.size();
    resultDendList.clear();
    resultDendList = dendCurves;

    for ( std::vector<VectorVec5d>::size_type ij = 0; ij < kk; ++ij){
        Mat2i currentConInfo = dendConInfo[ij];//这个要复制一份
        if (currentConInfo(0,0) > 0){
            const VectorVec5d& currentDend = resultDendList[ij];
            Vec3d firstDendPt;
            //ChangeVec3d(data1[0], datap);
            firstDendPt << currentDend[0](0), currentDend[0](1), currentDend[0](2);

            VectorVec3d data16;
            Vec3d tmpData;
            //Change:2014-3-21-10-28
            for (int i = 5; i > - 1; --i){
                //ChangeVec3d(data1[i], tmpData);
                tmpData << currentDend[i](0), currentDend[i](1), currentDend[i](2);
                data16.push_back(tmpData);
            }
            Vec3d mainDir;
            //Vec3d directions2;
            //Vec3d directions3;
            CalcPtCurveDirection(data16, mainDir);

            const VectorVec5d &conDendCurve = resultDendList[currentConInfo(0,0) - 1];//因为不能改变另外线的内容

            VectorVec5d::size_type nxx = conDendCurve.size();

            std::vector<std::pair<int, double> > conNodeDistList;
            Vec3d datap2;
            Vec3d datap22;
            for (VectorVec5d::size_type ii = 0; ii < nxx; ++ ii){
                datap2 << conDendCurve[ii](0), conDendCurve[ii](1), conDendCurve[ii](2);
                datap22 = firstDendPt - datap2;
                conNodeDistList.push_back(std::pair<int, double>(ii + 1, datap22.norm()));
            }
#ifdef __linux
            std::sort(conNodeDistList.begin(), conNodeDistList.end(), [](const std::pair<int, double>& lhs, const std::pair<int, double>& rhs){
                return lhs.second < rhs.second;
            });
#else
            std::sort(conNodeDistList.begin(), conNodeDistList.end(), Pair_less());

#endif
            //Change:2014-3-21-10-32
            int isShortestExist = std::min(1, (int)nxx);//1 or 0
            std::vector<int> shortestNodeId;
            for (int ii = 0; ii < isShortestExist; ++ii){
                shortestNodeId.push_back(conNodeDistList[ii].first);
            }

            std::vector<double> conNodeDistListCopy;
            Vec3d data2p;
            for(int ii = 0; ii < isShortestExist; ++ii){
                //ChangeVec3d(data2[aas[ii] - 1], data2p);
                data2p << conDendCurve[shortestNodeId[ii] - 1](0), conDendCurve[shortestNodeId[ii] - 1](1), conDendCurve[shortestNodeId[ii] - 1](2);
                double den = (data2p - firstDendPt).norm();
                double num = (data2p - firstDendPt).transpose() * mainDir;
                conNodeDistListCopy.push_back(num / den);
            }

            std::vector<double>::const_iterator maxItem = std::max_element(conNodeDistListCopy.begin(), conNodeDistListCopy.end());
            currentConInfo(1,0) = shortestNodeId[maxItem - conNodeDistListCopy.begin()];
            VectorVec5d tmp;
            tmp.push_back(conDendCurve[currentConInfo(1,0) - 1]);
            for (VectorVec5d::size_type k = 0; k  < currentDend.size(); ++k){
                tmp.push_back(currentDend[k]);
            }
            resultDendList[ij].clear();
            //Change:2014-3-21-10-40
            resultDendList[ij].swap(tmp);
        }//if

        if (currentConInfo(0,0) < 0){
            const VectorVec5d& currentDend = resultDendList[ij];
            currentConInfo(1,0) = -currentConInfo(0,0);
            VectorVec5d tmp;
            Vec5d myce;
            myce << somaList[currentConInfo(1,0) - 1](0), somaList[currentConInfo(1,0) - 1](1), somaList[currentConInfo(1,0) -1](2),
                1.0, 0.0;
            tmp.push_back(myce);

            for (VectorVec5d::size_type k = 0; k  < currentDend.size(); ++k){
                tmp.push_back(currentDend[k]);
            }
            resultDendList[ij].clear();
            resultDendList[ij].swap(tmp);
        }

        if (currentConInfo(0,1) < 0.0) {
            const VectorVec5d& currentDend = resultDendList[ij];
            currentConInfo(1,1) = -currentConInfo(0,1);
            VectorVec5d tmp;

            for (VectorVec5d::size_type k = 0; k  < currentDend.size(); ++k){
                tmp.push_back(currentDend[k]);
            }
            Vec5d myce;
            myce << somaList[currentConInfo(1,1) - 1](0), somaList[currentConInfo(1,1) - 1](1), somaList[currentConInfo(1,1) - 1](2),
                1.0, 0.0;
            tmp.push_back(myce);
            resultDendList[ij].clear();
            resultDendList[ij].swap(tmp);
        }

        if (currentConInfo(0,1) > 0.0){
            const VectorVec5d& data1 = resultDendList[ij];
            Vec3d firstDendPt;
            int nxx1 = data1.size();
            //ChangeVec3d(data1[nxx1 - 1], datap);
            firstDendPt << data1[nxx1 - 1](0), data1[nxx1 - 1](1), data1[nxx1 - 1](2);

            VectorVec3d data16;
            Vec3d tmpData;
            //Change:2014-3-21-10-42 8->6
            for (int i = std::max(nxx1 - 6, 0); i <nxx1; ++i){
                //ChangeVec3d(data1[i], tmpData);
                tmpData << data1[i](0), data1[i](1), data1[i](2);
                data16.push_back(tmpData);
            }
            Vec3d mainDir;
            //Vec3d directions2;
            //Vec3d directions3;
            CalcPtCurveDirection(data16, mainDir);

            const VectorVec5d &conDendCurve = resultDendList[currentConInfo(0,1) - 1];

            VectorVec5d::size_type nxx = conDendCurve.size();

            std::vector<std::pair<int, double> > conNodeDistList;
            Vec3d datap2;
            Vec3d datap22;
            for (VectorVec5d::size_type ii = 0; ii < nxx; ++ ii){
                //ChangeVec3d(data2[ii], datap2);
                datap2 << conDendCurve[ii](0), conDendCurve[ii](1), conDendCurve[ii](2);
                datap22 = firstDendPt - datap2;
                conNodeDistList.push_back(std::pair<int, double>(ii + 1, datap22.norm()));
            }
#ifdef __linux
            std::sort(conNodeDistList.begin(), conNodeDistList.end(), [](const std::pair<int, double>& lhs, const std::pair<int, double>& rhs){
                return lhs.second < rhs.second;
            });
#else
            std::sort(conNodeDistList.begin(), conNodeDistList.end(), Pair_less());

#endif
            //Change:5->1
            int isShortestExist = std::min(1, int(nxx));
            std::vector<int> shortestNodeId;
            for (int ii = 0; ii < isShortestExist; ++ii){
                shortestNodeId.push_back(conNodeDistList[ii].first);
            }

            std::vector<double> conNodeDistListCopy;
            Vec3d data2p;
            for (int ii = 0; ii < isShortestExist; ++ii){
                //ChangeVec3d(data2[aas[ii] - 1], data2p);
                data2p << conDendCurve[shortestNodeId[ii] - 1](0), conDendCurve[shortestNodeId[ii] - 1](1), conDendCurve[shortestNodeId[ii] - 1](2);
                double den = (data2p - firstDendPt).norm();
                double num = (data2p - firstDendPt).transpose() * mainDir;
                conNodeDistListCopy.push_back(num / den);
            }

            std::vector<double>::const_iterator maxItem = std::max_element(conNodeDistListCopy.begin(), conNodeDistListCopy.end());
            currentConInfo(1,1) = shortestNodeId[maxItem - conNodeDistListCopy.begin()];
            VectorVec5d tmp;

            for (VectorVec5d::size_type k = 0; k  < data1.size(); ++k){
                tmp.push_back(data1[k]);
            }
            tmp.push_back(conDendCurve[currentConInfo(1,1) - 1]);
            resultDendList[ij].clear();
            resultDendList[ij].swap(tmp);
        }
    }
}

void TraceFilter::ReconstructSomaShapeForTrace(const VectorVec3d &somaList, const SVolume &origImg, Volume<int> &indexImg, VectorMatXd &allRayLen)
{
    //indexImg.SetSize(origImg.x(), origImg.y(), origImg.z());
    //indexImg.SetZero();
    int nx = origImg.x();
    int ny = origImg.y();
    int nz = origImg.z();
    int somaListLen = somaList.size();

    for(int ii = 0; ii < somaListLen; ++ii){
        const Vec3d& initialP = somaList[ii];
        int Idex=std::min<int>(std::max<int>(Round(initialP(0)),0), nx-1);
        int Idey=std::min<int>(std::max<int>(Round(initialP(1)),0), ny-1);
        int Idez=std::min<int>(std::max<int>(Round(initialP(2)),0), nz-1);
        VectorVec3d innerSomaPts;
        if(indexImg(Idex, Idey,Idez) == 0){
            MatXd resultRayLength;
			if (origImg.XResolution() * origImg.YResolution() * origImg.ZResolution() > 1.5) {
				ReconstructShapeForTrace(initialP,origImg, resultRayLength,innerSomaPts);
			}else{
				printf("ReconstructSomaShapeQuanRevi\n");
				ReconstructSomaShapeQuanRevi(initialP, resultRayLength, innerSomaPts);//2015-8-13
			}

            //printf("ReconstructShapeForTrace:%d\n", ii);
            allRayLen.push_back(resultRayLength);
        } else{
            allRayLen.push_back(MatXd::Zero(42,22));
            innerSomaPts.clear();
        }
        //printf("ReconstructShapeForTrace finish\n");
        size_t innerSomaPtsNum = innerSomaPts.size();
        Vec3i roundCurPt;
        for(size_t i = 0; i < innerSomaPtsNum; ++i){
            Vec3d& curPt=innerSomaPts[i];
            roundCurPt << Round(curPt(0)),Round(curPt(1)),Round(curPt(2));
            roundCurPt(0) =std::max( std::min(nx-1, roundCurPt(0)), 0);
            roundCurPt(1) =std::max( std::min(ny-1, roundCurPt(1)), 0);
            roundCurPt(2) =std::max( std::min(nz-1, roundCurPt(2)), 0);
            indexImg(roundCurPt(0), roundCurPt(1), roundCurPt(2))=-(ii+1);
        }
    }
}

void TraceFilter::FindThickDendDirFromSoma(const MatXd &rayLength, const SVolume &origImg,
                                           const SVolume &backImg, const Vec3d &currentSoma,
                                           const Volume<int> &indexImg, const int seriID,
                                           Vec3d &mainDir1, Vec3d &mainDir2, Vec3d &mainDir3,
                                           Vec3d &thickDendInitPt)
{
    int Theta=40;
    int Phi = 20;
    Volume<unsigned short> locOrigImg;
    Vec3d locSoma;
    ExtractLocalDomainAlias(currentSoma, origImg, backImg, indexImg,
                            seriID, locOrigImg, locSoma);
    int nxLocOrig = locOrigImg.x();
    int nyLocOrig=locOrigImg.y();
    int nzLocOrig=locOrigImg.z();

    VectorVec3d innerSomaPt;
    const double a = 360.0 / (double)Theta;
    const double b = 180.0 / (double)Phi;
    const double PI_180 = M_PI / 180.0;
    double xx,yy,zz;
    for (int i = 1; i <= Theta; ++i){
        for (int j = 1; j <= Phi; ++j){
            if(rayLength(i,j) > 0.0){
                for(double k = 1.0; k <= 0.5 * rayLength(i,j); ++k){
                    xx = (k * std::sin(b * (double)(j) * PI_180)
                          * std::cos(a * (double)(i) * PI_180) + locSoma(0));
                    yy = (k * std::sin(b * (double)(j) * PI_180)
                          * std::sin(a * (double)(i) * PI_180) + locSoma(1));
                    zz = (k * std::cos(b * (double)(j) * PI_180) + locSoma(2));

                    xx=std::min<double>(std::max<double>(xx,0.0), double(nxLocOrig)-1.0);
                    yy=std::min<double>(std::max<double>(yy,0.0), double(nyLocOrig)-1.0);
                    zz=std::min<double>(std::max<double>(zz,0.0), double(nzLocOrig)-1.0);
                    xx = Round(xx );
                    yy = Round(yy );
                    zz = Round(zz );
                    innerSomaPt.push_back(Vec3d(xx,yy,zz));
                }
            }
        }
    }
#ifdef __linux
    std::sort(innerSomaPt.begin(), innerSomaPt.end(), [](const Vec3d& lhs, const Vec3d &rhs){
        if (lhs(0) != rhs(0))  return lhs(0) < rhs(0);
        else if (lhs(1) != rhs(1))  return lhs(1) < rhs(1);
        else if (lhs(2) != rhs(2))  return lhs(2) < rhs(2);
        return false;
    });
#else
    std::sort(innerSomaPt.begin(), innerSomaPt.end(), Vec3d_less());

#endif
    innerSomaPt.erase(std::unique(innerSomaPt.begin(), innerSomaPt.end()),
                      innerSomaPt.end());

    double aThrev = backImg(Round(currentSoma(0)),Round(currentSoma(1)),Round(currentSoma(2)) );
    aThrev = aThrev + 5.0 * std::sqrt(aThrev);

    VectorVec3d inflatedArea;
    InflateBoundary(innerSomaPt,locOrigImg, aThrev,inflatedArea);
    size_t inflatedPtsNum = inflatedArea.size();

    VectorVec3d resultInflatedPts,medianContourPtSet;
    Volume<unsigned char> locBinImg;
    locBinImg.SetSize(locOrigImg.x(), locOrigImg.y(), locOrigImg.z());
    //double xx,yy,zz;
    for(int ii = 0; ii < Theta; ++ii){
        for(int jj = 0; jj < Phi; ++jj){
            double k = 0.5 * rayLength(ii,jj);
            xx = (k * std::sin(b * (double)(jj+1) * PI_180)
                  * std::cos(a * (double)(ii+1) * PI_180) + locSoma(0));
            yy = (k * std::sin(b * (double)(jj+1) * PI_180)
                  * std::sin(a * (double)(ii+1) * PI_180) + locSoma(1));
            zz = (k * std::cos(b * (double)(jj+1) * PI_180) + locSoma(2));

            xx = Round(xx );
            yy = Round(yy );
            zz = Round(zz );
            medianContourPtSet.push_back(Vec3d(xx,yy,zz));
        }
    }
#ifdef __linux
    std::sort(medianContourPtSet.begin(), medianContourPtSet.end(), [](const Vec3d& lhs, const Vec3d &rhs){
        if (lhs(0) != rhs(0))  return lhs(0) < rhs(0);
        else if (lhs(1) != rhs(1))  return lhs(1) < rhs(1);
        else if (lhs(2) != rhs(2))  return lhs(2) < rhs(2);
        return false;
    });
#else
    std::sort(medianContourPtSet.begin(), medianContourPtSet.end(), Vec3d_less());

#endif
    medianContourPtSet.erase(std::unique(medianContourPtSet.begin(),
                                            medianContourPtSet.end()),
                      medianContourPtSet.end());

    size_t medianContourPtSetNum = medianContourPtSet.size();

    for(size_t i = 0; i < inflatedPtsNum; ++i){
        Vec2d distFlag;
        double minDist = 100000;
        double curDist;
        for(size_t j = 0; j < medianContourPtSetNum; ++j){
            curDist = (inflatedArea[i] - medianContourPtSet[j]).norm();
            if(curDist > 2.0 && curDist < minDist){
                distFlag << curDist, double(i);
                minDist = curDist;
            }
        }
        Vec3i tmp;
        if(distFlag(0) > 2.0){
            resultInflatedPts.push_back(inflatedArea[i]);
            tmp << int(inflatedArea[i](0)), int(inflatedArea[i](1)),int(inflatedArea[i](2));
            locBinImg(tmp(0), tmp(1),tmp(2))=255;
        }
    }
    CVolume eroBinImg;
    VectorVec3d eroIndexPtSet;
    Corrode(locBinImg, resultInflatedPts,10.0,eroBinImg, eroIndexPtSet);
    VectorVec3d cxDomainSet;
    std::vector<int> cxDomainNumList;
    DetectCellSignalModify(eroBinImg, eroIndexPtSet, 12, cxDomainSet,cxDomainNumList);
    if(!cxDomainNumList.empty()){
        std::vector<int>::iterator maxIt = std::max_element(cxDomainNumList.begin(),
                                                            cxDomainNumList.end());
        int maxCxDmID = std::distance(cxDomainNumList.begin(), maxIt);
        std::deque<int> cumDmNumList;
        std::partial_sum(cxDomainNumList.begin(), cxDomainNumList.end(),std::back_inserter(
                             cumDmNumList));
        cumDmNumList.push_front(0);
        VectorVec3d curDmSet;
        for(int k = cumDmNumList[maxCxDmID]; k < cumDmNumList[maxCxDmID+1];++k){
            curDmSet.push_back(cxDomainSet[k]);
        }
        std::vector<double> curDmWet;
        WeighRayValue(curDmSet,locOrigImg, curDmWet);
        double sumCurDmWet = 0.0;
        for(size_t l = 0; l < curDmWet.size(); ++l){
            sumCurDmWet += curDmWet[l];
        }
        for(size_t l = 0; l < curDmWet.size(); ++l){
            curDmWet[l] /= sumCurDmWet;
        }
        Vec3d curDmCenter(0.0,0.0,0.0);
        for(size_t l = 0; l < curDmWet.size(); ++l){
            curDmCenter += curDmSet[l]*curDmWet[l];
        }
        thickDendInitPt=curDmCenter - locSoma + currentSoma;
        mainDir1 = thickDendInitPt - currentSoma;
        mainDir1.normalize();
        CalcOrthoBasis(mainDir1, mainDir2, mainDir3);

    }else{
        mainDir1.setZero();
        mainDir2.setZero();
        mainDir3.setZero();
        thickDendInitPt.setZero();
    }
}

void TraceFilter::ExtractLocalDomainAlias(const Vec3d &currentSoma,
                                          const Volume<unsigned short> &origImg,
                                          const Volume<unsigned short> &backImg,
                                          const Volume<int> &indexImg, const int seriID,
                                          Volume<unsigned short>& locOrigImg, Vec3d &locPt)
{
    int nx = origImg.x();
    int ny = origImg.y();
    int nz = origImg.z();
    int xMin, xMax, yMin, yMax, zMin, zMax;
    Get3DRegion(xMin, xMax, yMin, yMax, zMin, zMax, int(currentSoma(0)+0.5),
                int(currentSoma(1)+0.5), int(currentSoma(2)+0.5), 30,30,15,
                0,nx-1,0,ny-1,0,nz-1);
    int xLen = xMax - xMin + 1;
    int yLen = yMax - yMin + 1;
    int zLen = zMax - zMin + 1;
    locOrigImg.SetSize(xLen, yLen, zLen);
    for(int i = xMin; i <= xMax; ++i){
        for(int j = yMin; j <= yMax; ++j){
            for(int ij = zMin; ij <= zMax; ++ij){
                locOrigImg(i-xMin,j-yMin,ij-zMin) = origImg(i,j,ij);
            }
        }
    }
    //no locIndexImg, use indexImg(i+xMin...
    //no locBackImg, use backImg(i+xMin...
    for(int i = 0; i < xLen; ++i){
        for(int j = 0; j < yLen; ++j){
            for(int k = 0; k < zLen; ++k){
                if(std::abs(indexImg(i + xMin, j + yMin, k + zMin)) > 0
                        && indexImg(i + xMin, j + yMin, k + zMin)
                                    != seriID){

                    locOrigImg(i,j,k)=
                            int(0.8*double(backImg(i + xMin, j + yMin, k + zMin))+0.5);
                }
            }
        }
    }
    locPt << currentSoma(0) - xMin, currentSoma(1) - yMin, currentSoma(2) - zMin;

}

void TraceFilter::ForceModifyDirection(VectorMat3d &allThickDendInitDir, std::vector<int> &largeAngleFlagList)
{
    size_t nzz = allThickDendInitDir.size();
    MatXd mainDirCorrMat = MatXd::Identity(nzz,nzz);
    largeAngleFlagList.resize(nzz, 0);
    Vec3d mainDirI, mainDirJ;
    for(size_t i = 0; i < nzz; ++i){
        for(size_t j = i+1; j < nzz; ++j){
            mainDirI= allThickDendInitDir[i].col(0);
            mainDirJ = allThickDendInitDir[j].col(0);
            mainDirCorrMat(i,j)=mainDirI.dot(mainDirJ);
            mainDirCorrMat(j,i) = mainDirCorrMat(i,j);
        }
    }
    std::vector<double> eachDirCorrSumList(nzz, 0.0);
    for(size_t i = 0; i < nzz; ++i){
        eachDirCorrSumList[i] = mainDirCorrMat.col(i).sum();
    }
    std::vector<double>::iterator maxIt = std::max_element(eachDirCorrSumList.begin(), eachDirCorrSumList.end());
    int maxCorrDirID = std::distance(eachDirCorrSumList.begin(), maxIt);
    Vec3d maxCorrDir = allThickDendInitDir[maxCorrDirID].col(0);
    for(size_t i=0; i < nzz; ++i){
        mainDirI = allThickDendInitDir[i].col(0);
        if( mainDirI.dot(maxCorrDir) < 0.9  ){
            allThickDendInitDir[i]=allThickDendInitDir[maxCorrDirID];
            largeAngleFlagList[i]=1;
        }
    }
}

void TraceFilter::DetectCollision(const VectorVec5d &somaCurve, const VectorVec3d &somaList,
                                  const Vec2i &initCollideInfo, const Volume<int> &indexImg,
                                  const Volume<unsigned short> &origImg, const Volume<unsigned short> &backImg,
                                  const int origID, const int isSoma, const int isContinue,
                                  VectorVec5d &modifiedSomaCurve, int &isContinueTrace,
                                  Vec2i &resultCollideInfo, int &curID, int& resultConInfo)
{
    resultConInfo = 0;
    curID=origID;
    int nxx = origImg.x();
    int nyy = origImg.y();
    int nzz = origImg.z();
    Vec2i collideInfo = initCollideInfo;
    isContinueTrace = isContinue;
    modifiedSomaCurve = somaCurve;
    curID = origID;
    int id1 = Round(modifiedSomaCurve[curID](0));
    int id2 = Round(modifiedSomaCurve[curID](1));
    int id3 = Round(modifiedSomaCurve[curID](2));
    int xMin, xMax, yMin, yMax, zMin, zMax;
    xMin = std::max<int>(0, id1 - 2);
    xMax = std::min<int>(id1 + 2, nxx - 1);
    yMin = std::max<int>(0, id2 - 2);
    yMax = std::min<int>(id2 + 2, nyy - 1);
    zMin = std::max<int>(0, id3 - 2);
    zMax = std::min<int>(id3 + 2, nzz - 1);
    VectorVec3d tmpCurve;
    TraceUtil::GetPartVectorVec3d(modifiedSomaCurve, std::max<int>(curID-6,0),
                                  curID, tmpCurve);
    Vec3d directions;
    CalcPtCurveDirection(tmpCurve, directions);
    int collideConInfo;
    double curveCollideFlag;
    double somaCollideFlag;
    IsCurvesCollide(indexImg,xMin, xMax, yMin, yMax, zMin, zMax,collideConInfo,
                    curveCollideFlag, somaCollideFlag);
    if(curveCollideFlag > 0.5){
        collideInfo(0)=collideConInfo;
        isContinueTrace = 0;
    }
    if(somaCollideFlag < 0.5){
        if(isSoma == 0){
            collideInfo(0)=collideConInfo;
            isContinueTrace = 0;
        } else{
            Vec3d nodeOtherSomaVec = modifiedSomaCurve[curID].block(0,0,3,1)
                    - somaList[-collideConInfo-1];//warning
            nodeOtherSomaVec(2) *= 2;
            double dist = std::pow(nodeOtherSomaVec.norm(), 2.0) -
                    std::pow(directions.dot(nodeOtherSomaVec), 2.0);
            dist = std::sqrt(dist);
            if(dist < 5.5){
                collideInfo(0)=collideConInfo;
                isContinueTrace = 0;
            }
        }
    }
    if(curveCollideFlag > 0.5){
        TraceUtil::GetPartVectorVec3d(modifiedSomaCurve,
                                      std::max<int>(curID - 10,0),
                                      curID - 2, tmpCurve);
        Vec3d drc1;
        CalcPtCurveDirection(tmpCurve, drc1);
        Vec3d initPoint = modifiedSomaCurve[curID-2].block(0,0,3,1);
        VectorVec3d crossCurve;
        int isThroughKnot;
        TraceForwardInKnot(drc1, initPoint, backImg, origImg,
                           indexImg, collideConInfo, crossCurve,isThroughKnot);
        if(1 == isThroughKnot){
            resultConInfo = collideConInfo;
            isContinueTrace = 1;
            modifiedSomaCurve.resize(curID + 4);
            modifiedSomaCurve[curID - 1].block(0,0,3,1)=crossCurve[1];
            modifiedSomaCurve[curID].block(0,0,3,1)=crossCurve[2];
            modifiedSomaCurve[curID + 1].block(0,0,3,1)=crossCurve[3];
            modifiedSomaCurve[curID + 2].block(0,0,3,1)=crossCurve[4];
            modifiedSomaCurve[curID + 3].block(0,0,3,1)=crossCurve[5];
            curID += 3;
            collideInfo(0) = 0;
        }
    }
    resultCollideInfo = collideInfo;
}

void TraceFilter::DetectCollisionV2(const VectorVec5d &somaCurve, const VectorVec3d &somaList,
								  const Vec2i &initCollideInfo, const Volume<int> &indexImg,
								  const Volume<unsigned short> &origImg, const Volume<unsigned short> &backImg,
								  const int origID, const int isSoma, const int isContinue,
								  VectorVec5d &modifiedSomaCurve, int &isContinueTrace,
								  Vec2i &resultCollideInfo, int &curID, int& resultConInfo)
{
	resultConInfo = 0;
	curID=origID;
	int nxx = origImg.x();
	int nyy = origImg.y();
	int nzz = origImg.z();
	Vec2i collideInfo = initCollideInfo;
	isContinueTrace = isContinue;
	modifiedSomaCurve = somaCurve;
	curID = origID;
	int id1 = Round(modifiedSomaCurve[curID](0));
	int id2 = Round(modifiedSomaCurve[curID](1));
	int id3 = Round(modifiedSomaCurve[curID](2));
	int xMin, xMax, yMin, yMax, zMin, zMax;
	xMin = std::max<int>(0, id1 - 2);
	xMax = std::min<int>(id1 + 2, nxx - 1);
	yMin = std::max<int>(0, id2 - 2);
	yMax = std::min<int>(id2 + 2, nyy - 1);
	zMin = std::max<int>(0, id3 - 2);
	zMax = std::min<int>(id3 + 2, nzz - 1);
	VectorVec3d tmpCurve;
	TraceUtil::GetPartVectorVec3d(modifiedSomaCurve, std::max<int>(curID-6,0),
		curID, tmpCurve);
	Vec3d directions;
	CalcPtCurveDirection(tmpCurve, directions);
	int collideConInfo;
	double curveCollideFlag;
	double somaCollideFlag;
	IsCurvesCollideV2(indexImg,xMin, xMax, yMin, yMax, zMin, zMax,collideConInfo,
		curveCollideFlag, somaCollideFlag);
	if(curveCollideFlag > 0.1){//2015-8-13
		collideInfo(0)=collideConInfo;
		isContinueTrace = 0;
	}
	if(somaCollideFlag < 0.5){
		if(isSoma == 0){
			collideInfo(0)=collideConInfo;
			isContinueTrace = 0;
		} else{
			Vec3d nodeOtherSomaVec = modifiedSomaCurve[curID].block(0,0,3,1)
				- somaList[-collideConInfo-1];//warning
			nodeOtherSomaVec(2) *= 2;
			double dist = std::pow(nodeOtherSomaVec.norm(), 2.0) -
				std::pow(directions.dot(nodeOtherSomaVec), 2.0);
			dist = std::sqrt(dist);
			if(dist < 5.5){
				collideInfo(0)=collideConInfo;
				isContinueTrace = 0;
			}
		}
	}
	if(curveCollideFlag > 3.5){//2015-8-13
		TraceUtil::GetPartVectorVec3d(modifiedSomaCurve,
			std::max<int>(curID - 10,0),
			curID - 2, tmpCurve);
		Vec3d drc1;
		CalcPtCurveDirection(tmpCurve, drc1);
		Vec3d initPoint = modifiedSomaCurve[curID-2].block(0,0,3,1);
		VectorVec3d crossCurve;
		int isThroughKnot;
		TraceForwardInKnotV2(drc1, initPoint, backImg, origImg,
			indexImg, collideConInfo, crossCurve,isThroughKnot);
		if(1 == isThroughKnot){
			resultConInfo = collideConInfo;
			isContinueTrace = 1;
			modifiedSomaCurve.resize(curID + 4);
			modifiedSomaCurve[curID - 1].block(0,0,3,1)=crossCurve[1];
			modifiedSomaCurve[curID].block(0,0,3,1)=crossCurve[2];
			modifiedSomaCurve[curID + 1].block(0,0,3,1)=crossCurve[3];
			modifiedSomaCurve[curID + 2].block(0,0,3,1)=crossCurve[4];
			modifiedSomaCurve[curID + 3].block(0,0,3,1)=crossCurve[5];
			curID += 3;
			collideInfo(0) = 0;
		}
	}
	resultCollideInfo = collideInfo;
}

void TraceFilter::TraceForwardInKnot(const Vec3d &initMainDir, const Vec3d &initP,
                                     const Volume<unsigned short> &backImg,
                                     const Volume<unsigned short> &origImg,
                                     const Volume<int> &indexImg, const int collideConInfo,
                                     VectorVec3d &crossCurve, int &isContinueTrace)
{
    int i = 0;
    int nxx = backImg.x();
    int nyy = backImg.y();
    int nzz = backImg.z();
    VectorVec3d tmpDendCurve;
    tmpDendCurve.push_back(initP);
    int isNextNodeValid = 1;
    Vec3d nextDendDir(initMainDir);
    crossCurve.clear();
    //double projLength = 0.0;
    while(isNextNodeValid == 1 && i < 14){
        int xMin, xMax, yMin, yMax, zMin, zMax;
        Get3DRegion(xMin, xMax, yMin, yMax, zMin, zMax,
            Round(tmpDendCurve[i](0) ), Round(tmpDendCurve[i](1)), Round(tmpDendCurve[i](2) ), 10, 10, 7,
            0, nxx - 1, 0, nyy - 1, 0, nzz - 1);

        Volume<unsigned short> locOrigImg;
        ExtractArea(origImg, xMin, xMax, yMin, yMax, zMin, zMax, locOrigImg);

//        if((xMax - xMin) * (yMax - yMin) * (zMax - zMin) == 0){
//            isNextNodeValid = 0;
//            break;
//        }
        Vec3d lefttopAxis(xMin, yMin, zMin);

        int rdrvx = std::min(std::max(Round(tmpDendCurve[i](0) - 1.0 ), 0), nxx - 1);
        //2014-4-22
        //int rdrvy = std::min(std::max(int(tmpDendCurve[i](1) - 3.0 + 0.5), 0), nyy - 1);
        int rdrvy = std::min(std::max(Round(tmpDendCurve[i](1) - 1.0 ), 0), nyy - 1);
        int rdrvz = std::min(std::max(Round(tmpDendCurve[i](2) - 1.0 ), 0), nzz - 1);
        double threv = backImg(rdrvx, rdrvy, rdrvz);//int(double(zMin + zMax) / 2.0 + 0.5)
        Vec3d tmpCurrentCurveNode(tmpDendCurve[i](0) - xMin, tmpDendCurve[i](1) - yMin, tmpDendCurve[i](2) - zMin);

        Vec3d nextCurveNode;
        nextCurveNode.setZero();
        Vec3d nextDendDirCopy;
        nextDendDirCopy.setZero();
        Vec3d tmpx1;
        TraceNextCurveNode(locOrigImg, tmpCurrentCurveNode, threv, nextDendDir,nextCurveNode, tmpx1,
            isNextNodeValid);
        nextDendDir = tmpx1;
        nextDendDirCopy = nextDendDir;
        nextDendDir=0.8*initMainDir + 0.2 * nextDendDir;
        nextDendDir.normalize();

        if ( nextCurveNode(0) != 0.0 && nextCurveNode(1) != 0.0 && nextCurveNode(2) != 0.0 )
            nextCurveNode += lefttopAxis;

        if(isNextNodeValid == 1){
            tmpDendCurve[i].block(0,0,3,1)=nextCurveNode;
            ++i;
            tmpDendCurve.push_back(nextCurveNode+2.0*nextDendDirCopy);
        }
        if(tmpDendCurve[i].block(0,0,3,1).minCoeff() < -1.0 || tmpDendCurve[i](0) > nxx
                || tmpDendCurve[i](1) > nyy || tmpDendCurve[i](2) > nzz){
            isNextNodeValid = 0;
        }

        if(isNextNodeValid > 0){
            VectorVec3d tmpCurve;
            TraceUtil::GetPartVectorVec3d(tmpDendCurve, i-1, i, tmpCurve);
            std::vector<double> origWet,backWet;
            WeighRayValue(tmpCurve, origImg,origWet);
            WeighRayValue(tmpCurve, backImg, backWet);
            std::vector<double> threvWet(origWet.size());
            for(size_t k = 0; k < origWet.size(); ++k){
                threvWet[k]=origWet[k] - backWet[k] - 3.5 * std::sqrt(backWet[k]);
            }
            double tmpMax = threvWet[0] > threvWet[1]?threvWet[0] : threvWet[1];
            if(tmpMax < 0.0){
                isNextNodeValid = 0;
                i = i-2;
            }
        }
    }
    TraceUtil::GetPartVectorVec3d(tmpDendCurve,0,i,crossCurve);
    if(i > 4){
        std::vector<int> indexWindow(i+1);
        std::vector<int> origWindow(i+1);
        std::vector<int> backWindow(i+1);
        for(int ij = 0; ij <= i; ++ij){
            //<=i
            Vec3i curRoundPt;
            curRoundPt << Round(crossCurve[ij](0)),Round(crossCurve[ij](1)),
                          Round(crossCurve[ij](2));

            indexWindow[ij]= indexImg(ValueInMinMax<int>(curRoundPt(0),0,nxx-1),
                                      ValueInMinMax<int>(curRoundPt(1),0,nyy-1),
                                      ValueInMinMax<int>(curRoundPt(2),0,nzz-1));
            origWindow[ij]= origImg(ValueInMinMax<int>(curRoundPt(0),0,nxx-1),
                                      ValueInMinMax<int>(curRoundPt(1),0,nyy-1),
                                      ValueInMinMax<int>(curRoundPt(2),0,nzz-1));
          backWindow[ij]= backImg(ValueInMinMax<int>(curRoundPt(0),0,nxx-1),
                                    ValueInMinMax<int>(curRoundPt(1),0,nyy-1),
                                    ValueInMinMax<int>(curRoundPt(2),0,nzz-1));

            if(double(origWindow[ij]) > double(backWindow[ij])+4.5*std::sqrt(double(backWindow[ij]))){
                continue;
            }else{
                indexWindow[ij]=0;
            }
        }
        double projLength = (crossCurve[i]-crossCurve[0]).dot(initMainDir);
        std::vector<int> tmpList;
        FindEqualList(indexWindow, collideConInfo, tmpList);
        size_t validLen = tmpList.size();
        size_t noValidLen = indexWindow.size() - validLen;
        if(projLength > 10.0 && noValidLen >6){
            isContinueTrace = 1;
        } else{
            isContinueTrace = 0;
        }
    } else{
        isContinueTrace = 0;
    }
}

void TraceFilter::TraceForwardInKnotV2(const Vec3d &initMainDir, const Vec3d &initP,
									 const Volume<unsigned short> &backImg,
									 const Volume<unsigned short> &origImg,
									 const Volume<int> &indexImg, const int collideConInfo,
									 VectorVec3d &crossCurve, int &isContinueTrace)
{
	int i = 0;
	int nxx = backImg.x();
	int nyy = backImg.y();
	int nzz = backImg.z();
	VectorVec3d tmpDendCurve;
	tmpDendCurve.push_back(initP);
	int isNextNodeValid = 1;
	Vec3d nextDendDir(initMainDir);
	crossCurve.clear();
	//double projLength = 0.0;
	while(isNextNodeValid == 1 && i < 14){
		int xMin, xMax, yMin, yMax, zMin, zMax;
		Get3DRegion(xMin, xMax, yMin, yMax, zMin, zMax,
			Round(tmpDendCurve[i](0) ), Round(tmpDendCurve[i](1)), Round(tmpDendCurve[i](2) ), 15, 15, 10,
			0, nxx - 1, 0, nyy - 1, 0, nzz - 1);

		Volume<unsigned short> locOrigImg;
		ExtractArea(origImg, xMin, xMax, yMin, yMax, zMin, zMax, locOrigImg);

		Vec3d lefttopAxis(xMin, yMin, zMin);

		int rdrvx = std::min(std::max(Round(tmpDendCurve[i](0) - 1.0 ), 0), nxx - 1);
		//2014-4-22
		//int rdrvy = std::min(std::max(int(tmpDendCurve[i](1) - 3.0 + 0.5), 0), nyy - 1);
		int rdrvy = std::min(std::max(Round(tmpDendCurve[i](1) - 1.0 ), 0), nyy - 1);
		int rdrvz = std::min(std::max(Round(tmpDendCurve[i](2) - 1.0 ), 0), nzz - 1);
		double threv = backImg(rdrvx, rdrvy, rdrvz);//int(double(zMin + zMax) / 2.0 + 0.5)
		Vec3d tmpCurrentCurveNode(tmpDendCurve[i](0) - xMin, tmpDendCurve[i](1) - yMin, tmpDendCurve[i](2) - zMin);

		Vec3d nextCurveNode;
		nextCurveNode.setZero();
		Vec3d nextDendDirCopy;
		nextDendDirCopy.setZero();
		Vec3d tmpx1;
		TraceNextCurveNodeV2(locOrigImg, tmpCurrentCurveNode, threv, nextDendDir,nextCurveNode, tmpx1,
			isNextNodeValid);
		nextDendDir = tmpx1;
		nextDendDirCopy = nextDendDir;
		nextDendDir=0.8*initMainDir + 0.2 * nextDendDir;
		nextDendDir.normalize();

		if ( nextCurveNode(0) != 0.0 && nextCurveNode(1) != 0.0 && nextCurveNode(2) != 0.0 )
			nextCurveNode += lefttopAxis;

		if(isNextNodeValid == 1){
			tmpDendCurve[i].block(0,0,3,1)=nextCurveNode;
			++i;
			tmpDendCurve.push_back(nextCurveNode+2.0*nextDendDirCopy);
		}
		if(tmpDendCurve[i].block(0,0,3,1).minCoeff() < -1.0 || tmpDendCurve[i](0) > nxx
			|| tmpDendCurve[i](1) > nyy || tmpDendCurve[i](2) > nzz){
				isNextNodeValid = 0;
		}

		if(isNextNodeValid > 0){
			VectorVec3d tmpCurve;
			TraceUtil::GetPartVectorVec3d(tmpDendCurve, i-1, i, tmpCurve);
			std::vector<double> origWet,backWet;
			WeighRayValue(tmpCurve, origImg,origWet);
			WeighRayValue(tmpCurve, backImg, backWet);
			std::vector<double> threvWet(origWet.size());
			for(size_t k = 0; k < origWet.size(); ++k){
				threvWet[k]=origWet[k] - backWet[k] - 3.5 * std::sqrt(backWet[k]);
			}
			double tmpMax = threvWet[0] > threvWet[1]?threvWet[0] : threvWet[1];
			if(tmpMax < 0.0){
				isNextNodeValid = 0;
				i = i-2;
			}
		}
	}
	TraceUtil::GetPartVectorVec3d(tmpDendCurve,0,i,crossCurve);
	if(i > 4){
		std::vector<int> indexWindow(i+1);
		std::vector<int> origWindow(i+1);
		std::vector<int> backWindow(i+1);
		for(int ij = 0; ij <= i; ++ij){
			//<=i
			Vec3i curRoundPt;
			curRoundPt << Round(crossCurve[ij](0)),Round(crossCurve[ij](1)),
				Round(crossCurve[ij](2));

			indexWindow[ij]= indexImg(ValueInMinMax<int>(curRoundPt(0),0,nxx-1),
				ValueInMinMax<int>(curRoundPt(1),0,nyy-1),
				ValueInMinMax<int>(curRoundPt(2),0,nzz-1));
			origWindow[ij]= origImg(ValueInMinMax<int>(curRoundPt(0),0,nxx-1),
				ValueInMinMax<int>(curRoundPt(1),0,nyy-1),
				ValueInMinMax<int>(curRoundPt(2),0,nzz-1));
			backWindow[ij]= backImg(ValueInMinMax<int>(curRoundPt(0),0,nxx-1),
				ValueInMinMax<int>(curRoundPt(1),0,nyy-1),
				ValueInMinMax<int>(curRoundPt(2),0,nzz-1));

			if(double(origWindow[ij]) > double(backWindow[ij])+4.5*std::sqrt(double(backWindow[ij]))){
				continue;
			}else{
				indexWindow[ij]=0;
			}
		}
		double projLength = (crossCurve[i]-crossCurve[0]).dot(initMainDir);
		std::vector<int> tmpList;
		FindEqualList(indexWindow, collideConInfo, tmpList);
		size_t validLen = tmpList.size();
		size_t noValidLen = indexWindow.size() - validLen;
		if(projLength > 10.0 && noValidLen >6){
			isContinueTrace = 1;
		} else{
			isContinueTrace = 0;
		}
	} else{
		isContinueTrace = 0;
	}
}

void TraceFilter::PushNodeUseRayBurst(const Vec3d &curveNode, const Volume<unsigned short> &origImg,
                                      const double threv, const Vec3d &x1, Vec3d &pushedNode)
{
    size_t nx = origImg.x();
    size_t ny = origImg.y();
    size_t nz = origImg.z();
    int xMin = (std::max)((int)Round(curveNode(0) - 7.0), 0);
    int xMax = (std::min)((int)Round(curveNode(0) + 7.0), (int)(nx) - 1);
    int yMin = (std::max)((int)Round(curveNode(1) - 7.0), 0);
    int yMax = (std::min)((int)Round(curveNode(1) + 7.0), (int)(ny) - 1);
    int zMin = (std::max)((int)Round(curveNode(2) - 6.0), 0);
    int zMax = (std::min)((int)Round(curveNode(2) + 6.0), (int)(nz) - 1);
    Vec3d curCenter(curveNode(0) - (double)xMin,
                    curveNode(1) - (double)yMin, curveNode(2) - (double)zMin);//ML1,中心点
    SVolume locOrigImg;
    locOrigImg.SetSize(xMax - xMin + 1, yMax - yMin + 1, zMax - zMin + 1);
    ExtractArea(origImg, xMin, xMax, yMin, yMax, zMin, zMax, locOrigImg);
    size_t minLen = (locOrigImg.x()) < (locOrigImg.y()) ?
        (locOrigImg.x() < locOrigImg.z() ? (locOrigImg.x()) : (locOrigImg.z())) :
        ((locOrigImg.y()) < (locOrigImg.z()) ? (locOrigImg.y()) : (locOrigImg.z()));

    Vec3d x10;
    double PI_18 = M_PI / 18.0;
    if (minLen > 3 && nz > 3)
    {
        VectorVec4d forwardNodeSet;
        double ii, jj;
        for (int i = 0; i < 36; ++i) {
            for ( int j = 0; j < 18; ++j){
                ii = (double)i * PI_18;
                jj = (double)j * PI_18;
                Vec3d polarPosition((std::sin(jj)) * (std::cos(ii)), (std::sin(ii)) * (std::sin(jj)), (std::cos(jj)));//ds1
                double angleDiff = polarPosition.dot(x1);/*jd*/
                if (angleDiff > 0.85)
                {
                    VectorVec3d rayNode;
                    rayNode.push_back(curCenter + 0.5 * polarPosition);
                    rayNode.push_back(curCenter + 1.5 * polarPosition);
                    rayNode.push_back(curCenter + 2.0 * polarPosition);
                    std::vector<double> rayNodeWet;
                    WeighRayValue(rayNode, locOrigImg, rayNodeWet);
                    int rayLimitLen;
                    ContourUtil::CalculateOneRayLimit(rayNodeWet, threv + 4.5 * sqrt(threv), rayLimitLen);//4.5不是4.0
                    forwardNodeSet.push_back(Vec4d(polarPosition(0), polarPosition(1), polarPosition(2),
                                                 300.0 * rayLimitLen + angleDiff) );
                }
            }
        }

#ifdef __linux
        Vec4d tmpNode1 = *(std::max_element(forwardNodeSet.begin(), forwardNodeSet.end(),
            [](const Vec4d& lhs, const Vec4d& rhs){
                return lhs(3) < rhs(3);
        }));

#else
        Vec4d tmpNode1 = *(std::max_element(forwardNodeSet.begin(), forwardNodeSet.end(),
            Vec4d_3th_less()));

#endif
        Vec3d tmpx10(tmpNode1(0), tmpNode1(1), tmpNode1(2));
        x10 = tmpx10.normalized();
    }
    else
    {
        x10 = x1;
    }
    pushedNode = curveNode + 1.5 * x10;
}

bool TraceFilter::Corrode(const CVolume &binImage, const VectorVec3d &binPtSet,
                          const double eroIntensity,
                          CVolume &eroBinImg, VectorVec3d &eroPtSet)
{
    int xMin,xMax;
    int yMin,yMax;
    int zMin,zMax;
    int binDomainSum;

    //int nXX = ero_pt_set.size();
    int nx	= binImage.x();
    int ny	= binImage.y();
    int nz	= binImage.z();

    eroBinImg.SetSize(nx,ny,nz);
    eroPtSet.clear();
    ///---------------------腐蚀-----------------------------------
    //VectorVec3i::const_iterator itend = binPtSet.end();
    //for (VectorVec3i::const_iterator it = binPtSet.begin(); it != itend; ++it)
    for(size_t i = 0; i < binPtSet.size(); ++i){
        const Vec3d& it = binPtSet[i];
        binDomainSum = 0;
        Get3DRegion(xMin, xMax, yMin, yMax, zMin, zMax, Round(it(0)), Round(it(1)), Round(it(2)),
                    1, 1, 1,
                    0, nx-1, 0, ny-1, 0, nz-1);
        ///------------------邻域-----------------------------------
        GetAreaSum(binImage, xMin, xMax, yMin, yMax, zMin, zMax, binDomainSum);
        binDomainSum /= 255;
        ///----------------------判断-------------------------------
        if (binDomainSum > int(27.0f*( 1.0f/3.0f + 0.01f * eroIntensity))) {
            eroBinImg( Round(it(0)), Round(it(1)) , Round(it(2))) = 255;
            eroPtSet.push_back(Vec3d(it(0), it(1), it(2)));
        }
    }
    if (eroPtSet.empty()) return false;
    return true;
}


void TraceFilter::RayburstShapeTrack(const Vec3d &initialPt, const SVolume &origImg, const Vec3d &initDir,
                                     int len, VectorVec5d &forwardArea)
{
    int nxx = origImg.x();
    int nyy = origImg.y();
    int nzz = origImg.z();
    forwardArea.clear();
    //int kk = 0;
    Vec3d initPt(initialPt);
    for(int j = 0; j < len; ++j){
        VectorVec3d rayNode; DVolume smoothRay;
        RayBurstShape(initPt, origImg, rayNode, smoothRay);
        int rayNodeNum = int(rayNode.size());
        double idexxt;
        if(rayNodeNum > 0){
            idexxt = rayNode[0](0)*rayNode[0](1)*rayNode[0](2);
        }
        std::vector<int> validNodeList;//(nss, 0);
        Vec3d forwardDir;
        if(rayNodeNum > 2 && idexxt >= 0.0){
            for(int i = 0; i < rayNodeNum; ++i){
                forwardDir = rayNode[i] - initPt;
                double intersectAngle= initDir.dot(forwardDir);
                if(intersectAngle > 0.0)
                    validNodeList.push_back(i);
            }
            if(validNodeList.size() > 1){
                VectorVec3d tmpRayNode;
                for(size_t k = 0; k < validNodeList.size(); ++k){
                    tmpRayNode.push_back(rayNode[validNodeList[k]]);
                }
                std::vector<double> rayNodeWet;
                WeighRayValue(tmpRayNode, origImg, rayNodeWet);
                double sumRayNodeWet = 0.0;
                for(size_t k = 0; k < rayNodeWet.size(); ++k){
                    sumRayNodeWet += rayNodeWet[k];
                }
                for(size_t k = 0; k < rayNodeWet.size(); ++k){
                    rayNodeWet[k] /= sumRayNodeWet;
                }
                //initPtCp=rayNode(:,JJk)*rayNodeWet';
                Vec3d initPtCp;
                initPtCp.setZero();
                for(size_t k = 0; k < rayNodeWet.size(); ++k){
//                    initPtCp(0) += rayNodeWet[k]
//                            * rayNode[validNodeList[k]](0);
//                    initPtCp(1) += rayNodeWet[k]
//                            * rayNode[validNodeList[k]](1);
//                    initPtCp(2) += rayNodeWet[k]
//                            * rayNode[validNodeList[k]](1);
                    initPtCp += rayNodeWet[k] * rayNode[validNodeList[k]];
                }
                initPt=initPtCp;
                Vec5d tmpNode;
                int idxx=std::min<int>(std::max<int>(int(initPtCp(0)+0.5),
                                                     0), nxx-1 );
                int idxy=std::min<int>(std::max<int>(int(initPtCp(1)+0.5),
                                                     0), nyy-1 );
                int idxz=std::min<int>(std::max<int>(int(initPtCp(2)+0.5),
                                                     0), nzz-1 );
                tmpNode << initPtCp(0), initPtCp(1), initPtCp(2), rayNodeNum,
                        origImg(idxx, idxy, idxz);
                forwardArea.push_back(tmpNode);
            } else{
                break;
            }
        } else{
            break;
        }
    }
}

void TraceFilter::RayBurstShape(const Vec3d &initSoma, const SVolume &v, VectorVec3d &rayNode, DVolume &smoothRay)
{
    rayNode.clear();
    smoothRay.SetSize(0,0,0);

    double slice = 0.3;//(double)(minLen) / 82.0;
    const int blocksum = 26;

    std::vector<double> lineSegment;
    for(int i = 0; i < blocksum; ++i){
        lineSegment.push_back(double(i) * slice);
    }

    const int Theta = 20;
    const int Phi = 10;

    const SVolume& locOrigImg = v;
    const Vec3d& locSoma = initSoma;

    DVolume sphereRayWet;//(lineSegment.size(), Theta, Phi);
    sphereRayWet.SetSize(lineSegment.size(), Theta, Phi);
    const double a = 360.0 / (double)Theta;
    const double b = 180.0 / (double)Phi;
    const double PI_180 = M_PI / 180.0;

    //Volumn SubVol;//(locOrigImg.x(), locOrigImg.y(), locOrigImg.z());
    //int subx = locOrigImg.x();
    //int suby = locOrigImg.y();
    //int subz = locOrigImg.z();

    double segmentDense(0.0), segmentWet(0.0);
    double x,y,z;

    int numLineSegment;
    numLineSegment = lineSegment.size();
    for (int k = 0; k < numLineSegment; ++k){
        for (int i = 1; i <= Theta; ++i){
            for (int j = 1; j <= Phi; ++j){
                x = lineSegment[k] * std::sin(b * (double)j * PI_180) * std::cos(a * (double)i * PI_180) + locSoma(0);
                y = lineSegment[k] * std::sin(b * (double)j * PI_180) * std::sin(a * (double)i * PI_180) + locSoma(1);
                z = lineSegment[k] * std::cos(b * (double)j * PI_180) + locSoma(2);
                segmentDense = segmentWet = 0.0;
                ContourUtil::CalculateSphereOneNode(locOrigImg, 1.0, x, y, z, segmentDense, segmentWet);
                sphereRayWet(k, i-1, j-1) = segmentDense / (segmentWet + 0.0001);
            }
        }
    }//for
    //列存储
    int lenR0_1 = int(lineSegment.size()) - 1;
    std::vector<double> outerShell;
    for (int j = 0; j < Phi; ++j){
        for (int i = 0; i < Theta; ++i){
            outerShell.push_back(sphereRayWet(lenR0_1, i, j));
        }
    }

    std::vector<double> boundaryBack;
    ContourUtil::GetBoundaryBack(outerShell, 4, boundaryBack);

    //three_vs = mean(Lssx1)+3.5*std(Lssx1)
    double constrictionThrev(0.0);
    double constrictionThrevMean =
        std::accumulate(boundaryBack.begin(), boundaryBack.end(), 0.0)
            / double(boundaryBack.size());
    //求标准差---------------------------------------
    double constrictionThrevMeanSqrtSum(0);
    int numBoundaryBack = boundaryBack.size();
    for (int i = 0; i< numBoundaryBack; ++i)
    {
        constrictionThrevMeanSqrtSum +=
            (boundaryBack[i] - constrictionThrevMean) * (boundaryBack[i] - constrictionThrevMean);
    }
    constrictionThrev = 3.5 * std::sqrt(constrictionThrevMeanSqrtSum / (boundaryBack.size() - 1));
    constrictionThrev += constrictionThrevMean;
    //边界阈值给出
    for (int i = 0; i < Phi; ++i){
        for (int j = 0; j < Theta; ++j)
            sphereRayWet(lenR0_1, j, i) = boundaryBack[i * Theta + j];
    }

    std::vector<std::vector<double> > rayLimit;
    ContourUtil::GetRayLimit(sphereRayWet, constrictionThrev, rayLimit);
    TraceUtil::GetGradientVectorFlowForTrace(sphereRayWet, smoothRay);

    //MatXd resultRayLength = 3.0 * MatXd::Ones(Theta + 2, Phi + 2);//射线长度
    std::vector<double> lineSegLength;//分段长度
    //generate_n(back_inserter(lineSegLength), blocksum, GenArray<double>(1.0, 1.0));//注意这里要同步
    for(int i = 0; i < blocksum; ++i){
        lineSegLength.push_back(double(i + 1));
    }

    double curRayLength(0);
    std::vector<double> curSmoothRay( smoothRay.x() );
    std::vector<double> distWet( smoothRay.x() );

    double reduceSmooth[2];
    reduceSmooth[0] = 0.9;
    reduceSmooth[1] = 1.0 - reduceSmooth[0];

    MatXd resultRayLength=3.0*MatXd::Ones(Theta+2, Phi+2);// = rayLimit;
    for(size_t i = 0; i < rayLimit.size();++i){
        for(size_t j = 0; j < rayLimit[0].size(); ++j){
            resultRayLength(i+1,j+1)=rayLimit[i][j];
        }
    }

    int repeat = 10;//!!!!!!
    for (int jj = 0; jj < repeat;++jj){
        for (int i = 1; i <= Theta; ++i){
            for (int j = 1; j <= Phi; ++j){
                curRayLength = resultRayLength(i, j);
                for (int ij = 0; ij < smoothRay.x(); ++ij){
                    curSmoothRay[ij] = smoothRay(ij, i - 1, j - 1);
                    distWet[ij] = curSmoothRay[ij] * std::exp( -0.05 *std::pow( lineSegLength[ij] - curRayLength, 2.0));
                }

                resultRayLength(i,j) += reduceSmooth[0] * ( ( (std::inner_product(lineSegLength.begin(), lineSegLength.end(),
                    distWet.begin(), 0.0)) /
                    (std::accumulate(distWet.begin(), distWet.end(), 0.0) + 0.001 ) ) - resultRayLength(i,j) )
                    - reduceSmooth[1] * (4.0 * resultRayLength(i,j)
                    - resultRayLength(i-1,j)
                    - resultRayLength(i+1,j)
                    - resultRayLength(i,j-1)
                    - resultRayLength(i,j+1));

                resultRayLength(i,j) = std::min(resultRayLength(i,j), rayLimit[i - 1][ j - 1]);
            }
            resultRayLength = (resultRayLength.array() > 0.0).select(resultRayLength, 0.0);
            resultRayLength.row(0) = resultRayLength.row(2);
            resultRayLength.row(Theta + 1) = resultRayLength.row(Theta - 1);
            resultRayLength.col(0) = resultRayLength.col(2);
            resultRayLength.col(Phi + 1) = resultRayLength.col(Phi - 1);
        }
    }//for

    rayNode.reserve(Theta * Phi * lineSegment.size());
    double xx(0.0), yy(0.0), zz(0.0);
    Vec3d tmp;
    for(int j = 1; j < Phi + 1; ++j){
        for(int i = 1; i < Theta + 1; ++i){
            if(resultRayLength(i, j) > 1.0){
                for(double k = 1.0; k <= 0.3 * resultRayLength(i,j); k += 0.5){
                    xx = (k * std::sin(b * (double)(j) * PI_180) * std::cos(a * (double)(i) * PI_180) + initSoma(0));
                    yy = (k * std::sin(b * (double)(j) * PI_180) * std::sin(a * (double)(i) * PI_180) + initSoma(1));
                    zz = (k * std::cos(b * (double)(j) * PI_180) + initSoma(2));
                    tmp(0) = (double)(Round(xx));
                    tmp(1) = (double)(Round(yy));
                    tmp(2) = (double)(Round(zz));
                    rayNode.push_back(tmp);
                }
            }
        }
    }

#ifdef __linux
    std::sort(rayNode.begin(), rayNode.end(), [](const Vec3d& lhs, const Vec3d &rhs){
        if (lhs(0) != rhs(0))  return lhs(0) < rhs(0);
        else if (lhs(1) != rhs(1))  return lhs(1) < rhs(1);
        else if (lhs(2) != rhs(2))  return lhs(2) < rhs(2);
        return false;
    });
#else
    std::sort(rayNode.begin(), rayNode.end(), Vec3d_less());

#endif
    rayNode.erase(std::unique(rayNode.begin(), rayNode.end()), rayNode.end());

    int nxxp = rayNode.size();
    VectorVec3d rayNodeCp;
    int nxx = v.x();
    int nyy = v.y();
    int nzz = v.z();
    for(int i = 0; i < nxxp; ++i){
        const Vec3d& curRayNode = rayNode[i];
        if(curRayNode.minCoeff() > -1.0 && curRayNode(0) < nxx
                && curRayNode(1) < nyy  && curRayNode(2) < nzz ){
            rayNodeCp.push_back(curRayNode);
        }
    }
    rayNode.swap(rayNodeCp);
}

void TraceFilter::ReconstructSomaShapeQuanRevi(const Vec3d &initialPoint, MatXd& resultRayLength,
                                                     VectorVec3d &innerSomaPts)
{
    //2015-6-8
    const SVolume &origImg = *origImgPointer;
    const SVolume &backImg = *backImgPointer;
    //
    innerSomaPts.clear();
    SVolume locOrigImg;
    SVolume locBackImg;
    Vec3d locPoint;
    const int Theta = 40;
    const int Phi = 20;
    ExtractLocalDomainV2(initialPoint, origImg, locOrigImg, locPoint);//2015-6-18
    ExtractLocalDomainV2(initialPoint, backImg, locBackImg, locPoint);//2015-6-18
    double slice = 0.5;
    const int blocksum = 51;//2015-6-18

    std::vector<double> raySlice;
    for(int i = 0; i < blocksum; ++i){
        raySlice.push_back(slice * double(i));
    }

    /**/
    Volume<double> sphereRayWet;
    sphereRayWet.SetSize(raySlice.size(), Theta, Phi);
    Volume<double> sphereBackWet;
    sphereBackWet.SetSize(raySlice.size(), Theta, Phi);

    const double a = 360.0 / (double)Theta;
    const double b = 180.0 / (double)Phi;
    const double PI_180 = M_PI / 180.0;

    double rayNodeDense(0.0), rayNodeWet(0.0);
    double x,y,z;

    for (std::vector<double>::size_type k = 0; k < raySlice.size(); ++k){
        for (int i = 1; i <= Theta; ++i){
            for (int j = 1; j <= Phi; ++j){
                x = raySlice[k] * std::sin(b * (double)j * PI_180) * std::cos(a * (double)i * PI_180) + locPoint(0);
                y = raySlice[k] * std::sin(b * (double)j * PI_180) * std::sin(a * (double)i * PI_180) + locPoint(1);
                z = raySlice[k] * std::cos(b * (double)j * PI_180) + locPoint(2);

                rayNodeDense = rayNodeWet = 0.0;
                ContourUtil::CalculateSphereOneNode(locOrigImg, 1.0, x, y, z, rayNodeDense, rayNodeWet);
                sphereRayWet(k, i-1, j-1) = (double)(rayNodeDense / (rayNodeWet + 0.0001));

                //2015-6-8
                rayNodeDense = rayNodeWet = 0.0;
                ContourUtil::CalculateSphereOneNode(locBackImg, 1.0, x, y, z, rayNodeDense, rayNodeWet);
                sphereBackWet(k, i-1, j-1) = (double)(rayNodeDense / (rayNodeWet + 0.0001));
            }
        }
    }//for

    std::vector<std::vector<double> > rayLimit;
    GetRayLimitV2(sphereRayWet, sphereBackWet, 8, rayLimit);

    Volume<double> smoothRay;

    TraceUtil::GetGradientVectorFlowForTrace(sphereRayWet, smoothRay);

    double gradThrev(0.0);//max value of 
    GetAreaMaxValue(smoothRay, 0,  smoothRay.x() - 1, 0, smoothRay.y() - 1, 0, smoothRay.z() - 1, gradThrev);
    gradThrev *= 0.1;
    //MatXd resultRayLength;//2015-2-7
    resultRayLength = 5.0 * MatXd::Ones(Theta + 2, Phi + 2);//ray length
    for(size_t i = 0; i < rayLimit.size();++i){
        for(size_t j = 0; j < rayLimit[0].size(); ++j){
            resultRayLength(i+1,j+1)=rayLimit[i][j];
        }
    }
    MatXd AAss = resultRayLength;//2015-6-8

    std::vector<double> raySliceIndex;
    for(int i = 0; i < blocksum; ++i){
        raySliceIndex.push_back(1.0 + double(i));
    }

    double currentRayLength(0);
    std::vector<double> currentRayGrad(smoothRay.x());
    std::vector<double> rayGradWet(smoothRay.x());

    double pullParm[2];
    pullParm[0] = 0.8;
    pullParm[1] = 1.0 - pullParm[0];

    int repeat = 20;//2015-6-8
    double rayGradWetSum;

    for (int jj = 0; jj < repeat;++jj){
        for (int i = 1; i <= Theta; ++i){
            for (int j = 1; j <= Phi; ++j){
                currentRayLength = resultRayLength(i, j);
                for (int ij = 0; ij < smoothRay.x(); ++ij){
                    currentRayGrad[ij] = smoothRay(ij, i - 1, j - 1);
                    rayGradWet[ij] = currentRayGrad[ij]
                            * std::exp( -0.05 *std::pow( raySliceIndex[ij] - currentRayLength, 2.0));
                }
                //rayGradWetSum = std::accumulate(rayGradWet.begin(), rayGradWet.end(), 0.0);
                rayGradWetSum = 0.0;
                for(std::vector<double>::size_type k = 0; k < rayGradWet.size(); ++k){
                    rayGradWetSum += rayGradWet[k];
                }
                //if (std::abs(rayGradWetSum) > 0.001){
                resultRayLength(i,j) += pullParm[0] * ( ( (std::inner_product(raySliceIndex.begin(), raySliceIndex.end(), rayGradWet.begin(), 0.0)) /
                                                          (rayGradWetSum ) ) - resultRayLength(i,j) )//2015-6-8+0.001
                        - pullParm[1] * (4.0 * resultRayLength(i,j)
                                         - resultRayLength(i-1,j) - resultRayLength(i+1,j) - resultRayLength(i,j-1) - resultRayLength(i,j+1));
                //}

            }
            //resultRayLength = (resultRayLength.array() > 0.0).select(resultRayLength, 0.0);
            resultRayLength = resultRayLength.cwiseMax(AAss);//2015-6-8
            resultRayLength.row(0) = resultRayLength.row(2);
            resultRayLength.row(Theta + 1) = resultRayLength.row(Theta - 1);
            resultRayLength.col(0) = resultRayLength.col(2);
            resultRayLength.col(Phi + 1) = resultRayLength.col(Phi - 1);
        }
    }//for

    for (int jj = 0; jj < repeat;++jj){
        for (int i = 1; i <= Theta; ++i){
            for (int j = 1; j <= Phi; ++j){
                currentRayLength = resultRayLength(i, j);
                for (int ij = 0; ij < smoothRay.x(); ++ij){
                    currentRayGrad[ij] = smoothRay(ij, i - 1, j - 1);
                    rayGradWet[ij] = currentRayGrad[ij] * std::exp( -0.05 * std::pow( raySliceIndex[ij] - currentRayLength, 2.0));
                }

                rayGradWetSum = 0.0;
                for(std::vector<double>::size_type k = 0; k < rayGradWet.size(); ++k){
                    rayGradWetSum += rayGradWet[k];
                }
                if ( smoothRay( (std::min)(Round(currentRayLength + 0.5) -1, (int)raySlice.size() - 1), i - 1, j - 1 ) >  gradThrev ){
                    resultRayLength(i,j) += pullParm[0] * ( ( (inner_product(raySliceIndex.begin(), raySliceIndex.end(), rayGradWet.begin(), 0.0)) /
                                                              (rayGradWetSum ) ) - resultRayLength(i,j) )//2015-6-8+0.001
                            - pullParm[1] * (4.0 * resultRayLength(i,j) - resultRayLength(i-1,j) - resultRayLength(i+1,j) - resultRayLength(i,j-1) - resultRayLength(i,j+1));
                }
                else{
                    resultRayLength(i,j) += (-pullParm[1]) * (4.0 * resultRayLength(i,j) - resultRayLength(i-1,j) - resultRayLength(i+1,j) - resultRayLength(i,j-1) - resultRayLength(i,j+1));
                }
            }
            //resultRayLength = (resultRayLength.array() > 0.0).select(resultRayLength, 0.0);
            resultRayLength = resultRayLength.cwiseMax(AAss);//2015-6-8
            resultRayLength.row(0) = resultRayLength.row(2);
            resultRayLength.row(Theta + 1) = resultRayLength.row(Theta - 1);
            resultRayLength.col(0) = resultRayLength.col(2);
            resultRayLength.col(Phi + 1) = resultRayLength.col(Phi - 1);
        }
    }//for

    /*
     * 2015-2-5 new inner point collect method
    */
    Vec3d curPt, curNormPt;
    double curPtLength;
    double altitudeAngle, azimuthAngle;
    int altitudeIndicator, azimuthIndicator;
    double ray0, ray1, ray2, ray3, ray4, rayMean;
    Vec3d soma;
    soma << Round(initialPoint(0)), Round(initialPoint(1)), Round(initialPoint(2));
    //2015-6-18
    for(int i = -40; i <= 40; ++i){
        for(int j = -40; j <= 40; ++j){
            for(int ij = -40; ij <= 40; ++ij){
                curPt << i,j,ij;
                curPtLength = curPt.norm();
                if(curPtLength > 0.01){
                    curNormPt = curPt.normalized();
                    CalVectorAngle(curNormPt, altitudeAngle, azimuthAngle);
                    altitudeIndicator=std::min(Round(altitudeAngle*180.0/M_PI/b+0.5),Phi);//+1;
                    azimuthIndicator=std::min(Round(azimuthAngle*180.0/M_PI/a+0.5),Theta);//+1;//C++ minus 1
                    ray0 = resultRayLength(azimuthIndicator,altitudeIndicator)*0.5;
                    ray1 = resultRayLength(azimuthIndicator+1,altitudeIndicator)*0.5;
                    ray2 = resultRayLength(azimuthIndicator-1,altitudeIndicator)*0.5;
                    ray3 = resultRayLength(azimuthIndicator,altitudeIndicator+1)*0.5;
                    ray4 = resultRayLength(azimuthIndicator,altitudeIndicator-1)*0.5;
                    rayMean = 0.2 * (ray0 + ray1 + ray2 + ray3 + ray4);
                    if(curPtLength < rayMean + 0.5)
                        innerSomaPts.push_back(Vec3d(i,j,ij) + soma);
                }else{
                    innerSomaPts.push_back(Vec3d(i,j,ij) + soma);//2015-6-8
                }
            }
        }
    }
    {
        VectorVec3d tmp(innerSomaPts);
        innerSomaPts.swap(tmp);
    }
}

//2015-6-8
void TraceFilter::GetRayLimitV2(const Volume<double> &sphereRayWet, const Volume<double> &sphereBackWet,
								  const double constrictionThrev, std::vector<std::vector<double> > &rayLimit)
{
	int nx = sphereRayWet.x();
	int ny = sphereRayWet.y();
	int nz = sphereRayWet.z();
	//std::vector<double> sphereRay;
	//std::vector<double> sphereBackRay;
	std::vector<double> tmpRay;
	std::vector<double> tmp;
	int arra(0);
	double value;

	for (int i = 0; i < ny; ++i){
		tmp.clear();
		for (int j = 0; j < nz; ++j){
			for (int ij = 0; ij < nx; ++ij){
				//sphereRay.push_back(sphereRayWet(ij, i, j));
				//sphereBackRay.push_back(sphereBackWet(ij, i, j));
				value = sphereRayWet(ij, i, j) - sphereBackWet(ij, i, j) -
					constrictionThrev * std::sqrt(sphereBackWet(ij, i, j));
				tmpRay.push_back(value);
			}

			ContourUtil::CalculateOneRayLimit(tmpRay, 0, arra);
			tmp.push_back(arra);
			tmpRay.clear();
			//sphereBackRay.clear();
		}
		rayLimit.push_back(tmp);
	}
}

void TraceFilter::CalVectorAngle(const Vec3d &curNormPt, double &altitudeAngle, double &azimuthAngle)
{
	altitudeAngle = std::acos(curNormPt(2));
	if(std::abs(curNormPt(0)) < 0.01){
		if(curNormPt(1) > 0.0)
			azimuthAngle = M_PI_2;
		else
			azimuthAngle = M_PI_2 + M_PI;
	}
	else{
		azimuthAngle=std::atan2(curNormPt(1), curNormPt(0));
		if(azimuthAngle <= 0.0)
			azimuthAngle += 2 * M_PI;
	}
}

//2015-6-18
void TraceFilter::ExtractLocalDomainV2(const Vec3d &initPoint, const SVolume &origImg,
									   SVolume &locOrigImg, Vec3d &locPoint)
{
	//Volumn vol(v.Width(), v.Height(), v.Depth());
	int nx = origImg.x();
	int ny = origImg.y();
	int nz = origImg.z();
	int minX = std::max(Round(initPoint(0) - 40.0 ), 0);
	int maxX = std::min(Round(initPoint(0) + 40.0 ), nx - 1);
	int minY = std::max(Round(initPoint(1) - 40.0 ), 0);
	int maxY = std::min(Round(initPoint(1) + 40.0 ), ny - 1);
	int minZ = std::max(Round(initPoint(2) - 40.0 ), 0);//2015-6-18
	int maxZ = std::min(Round(initPoint(2) + 40.0 ), nz - 1);

	locPoint = Vec3d(initPoint(0) - minX, initPoint(1) - minY, initPoint(2) - minZ);

	//Volumn SubVol(maxX - minX + 1, maxY - minY + 1, maxZ - minZ + 1);
	int sx = maxX - minX + 1;
	int sy = maxY - minY + 1;
	int sz = maxZ - minZ + 1;

	locOrigImg.SetSize(sx, sy, sz);
	for (int i = minX; i <= maxX; ++i){
		for (int j = minY; j <= maxY; ++j){
			for (int ij = minZ; ij <= maxZ; ++ij)
				locOrigImg(i - minX, j - minY, ij - minZ) = origImg(i, j, ij);
		}
	}//for
}

void TraceFilter::SetThreValue( double arg)
{
	fillThrev = arg * 4.0;
	endThrev = (arg + 5.0) * 4.0;
}

void TraceFilter::MyCrossDetection( const std::vector<VectorVec5d>& dendCurves, const VectorMat2i &dendConInfo, 
					  std::vector<VectorVec5d>& resultDendCurves, VectorMat2i &resultDendConInfo )
{
	resultDendCurves = dendCurves;
	resultDendConInfo = dendConInfo;
	VectorVec5d connectDotSet;
	size_t sz = dendCurves.size();
	Vec5d tmpDot;
	for (size_t i = 0; i < sz; ++i) {
		if (dendConInfo[i](0, 0) > 0) {
			tmpDot << double(i), 1.0, dendCurves[i][0](0), dendCurves[i][0](1), dendCurves[i][0](2);
			connectDotSet.push_back(tmpDot);
		}
		if (dendConInfo[i](0, 1) > 0) {
			tmpDot << double(i), 2.0, dendCurves[i].back()(0), dendCurves[i].back()(1), dendCurves[i].back()(2);
			connectDotSet.push_back(tmpDot);
		}
	}
	
	std::vector<std::vector<int> > clustreConnectDotSet;
	ClustreFuseConnectDotSet( connectDotSet, 10.0, clustreConnectDotSet );

	size_t clustreConnectDotSetNum = clustreConnectDotSet.size();
	for (size_t ik = 0; ik < clustreConnectDotSetNum; ++ik) {
		std::vector<int>& curClustre = clustreConnectDotSet[ik];
		size_t curClustreNum = curClustre.size();
		MatXd paraDistMatrix(curClustreNum, curClustreNum);
		paraDistMatrix.setZero();
		MatXd corrDirMatrix(curClustreNum, curClustreNum);
		corrDirMatrix.setZero();
		MatXi fuseFlagMatrix(curClustreNum, curClustreNum);
		fuseFlagMatrix.setZero();
		Vec2i headTailFlags(0,0);
		int fuseFlag = 0;
		double paraDist = 0.0, corrDir = 0.0;
		for(size_t i = 0; i < curClustreNum; ++i){
			for (size_t j = 0; j < curClustreNum; ++j){
				if (i != j) {
					const VectorVec5d& curCurve1 = dendCurves[int(connectDotSet[curClustre[i]](0)) ];
					const VectorVec5d& curCurve2 = dendCurves[int(connectDotSet[curClustre[j]](0)) ];
					headTailFlags << int(connectDotSet[curClustre[i]](1)), int(connectDotSet[curClustre[j]](1));
					MyIdentifyCurvesMerge(curCurve1, curCurve2, headTailFlags, fuseFlag, paraDist, corrDir);
					paraDistMatrix(i, j) = paraDist;
					corrDirMatrix(i, j) = corrDir;
					fuseFlagMatrix(i, j) = fuseFlag;
				}
			}
		}
		/*std::cout<<ik << "\n paraDistMatrix:" <<std::endl;
		std::cout << paraDistMatrix << std::endl;
		std::cout<<ik << "\n corrDirMatrix:" <<std::endl;
		std::cout << corrDirMatrix << std::endl;*/
		//
		VectorVec3d fuseList;
		for(size_t i = 0; i < curClustreNum; ++i){
			for (size_t j = 0; j < curClustreNum; ++j){
				if (fuseFlagMatrix(i, j) == 1) {
					fuseList.push_back(Vec3d(double(i), double(j), corrDirMatrix(i, j)));
				}
			}
		}
		//
		if (!fuseList.empty()) {
			//[~,minIndex]=min(fuseList(3,:));%融合曲线
			int minIndex = -1;
			double tmpMin = 10000.0;
			for (size_t i = 0; i < fuseList.size(); ++i) {
				if (tmpMin > fuseList[i](2)) {
					tmpMin = fuseList[i](2);
					minIndex = int(i);
				}
			}
			int fuseIndex1 = int(fuseList[minIndex](0));
			int fuseIndex2 = int(fuseList[minIndex](1));
			int fuseCurveIndex1 = int(connectDotSet[curClustre[fuseIndex1] ](0));
			int fuseCurveIndex2 = int(connectDotSet[curClustre[fuseIndex2] ](0));
			Vec2i headTailFlags;
			headTailFlags << int(connectDotSet[curClustre[fuseIndex1] ](1)), int(connectDotSet[curClustre[fuseIndex2] ](1));
			if(fuseCurveIndex1 != fuseCurveIndex2){
				std::cout << fuseCurveIndex1 <<"  " << fuseCurveIndex2 << std::endl;
				MyMergeCurves(resultDendCurves, resultDendConInfo, fuseCurveIndex1, fuseCurveIndex2, headTailFlags);
			}
		}
	}
}

void TraceFilter::ClustreFuseConnectDotSet( const VectorVec5d& connectDotSet, double distThrev, std::vector<std::vector<int> >& clustreConnectDotSet )
{
	std::vector<std::vector<int> > adjacentList;
	adjacentList.resize(connectDotSet.size());
	size_t sz = connectDotSet.size();
	double tmpDist = 1000.0;
	double dist;
	int idx = -1;
	//construct adjacent list
	for (size_t i = 0; i < sz; ++i) {
		tmpDist = 10000.0;
		idx = -1;
		for(size_t j = i + 1; j < sz; ++j){
			dist = (connectDotSet[i].block(2,0,3,1) - connectDotSet[j].block(2,0,3,1)).norm();
			if (dist < distThrev) {
				if (tmpDist > dist) {
					idx = j;
					tmpDist = dist;
				}
			}
		}
		if (idx > -1) {
			adjacentList[i].push_back(idx);
			adjacentList[idx].push_back(i);
		}
	}

	//clustre
	clustreConnectDotSet.clear();
	for (size_t i = 0; i < sz; ++i) {
		if(!adjacentList[i].empty()){
			clustreConnectDotSet.push_back(std::vector<int>());
			clustreConnectDotSet.back() = adjacentList[i];
			std::vector<int> curLevel;
			curLevel.swap(adjacentList[i]);
			while(!curLevel.empty()){
				std::vector<int> curLevelCp;
				curLevel.swap(curLevelCp);
				for (size_t j = 0; j < curLevelCp.size(); ++j) {
					std::copy(adjacentList[curLevelCp[j] ].begin(), adjacentList[curLevelCp[j] ].end(), std::back_inserter(clustreConnectDotSet.back()));
					std::copy(adjacentList[curLevelCp[j] ].begin(), adjacentList[curLevelCp[j] ].end(), std::back_inserter(curLevel));
					adjacentList[curLevelCp[j] ].clear();
				}
			}
			std::sort(clustreConnectDotSet.back().begin(), clustreConnectDotSet.back().end());
			clustreConnectDotSet.back().erase(std::unique(clustreConnectDotSet.back().begin(), clustreConnectDotSet.back().end()
				), clustreConnectDotSet.back().end());
		}
	}
}

void TraceFilter::MyIdentifyCurvesMerge( const VectorVec5d& curCurve1, const VectorVec5d& curCurve2, const Vec2i& headTailFlags, 
						   int& fuseFlag, double& paraDist, double& corrDir )
{
	//
	std::vector<VectorVec3d> subCurveSet1, subCurveSet2;
	size_t curCurve1Num = curCurve1.size();
	size_t curCurve2Num = curCurve2.size();
	if (headTailFlags(0)==1) {
		VectorVec3d tmp;
		for (size_t beg = 2; beg < 6; ++beg) {
			for (size_t i = beg; i < std::min(curCurve1Num, beg + size_t(7)); i += 2 ) {
				tmp.push_back(curCurve1[i].block(0,0,3,1));
			}
			subCurveSet1.push_back(VectorVec3d());
			subCurveSet1.back().swap(tmp);
			tmp.clear();
		}
	}else{
		VectorVec3d tmp;
		int curCurve1NumCp = int(curCurve1Num);
		for (int end = curCurve1NumCp - 2 - 1; end >= std::max(curCurve1NumCp-6,0); --end) {
			for (int i = end; i >= std::max(end - 6, 0); i -= 2 ) {
				tmp.push_back(curCurve1[i].block(0,0,3,1));
			}
			subCurveSet1.push_back(VectorVec3d());
			subCurveSet1.back().swap(tmp);
			tmp.clear();
		}
	}
	//
	if (headTailFlags(1)==1) {
		VectorVec3d tmp;
		for (size_t beg = 2; beg < 6; ++beg) {
			for (size_t i = beg; i < std::min(curCurve2Num, beg + size_t(7)); i += 2 ) {
				tmp.push_back(curCurve2[i].block(0,0,3,1));
			}
			subCurveSet2.push_back(VectorVec3d());
			subCurveSet2.back().swap(tmp);
			tmp.clear();
		}
	}else{
		VectorVec3d tmp;
		int curCurve2NumCp = int(curCurve2Num);
		for (int end = curCurve2NumCp - 2 - 1; end >= std::max(curCurve2NumCp-6,0); --end) {
			for (int i = end; i >= std::max(end - 6, 0); i -= 2 ) {
				tmp.push_back(curCurve2[i].block(0,0,3,1));
			}
			subCurveSet2.push_back(VectorVec3d());
			subCurveSet2.back().swap(tmp);
			tmp.clear();
		}
	}

	Mat4d paraDistMatrix;
	paraDistMatrix.setZero();
	Mat4d corrDirMatrix;
	corrDirMatrix.setZero();

	for (size_t i = 0; i < 4; ++i) {
		for (size_t j = 0; j < 4; ++j) {
			double pd, cd;
			MyCalcCurvesSimilarity(subCurveSet1[i], subCurveSet2[j], pd, cd);
			paraDistMatrix(i,j) = pd;
			corrDirMatrix(i,j) = cd;
		}
	}
	double tmpValue = 10000.0;
	Vec2i optIndex(-1,-1);
	for (size_t i = 0; i < 4; ++i) {
		for (size_t j = 0; j < 4; ++j) {
			if (paraDistMatrix(i,j)+10.0*corrDirMatrix(i,j)<tmpValue) {
				optIndex << int(i), int(j);
				tmpValue = paraDistMatrix(i,j) + 10.0 * corrDirMatrix(i,j);
			}
		}
	}
	//
	paraDist = paraDistMatrix(optIndex(0),optIndex(1));
	corrDir = corrDirMatrix(optIndex(0),optIndex(1));
	for (size_t i = 0; i < 4; ++i) {
		for (size_t j = 0; j < 4; ++j) {
			if ( paraDistMatrix(i,j) < 6.0 && corrDirMatrix(i,j) < -0.9){
				fuseFlag = 1;
				break;
			}
		}
	}
}

void TraceFilter::MyCalcCurvesSimilarity( const VectorVec3d& curve1, const VectorVec3d& curve2, double& paraDist, double& corrDir )
{
	int curve1Num = int(curve1.size()) - 1;
	int curve2Num = int(curve2.size()) - 1;
	MatXd paraDistMatrix(curve1Num, curve2Num);
	paraDistMatrix.setZero();
	MatXd corrDirMatrix(curve1Num, curve2Num);
	corrDirMatrix.setZero();
	VecXd lengthList(curve1Num);
	double pd(0.0), cd(0.0);
	for (int i = 0; i < curve1Num; ++i) {
		const Vec3d& curve1Dot1 = curve1[i];
		const Vec3d& curve1Dot2 = curve1[i + 1];
		lengthList(i) = (curve1Dot2-curve1Dot1).norm();
		for (int j = 0; j < curve2Num; ++j) {
			const Vec3d& curve2Dot1 = curve2[j];
			const Vec3d& curve2Dot2 = curve2[j + 1];
			MyCalcSegmentSimilarity(curve1Dot1, curve1Dot2, curve2Dot1, curve2Dot2, pd, cd);
			paraDistMatrix(i,j) = pd;
			corrDirMatrix(i,j) = cd;
		}
	}
	double sum = lengthList.sum();
	VecXd normList = lengthList / sum;
	paraDist = normList.transpose() * paraDistMatrix.rowwise().mean();
	corrDir = normList.transpose() * corrDirMatrix.rowwise().mean();

}

void TraceFilter::MyCalcSegmentSimilarity( const Vec3d& curve1Dot1, const Vec3d& curve1Dot2, const Vec3d& curve2Dot1, const Vec3d& curve2Dot2,
							 double& paraDist, double& corrDir )
{
	double den1 = std::max( (curve1Dot2-curve1Dot1).norm(), 0.001);
	Vec3d dir1 = (curve1Dot2-curve1Dot1) / den1;
	double den2 = std::max( (curve2Dot2-curve2Dot1).norm(), 0.001);
	Vec3d dir2 = (curve2Dot2-curve2Dot1) / den2;
	double t1 = (curve1Dot1-curve2Dot1).transpose() * dir2;
	Vec3d mapCurve1Dot1 = curve2Dot1 + t1 * dir2;
	double t2 = (curve1Dot2-curve2Dot1).transpose() * dir2;
	Vec3d mapCurve1Dot2 = curve2Dot1 + t2*dir2;
	double d1 = (curve1Dot1-mapCurve1Dot1).norm();
	double d2 = (curve1Dot2-mapCurve1Dot2).norm();
	paraDist = (d1 * d1 + d2 * d2) / (d1 + d2 + 0.001);
	corrDir = dir1.transpose() * dir2;
}

void TraceFilter::MyMergeCurves( std::vector<VectorVec5d>& resultDendCurves, VectorMat2i& resultDendConInfo,
				   int fuseCurveIndex1, int fuseCurveIndex2, const Vec2i& headTailFlags)
{
	VectorVec5d &curCurve1 = resultDendCurves[fuseCurveIndex1];
	VectorVec5d &curCurve2 = resultDendCurves[fuseCurveIndex2];
	int curCurve1Num = int(curCurve1.size());
	int curCurve2Num = int(curCurve2.size());
	Vec3d curve1HeadPt = curCurve1[0].block(0,0,3,1);
	Vec3d curve1TailPt = curCurve1.back().block(0,0,3,1);
	std::vector<double> curveHeadNodeDist(curCurve2Num);
	std::vector<double> curveTailNodeDist(curCurve2Num);
	for (int i = 0; i < curCurve2Num; ++i) {
		curveHeadNodeDist[i] = (curve1HeadPt-curCurve2[i].block(0,0,3,1)).norm();
		curveTailNodeDist[i] = (curve1TailPt-curCurve2[i].block(0,0,3,1)).norm();
	}
	std::vector<double>::iterator it1 = std::min_element(curveHeadNodeDist.begin(), curveHeadNodeDist.end());
	std::vector<double>::iterator it2 = std::min_element(curveTailNodeDist.begin(), curveTailNodeDist.end());
	double headMinDist = *it1;
	int headMinIndex = int(std::distance(curveHeadNodeDist.begin(), it1));
	double tailMinDist = *it2;
	int tailMinIndex = int(std::distance(curveTailNodeDist.begin(), it2));
	if(headMinDist<tailMinDist){
		if(headTailFlags(1)==1){
			VectorVec5d mergeCurve;
			std::copy(curCurve2.rbegin(), curCurve2.rbegin() + curCurve2Num - headMinIndex, std::back_inserter(mergeCurve));
			std::copy(curCurve1.begin(), curCurve1.end(), std::back_inserter(mergeCurve));
			resultDendCurves[fuseCurveIndex1].swap(mergeCurve);
			Mat2i conInfo1 = resultDendConInfo[fuseCurveIndex1];
			Mat2i conInfo2 = resultDendConInfo[fuseCurveIndex2];
			conInfo1(0,0)=conInfo2(0,1);
			resultDendConInfo[fuseCurveIndex1]=conInfo1;
			resultDendConInfo[fuseCurveIndex2].setZero();
		}else{
			VectorVec5d mergeCurve;
			std::copy(curCurve2.begin(), curCurve2.begin() + headMinIndex + 1, std::back_inserter(mergeCurve));
			std::copy(curCurve1.begin(), curCurve1.end(), std::back_inserter(mergeCurve));
			resultDendCurves[fuseCurveIndex1].swap(mergeCurve);
			Mat2i conInfo1 = resultDendConInfo[fuseCurveIndex1];
			Mat2i conInfo2 = resultDendConInfo[fuseCurveIndex2];
			conInfo1(0,0)=conInfo2(0,0);
			resultDendConInfo[fuseCurveIndex1]=conInfo1;
			resultDendConInfo[fuseCurveIndex2].setZero();
		}
		//
		int resultDendCurvesNum = int(resultDendCurves.size());
		for (int ik = 0; ik < resultDendCurvesNum; ++ik) {
			if(ik != fuseCurveIndex1 && ik != fuseCurveIndex2){
				Mat2i& conInfo = resultDendConInfo[ik];
				if( conInfo(0,0) == fuseCurveIndex2 + 1 ){
					conInfo(0,0) = fuseCurveIndex1 + 1;
				}
				if( conInfo(0,1) == fuseCurveIndex2 + 1 ){
					conInfo(0,1) = fuseCurveIndex1 + 1;
				}
			}
		}
	}else{//if(headMinDist<tailMinDist){
		if(headTailFlags(1)==1){
			VectorVec5d mergeCurve;
			std::copy(curCurve2.rbegin(), curCurve2.rbegin() + curCurve2Num - tailMinIndex, std::back_inserter(mergeCurve));
			std::copy(curCurve1.rbegin(), curCurve1.rend(), std::back_inserter(mergeCurve));
			resultDendCurves[fuseCurveIndex1].swap(mergeCurve);
			Mat2i conInfo1 = resultDendConInfo[fuseCurveIndex1];
			Mat2i conInfo2 = resultDendConInfo[fuseCurveIndex2];
			Mat2i conInfo;
			conInfo.setZero();
			conInfo(0,0)=conInfo2(0,1);
			conInfo(0,1)=conInfo1(0,0);
			resultDendConInfo[fuseCurveIndex1]=conInfo;
			resultDendConInfo[fuseCurveIndex2].setZero();
		}else{
			VectorVec5d mergeCurve;
			std::copy(curCurve2.begin(), curCurve2.begin() + tailMinIndex + 1, std::back_inserter(mergeCurve));
			std::copy(curCurve1.rbegin(), curCurve1.rend(), std::back_inserter(mergeCurve));
			resultDendCurves[fuseCurveIndex1].swap(mergeCurve);
			Mat2i conInfo1 = resultDendConInfo[fuseCurveIndex1];
			Mat2i conInfo2 = resultDendConInfo[fuseCurveIndex2];
			Mat2i conInfo;
			conInfo.setZero();
			conInfo(0,0)=conInfo2(0,0);
			conInfo(0,1)=conInfo1(0,0);
			resultDendConInfo[fuseCurveIndex1]=conInfo;
			resultDendConInfo[fuseCurveIndex2].setZero();
		}
		//
		int resultDendCurvesNum = int(resultDendCurves.size());
		for (int ik = 0; ik < resultDendCurvesNum; ++ik) {
			if(ik != fuseCurveIndex1 && ik != fuseCurveIndex2){
				Mat2i& conInfo = resultDendConInfo[ik];
				if( conInfo(0,0) == fuseCurveIndex2 + 1 ){
					conInfo(0,0) = fuseCurveIndex1 + 1;
				}
				if( conInfo(0,1) == fuseCurveIndex2 + 1 ){
					conInfo(0,1) = fuseCurveIndex1 + 1;
				}
			}
		}
	}
}



///*
// * Copyright (c)2013-2015  Zhou Hang, Shaoqun Zeng, Tingwei Quan
// * Britton Chance Center for Biomedical Photonics, Huazhong University of Science and Technology
// * All rights reserved.
// */
