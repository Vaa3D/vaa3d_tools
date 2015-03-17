#include "binaryfilter.h"
#include "../ngtypes/volume.h"
#ifdef _WIN32
#include <ctime>
#else
#include <sys/time.h>
#include <omp.h>
#endif

BinaryFilter::BinaryFilter()
{
    identifyName = std::string("BinaryFilter");
    m_Source = std::shared_ptr<CVolume>(new CVolume(this));
    m_Back = std::shared_ptr<SVolume>(new SVolume(this));
    m_BinPtSet = BinPtSetPointer(new VectorVec3i);
    threadNum = 4;
    binThreshold = 6.0;
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
    if(!m_Source) m_Source = std::shared_ptr<SVolume>(new SVolume(this));
    if(!m_Back) m_Back = std::shared_ptr<CVolume>(new CVolume(this));
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
    std::shared_ptr<const SVolume> tmpImg = std::dynamic_pointer_cast<const SVolume>(m_Input);
    std::shared_ptr<CVolume> tmpBin = std::dynamic_pointer_cast<CVolume>(m_Source);
    std::shared_ptr<SVolume> tmpBack = std::dynamic_pointer_cast<SVolume>(m_Back);
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
    omp_set_num_threads(threadNum);
#pragma omp parallel
#pragma omp for  private( i, j, num,k,l,tmp_value)
    for (ij = 0 ; ij < f ; ++ij){			//all frame
        double *m1 = new double[wh];
        double *sum = new double[ w - 2*radius ];
        memset(sum, 0, sizeof(double) * (w-2*radius));//model
        ///-----------------------------有没有更好的赋值？--------------------------
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
                if ( ( double(tmpImg->GetPixel(i,j,ij)) - m1[ j * w + i ]) > (1.0f + binThreshold * sqrt( m1[ j * w + i ] ) ) )
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
