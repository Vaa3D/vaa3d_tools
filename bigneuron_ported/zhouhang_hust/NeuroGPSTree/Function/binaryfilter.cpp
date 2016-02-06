/*
 * Copyright (c)2013-2015  Zhou Hang, Shaoqun Zeng, Tingwei Quan
 * Britton Chance Center for Biomedical Photonics, Huazhong University of Science and Technology
 * All rights reserved.
 */

#ifdef _WIN32
#include <ctime>
#include <omp.h>
#else
#include <sys/time.h>
#endif
#ifdef __linux
#include <omp.h>
#endif
#include "binaryfilter.h"
#include "../ngtypes/volume.h"

BinaryFilter::BinaryFilter()
{
    identifyName = std::string("BinaryFilter");
#ifdef _WIN32
    m_Source = std::tr1::shared_ptr<CVolume>(new CVolume(this));
    m_Back = std::tr1::shared_ptr<SVolume>(new SVolume(this));
#else
    m_Source = std::shared_ptr<CVolume>(new CVolume(this));
    m_Back = std::shared_ptr<SVolume>(new SVolume(this));
#endif
    m_BinPtSet = BinPtSetPointer(new VectorVec3i);
    threadNum = 4;
    binThreshold = 6.0;
    threValue=255;
}

BinaryFilter::~BinaryFilter()
{

}

bool BinaryFilter::Update()
{
    if(!m_Input){
        printf("error occured in %s\n", identifyName.c_str());
        return false;
    }
    if(m_Input->GetProcessObject()){
        if(! m_Input->GetProcessObject()->Update()){
            printf("error occured in %s\n", identifyName.c_str());
            return false;
        }
    }
    if(!Binary()) {
        printf("error occured in %s\n", identifyName.c_str());
        return false;
    }
    return true;
}

ConstDataPointer BinaryFilter::GetOutput()
{
#ifdef _WIN32
    if(!m_Source) m_Source = std::tr1::shared_ptr<SVolume>(new SVolume(this));
    if(!m_Back) m_Back = std::tr1::shared_ptr<CVolume>(new CVolume(this));
#else
    if(!m_Source) m_Source = std::shared_ptr<SVolume>(new SVolume(this));
    if(!m_Back) m_Back = std::shared_ptr<CVolume>(new CVolume(this));
#endif
    if(!m_BinPtSet) m_BinPtSet = BinPtSetPointer(new VectorVec3i);
    return m_Source;
}

ConstDataPointer BinaryFilter::GetBackNoiseImage()
{
    return m_Back;
}

BinaryFilter::BinPtSetPointer BinaryFilter::GetBinPtSet()
{
    return m_BinPtSet;
}

DataPointer BinaryFilter::ReleaseData()
{
    m_Source->ReleaseProcessObject();
    DataPointer tData(m_Source);
    m_Source.reset();
    return tData;
}

DataPointer BinaryFilter::ReleaseBackNoiseImage()
{
    m_Back->ReleaseProcessObject();
    DataPointer tData(m_Back);
    m_Back.reset();
    return tData;
}

BinaryFilter::BinPtSetPointer BinaryFilter::ReleaseBinPtSet()
{
    return m_BinPtSet;
}

void BinaryFilter::SetThreadNum(int t)
{
    threadNum = t;
}

void BinaryFilter::SetThreshold(double t)
{
    binThreshold = t;
}

bool BinaryFilter::Binary()
{
    if ( !m_Input || m_Input->GetIdentifyName() != std::string("Volume")) return false;
#ifdef _WIN32
    std::tr1::shared_ptr<const SVolume> tmpImg = std::tr1::dynamic_pointer_cast<const SVolume>(m_Input);
    std::tr1::shared_ptr<CVolume> tmpBin = std::tr1::dynamic_pointer_cast<CVolume>(m_Source);
    std::tr1::shared_ptr<SVolume> tmpBack = std::tr1::dynamic_pointer_cast<SVolume>(m_Back);
#else
    std::shared_ptr<const SVolume> tmpImg = std::dynamic_pointer_cast<const SVolume>(m_Input);
    std::shared_ptr<CVolume> tmpBin = std::dynamic_pointer_cast<CVolume>(m_Source);
    std::shared_ptr<SVolume> tmpBack = std::dynamic_pointer_cast<SVolume>(m_Back);
#endif
    /*range*/
    int i, j, ij, num,k,l;
    int w = tmpImg->x();
    int h = tmpImg->y();
    int f = tmpImg->z();
    int wh =w * h;
    //double minus = 0;
    int filterNum = 20;
    /*initialization*/
    tmpBin->SetSize(w,h,f);
    tmpBin->SetResolution(tmpImg->XResolution(), tmpImg->YResolution(), tmpImg->ZResolution());
    tmpBack->SetSize(w, h, f);
    /*parameter*/
    int radius = 4;
    double tmp_value = 0.0f;
    double accu = double( (2*radius+1) * (2*radius+1) );/*model sum num*/

#ifdef _WIN32
    clock_t beg = clock();
#else
    timeval start1, end1;
    gettimeofday(&start1, 0);
#endif

    ///---------------------------------20 filters--------------------------///

#ifdef _WIN32
    omp_set_num_threads(threadNum);
#endif
#ifdef __linux
    omp_set_num_threads(threadNum);
#endif
//#pragma omp parallel
//#pragma omp for  private( i, j, num,k,l,tmp_value)
    for (ij = 0 ; ij < f ; ++ij){			//all frame
        double *m1 = new double[wh];
        double *sum = new double[ w - 2*radius ];
        memset(sum, 0, sizeof(double) * (w-2*radius));//model
        ///-------------------------------------------------------
        for (j = 0; j < h; ++j)
            for (i = 0 ; i < w ; ++i)
                m1[ j * w + i ] = std::min((double)tmpImg->GetPixel(i,j,ij), 400.0);//100.0

        double *m2 = new double[wh];			//memset(m2, 0, sizeof(double) * H * W);

        for(num = 0 ; num < filterNum; ++num)
        {
            ///------------------first mean filter---------------------------------

            //inner part
            for(i = radius ; i < w - radius ; ++i){   //initialization array template
                tmp_value = 0;
                for ( k = i - radius ; k <= i + radius ; ++k){     // linear scan
                    for ( l = 0 ; l <= 2 * radius ; ++l)
                        tmp_value += m1[ l * w + k ];
                }
                m2[ radius * w + i ] = tmp_value / accu;
                sum[ i - radius ] = tmp_value;							//finish linear template
            }

            for (j = radius+1 ; j < h - radius ; ++j){
                for(i = radius; i < w - radius; ++i){
                    for( k = i - radius ; k <= i + radius ; ++k)
                        sum[i-radius] += m1[ k+ (j+radius)* w] - m1[k+(j-radius-1)*w ];
                    m2[j * w + i] = sum[ i-radius ] / accu;//get mean value
                }
            }

            // boundray
            for (i = 0 ; i < w ; i++){
                for (l = 0 ; l < radius ; l ++){
                    //upper
                    m2[i+l*w] = (m1[i + l*w ] + m1[i  + (l+1)*w ]) / 2;
                    //lower
                    m2[i + (h - 1 -l ) * w]=(m1[i + (h - 1 -l) * w ]
                                    + m1[i + (h - 2 -l) * w ]) / 2;
                }
            }

            //left and right boundray
            for (j = 0 ; j < h  ; j++){
                for (l = 0 ; l < radius ; l ++){
                    //left
                    m2[l + j * w] =(m1[l + j * w ]
                    + m1[l+1 + j * w ])/ 2 ;
                    //right
                    m2[w - 1-l + j * w] =(m1[w - 1-l + j * w ] +
                        m1[w - 2-l + j * w])/ 2 ;
                }
            }

            memcpy(m1, m2, sizeof(double) * wh);		//there are some questions.
        }
        ///-------------------------------------finish filtering and Binary----------------------------
        // YY=MMt-YYs
        for ( j = 0; j < h; ++j ){//YY0=(YY>(1+threv*sqrt(YYs)));%(key points
            for (i = 0 ; i < w ; ++i){
                if ( ( double(tmpImg->GetPixel(i,j,ij)) - m1[ j * w + i ]) > (1.0f + binThreshold * sqrt( m1[ j * w + i ] ) )
                    || double(tmpImg->GetPixel(i,j,ij)) - double(tmpBack->GetPixel(i,j,ij)) > threValue)
                    tmpBin->GetPixel(i,j,ij) = 255;
                tmpBack->GetPixel(i,j,ij) = (int)m1[ j * w + i ];/*save background noise*/
            }
        }

        /*clear temp data*/
        delete[] m1;
        delete[] m2;
        delete[] sum;
    }//completed

    ///----------------dont use multi-thread----------------///
    for(int ij = 0; ij < f; ++ij)
        for(int j = 0; j < h; ++j)
            for(int i = 0; i < w; ++i){
                if(0 < tmpBin->GetPixel(i,j,ij))
                    m_BinPtSet->push_back(Vec3i(i,j,ij));
            }

    ///--------------------get eclipsed time--------------------------
#ifdef _WIN32
    clock_t end = clock();
    printf("%d ms eclipsed in binarization. \n", int(end - beg));
#else
    gettimeofday(&end1, 0);
    double timeuse=1000000*(end1.tv_sec-start1.tv_sec)+end1.tv_usec-start1.tv_usec;
    timeuse/=1000000;
    printf("%lf s eclipsed in binarization.\n", timeuse);
#endif
    /*update status*/
    //printf("Binarying Finished!\n");
    printf("There are %d dots before Erosion.\n", (int)m_BinPtSet->size());
    return true;
}

//2015-8-13
void BinaryFilter::SetThreValue( double arg)
{
    threValue = arg;
}
