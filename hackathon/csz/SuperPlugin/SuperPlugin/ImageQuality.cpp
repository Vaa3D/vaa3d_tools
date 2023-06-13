#include <ImageQuality.h>

using namespace std;

template <class T> bool scaleintensity(T *img, V3DLONG sz[4], V3DLONG channo, double lower_th, double higher_th, double target_min, double target_max)
//map the value linear from [lower_th, higher_th] to [target_min, target_max]
{
    if (!img || !sz || sz[0]<=0 || sz[1]<=0 || sz[2]<=0 || sz[3]<=0 || channo>=sz[3]) //channo < 0 is reserved to mean all channels
    {
        v3d_msg("Invalid parameters in scaleintensity();\n");
        return false;
    }

    double t;
    if (lower_th>higher_th) {t=lower_th; lower_th=higher_th; higher_th=t;}
    if (target_min>target_max) {t=target_min; target_min=target_max; target_max=t;}

    double rate = (higher_th==lower_th) ? 1 : (target_max-target_min)/(higher_th-lower_th); //if the two th vals equal, then later-on t-lower_th will be 0 anyway

    V3DLONG i,j,k,c;

    V3DLONG channelPageSize = sz[0]*sz[1]*sz[2];
    switch ( sizeof(T) )
    {
        case 1:
        case 2:
        case 4:
            for (c=0;c<sz[3];c++)
            {
                if (channo>=0 && c!=channo)
                    continue;
                T *data = img + c*channelPageSize;
                for (i=0;i<channelPageSize;i++)
                {
                    t = data[i];
                    if (t>higher_th) t=higher_th;
                    else if (t<lower_th) t=lower_th;
                    data[i] = (T)((t - lower_th)*rate + target_min);
                }
            }
            break;

        default:
            v3d_msg("invalid datatype in scaleintensity();\n", 0);
            return false;
    }

    return true;
}

template <class T> bool rc(T* data1d, V3DLONG *sz, V3DLONG c, double apercent)
//apercent is the percentage of signal that should be upper/lower saturation
{
    if (!data1d || !sz || c<0 || c>=sz[3])
    {
        printf("problem: c=[%ld] sz=[%p] szc=[%ld], data1d=[%p]\n",c, sz, sz[3], data1d);
        return false;
    }

    V3DLONG channelsz = sz[0]*sz[1]*sz[2];
    T * outimg = (data1d + c*channelsz);

    if (apercent<0 || apercent>=0.5)
    {
        v3d_msg(QString("Your percentage parameter [%1] is wrong. Must be bwteen 0 and 0.5.\n").arg(apercent));
        return false;
    }

    V3DLONG i;
    double minval, maxval;
    minval = maxval = double(outimg[0]);
    for (i=1; i<channelsz; i++)
    {
        if (minval > outimg[i]) minval = outimg[i];
        else if (maxval < outimg[i]) maxval = outimg[i];
    }

    if (sizeof(T) == 4) //V3D_FLOAT32, or any other 32bit data; for float data , first rescale it to [0, 4095]
    {
        if (!scaleintensity(data1d, sz, c, minval, maxval, double(0), double(4095)))
            return false;
        else
        {
            minval = 0;
            maxval = 4095;
        }
    }

    {
        V3DLONG maxvv = ceil(maxval+1); //this should be safe now as the potential FLOAT32 data has been rescaled

        qDebug() << "ch =" << c << " maxvv=" << maxvv;

        double *hist = 0;
        try
        {
            hist = new double [maxvv];
        }
        catch (...)
        {
            qDebug() << "fail to allocate"; return false;
            v3d_msg("Fail to allocate memory.\n");
            return false;
        }

        for (i=0;i<maxvv;i++)
        {
            hist[i] = 0;
        }

        //find the histogram
        for (i=0;i<channelsz;i++)
            hist[V3DLONG(outimg[i])] += 1;

        qDebug() << "Histogram computed.";

        //compute the CDF
        for (i=1;i<maxvv;i++)
        {
            hist[i] += hist[i-1];
        }

        for (i=0;i<maxvv;i++)
        {
            hist[i] /= hist[maxvv-1];
        }

        //now search for the intensity thresholds
        double lowerth, upperth; lowerth = upperth = 0;
        for (i=0;i<maxvv-1;i++) //not the most efficient method, but the code should be readable
        {
            if (hist[i]<apercent && hist[i+1]>apercent)
                lowerth = i;
            if (hist[i]<1-apercent && hist[i+1]>1-apercent)
                upperth = i;
        }

        v3d_msg(QString("channel=%1 lower th=%2 upper th=%3").arg(c).arg(lowerth).arg(upperth), 0);

        //real rescale of intensity
        scaleintensity(data1d, sz, c, lowerth, upperth, double(0), double(255));

        //free space
        if (hist) {delete []hist; hist=0;}
    }

    return true;
}

template <class T> bool getVec(const T * pdata1d, V3DLONG datalen, vector<uint8_t> &Vec)
{
    int i;
    // create vector of intensities
    for (i = 0; i < datalen; i++) {
        Vec.push_back(pdata1d[i]);
    }
    return true;
}

bool getStats(vector<uint8_t> pdata1d, V3DLONG datalen, int &minint, int &maxint, double &meanint, double &medianint, double &madint, double &stdevint)
{
    // init
    maxint = int(0);
    minint = int(255);
    meanint = float(0);
    medianint = int(0);
    stdevint = float(0);

    for (V3DLONG i=0;i<datalen;i++)
    {
        int ival = pdata1d.at(i);
        if(ival>maxint)
        {
            maxint = ival;
        }
        if(ival<minint)
        {
            minint = ival;
        }
        meanint += ival;
    }
    meanint /= datalen;

    qDebug() << "Starting Stdev calculation\n";
    for (V3DLONG i=0;i<datalen;i++)
    {
        stdevint += (pdata1d.at(i) - meanint)*(pdata1d.at(i) - meanint);
    }
    stdevint /= datalen;
    stdevint = sqrt(stdevint);


    qDebug() << "Starting Median calculation\n";
    sort(pdata1d.begin(),pdata1d.end());
    if (datalen % 2 != 0)
    {
        medianint = pdata1d.at(datalen / 2);
    }
    else
    {
        //cout << "even\n";
        //cout << datalen << "\n";
        medianint = int(lrint((double) pdata1d.at(datalen/2 - 1) + (double) pdata1d.at(datalen/2))/2);//int(lrint((((double) pdata1d.at(datalen - 1)) / 2 + ((double) pdata1d.at(datalen)) / 2) / 2.0));
    }

    qDebug() << "Starting MAD calculation\n";
    vector<uint8_t> mdevvec;
    uint8_t dev;
    for (int i = 0; i < datalen; i++) {
        dev= pdata1d.at(i)-medianint;
        if(dev < 0)dev *= -1;
        mdevvec.push_back(dev);
    }
    sort(mdevvec.begin(),mdevvec.end());
    if (datalen % 2 != 0)
    {
        madint = mdevvec.at(datalen / 2);
    }
    else
    {
        madint = int(round((double)(mdevvec.at((datalen - 1) / 2) + mdevvec.at(datalen / 2)) / 2.0));
    }
    mdevvec.clear();

    return true;

}

bool getOtsu(QVector<int> tmphist, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, int &ThreshOtsu){
    double var_max, sum, sumB, q1, q2, u1, u2, sigma2;
    var_max = sum = sumB = q1 = q2 = u1 = u2 = sigma2 = 0;
    for(int i=0; i<tmphist.size(); i++)
    {
        sum = sum + i*double(tmphist.at(i));
    }
    for(int t=0; t<tmphist.size(); t++)
    {
        q1 = q1 + double(tmphist.at(t));
        if(q1>0)
        {
            q2 = sz0*sz1*sz2-q1;
            sumB = sumB + double(t)*double(tmphist.at(t));
            u1 = sumB/q1;
            u2 = (sum-sumB)/q2;
            sigma2 = q1*q2*(u1-u2)*(u1-u2);
            if(sigma2 > var_max)
            {
                ThreshOtsu = t;
                var_max =sigma2;
            }
        }
    }

    return true;
}

bool getHistogram(vector<uint8_t> pdata1d, V3DLONG datalen, double max_value, V3DLONG & histscale, QVector<int> &hist)
{
    // init hist
    hist = QVector<int>(histscale, 0);

    for (V3DLONG i=0;i<datalen;i++)
    {
        V3DLONG ind = pdata1d.at(i)/max_value * histscale;
        //V3DLONG ind = pdata1d[i];
        hist[ind] ++;
    }

    return true;

}

bool getSNRCNR(V3DLONG mysize, vector<uint8_t> intvec, double meanint, int maxint, int ThreshOtsu, double &SNRmean, double &SNRotsu, double &CNRmean, double &CNRotsu){
    vector<uint8_t> otsunoise;//,otsusignal;
    double meanotsunoise,meanotsusignal,stdevotsu;//,sqsum;
    meanotsunoise=meanotsusignal=stdevotsu=0;
    vector<uint8_t> meannoise;//,meansignal;
    double avmeannoise,avmeansignal,stdevmean;
    avmeannoise=avmeansignal=stdevmean=0;
    for(int i=0; i<mysize; i++)
    {
        if(intvec.at(i)<=ThreshOtsu)
        {
            otsunoise.push_back(intvec.at(i));
            meanotsunoise += intvec.at(i);
        }
        else
        {
            meanotsusignal += intvec.at(i);
        }

        if(intvec.at(i)<=round(meanint))
        {
            meannoise.push_back(intvec.at(i));
            avmeannoise += intvec.at(i);
        }
        else
        {
            avmeansignal += intvec.at(i);
        }
    }
    meanotsunoise = meanotsunoise/otsunoise.size();
    meanotsusignal = meanotsusignal/(intvec.size() - otsunoise.size());
    for (V3DLONG i=0;i<otsunoise.size();i++)
    {
        stdevotsu += (otsunoise.at(i) - meanotsunoise)*(otsunoise.at(i) - meanotsunoise);
    }
    stdevotsu /= otsunoise.size();
    stdevotsu = sqrt(stdevotsu);
    avmeannoise = avmeannoise/meannoise.size();
    avmeansignal = avmeansignal/(intvec.size() - meannoise.size());
    for (V3DLONG i=0;i<meannoise.size();i++)
    {
        stdevmean += (meannoise.at(i) - avmeannoise)*(meannoise.at(i) - avmeannoise);
    }
    stdevmean /= meannoise.size();
    stdevmean = sqrt(stdevmean);


    //SNRmean = avmeansignal/stdevmean;
    qDebug() << "avemeansignal: " << avmeansignal << "\n";
    qDebug() << "meanint: " << meanint << "\n";
    SNRmean = (avmeansignal-meanint)/sqrt(avmeansignal);
    CNRmean = (maxint-meanint)/stdevmean;
    SNRotsu = (meanotsusignal-ThreshOtsu)/sqrt(meanotsusignal);
    CNRotsu = (maxint-ThreshOtsu)/stdevotsu;

    return true;
}


ImageQuality::ImageQuality(Image4DSimple *p4DImage)
{
    Img_Tobe_Analyse=p4DImage;
    compute();
}

ImageQuality::~ImageQuality()
{
    Img_Tobe_Analyse=nullptr;
}

int ImageQuality::getTracingMethod()
{
    return 1;
}

QString ImageQuality::getQuality()
{
    QString iq="";
    iq+=QString::number(ImgInfo.Channel)+",";
    iq+=QString::number(ImgInfo.MinIntensity)+",";
    iq+=QString::number(ImgInfo.MaxIntensity)+",";
    iq+=QString::number(ImgInfo.MeanIntensity)+",";
    iq+=QString::number(ImgInfo.MedianIntensity)+",";
    iq+=QString::number(ImgInfo.MADIntensity)+",";
    iq+=QString::number(ImgInfo.StdIntensity)+",";
    iq+=QString::number(ImgInfo.PercentMinimal)+",";
    iq+=QString::number(ImgInfo.PercentMaximal)+",";
    iq+=QString::number(ImgInfo.ThresholdOtsu)+",";
    iq+=QString::number(ImgInfo.SNR_mean)+",";
    iq+=QString::number(ImgInfo.CNR_mean)+",";
    iq+=QString::number(ImgInfo.SNR_otsu)+",";
    iq+=QString::number(ImgInfo.CNR_otsu)+",";
    iq+=QString::number(ImgInfo.FocusScore);
    return iq;
}

void ImageQuality::compute()
{
    double max_value = 256;
    V3DLONG histscale = 256;
    QVector<QVector<int> > hist_vec;
    QVector<double> FocusScore_vec;
    QVector<int> MinIntensity_vec;
    QVector<int> MaxIntensity_vec;
    QVector<double> MeanIntensity_vec;
    QVector<double> MedianIntensity_vec;
    QVector<double> MADIntensity_vec;
    QVector<double> StdIntensity_vec;
    QVector<double> PercentMinimal_vec;
    QVector<double> PercentMaximal_vec;
    QVector<double> ThreshOtsu_vec;
    QVector<double> SNRmean_vec;
    QVector<double> CNRmean_vec;
    QVector<double> SNRotsu_vec;
    QVector<double> CNRotsu_vec;
    int nChannel = Img_Tobe_Analyse->getCDim();
    V3DLONG sz[4];
    sz[0] = Img_Tobe_Analyse->getXDim();
    sz[1] = Img_Tobe_Analyse->getYDim();
    sz[2] = Img_Tobe_Analyse->getZDim();
    sz[3] = Img_Tobe_Analyse->getCDim();

    for (V3DLONG c=0;c<nChannel;c++)
    {
        unsigned char * inimg1d = Img_Tobe_Analyse->getRawDataAtChannel(c);

        // Data type conversion
        if (Img_Tobe_Analyse->getDatatype()!=V3D_UINT8)
        {
            v3d_msg("Converting to 8 bit image to standardize results.\n");
            if(Img_Tobe_Analyse->getDatatype()==V3D_UINT16)
            {
                //converting<unsigned short, unsigned char>((unsigned short *)subject1d, inimg1d, sz_sub, V3D_UINT8);
                rc((unsigned short int *)inimg1d, sz, c, 0.01);
            }
            else if(Img_Tobe_Analyse->getDatatype()==V3D_FLOAT32)
            {
                rc((float *)inimg1d, sz, c, 0.01);
                //converting<float, unsigned char>((float *)subject1d, inimg1d, sz_sub, V3D_UINT8);
            }

        }
        qDebug() << "\nComputing statistics\n";
        int  maxint,minint,ThreshOtsu;
        double meanint,medianint,madint,stdevint,pcmin,pcmax,focusscore,SNRmean,CNRmean,SNRotsu,CNRotsu;
        maxint=minint=ThreshOtsu=0;
        meanint=medianint=madint=stdevint=pcmin=pcmax=focusscore=SNRmean=CNRmean=SNRotsu=CNRotsu=0;
        vector<uint8_t> intvec;
        getVec(inimg1d+ c*sz[0]*sz[1]*sz[2], sz[0]*sz[1]*sz[2], intvec);
        if (inimg1d) {delete []inimg1d; inimg1d=NULL;}
        //getStats(inimg1d+ c*sz[0]*sz[1]*sz[2], sz[0]*sz[1]*sz[2], minint, maxint, meanint, medianint);
        getStats(intvec, intvec.size(), minint, maxint, meanint, medianint, madint, stdevint);
        qDebug() << maxint << "\t" << minint << "\t" << meanint << "\n";
        QVector<int> tmphist;
        //getHistogram(inimg1d+ c*sz[0]*sz[1]*sz[2], sz[0]*sz[1]*sz[2], max_value, histscale, tmphist);
        getHistogram(intvec, intvec.size(), max_value, histscale, tmphist);
        pcmin = double(tmphist.at(minint))*100/double(sz[0]*sz[1]*sz[2]);
        pcmax = double(tmphist.at(maxint))*100/double(sz[0]*sz[1]*sz[2]);
        hist_vec.append(tmphist);

        MaxIntensity_vec.append(maxint);
        MinIntensity_vec.append(minint);
        MeanIntensity_vec.append(meanint);
        MedianIntensity_vec.append(medianint);
        MADIntensity_vec.append(madint);
        StdIntensity_vec.append(stdevint);
        PercentMinimal_vec.append(pcmin);
        PercentMaximal_vec.append(pcmax);

        // Get Focus Score
        focusscore = stdevint*stdevint/meanint;
        FocusScore_vec.append(focusscore);

        // Get Otsu threshold
        getOtsu(tmphist, sz[0], sz[1], sz[3], ThreshOtsu);
        qDebug() << "Otsu threshold:\t" << ThreshOtsu << "\n";
        ThreshOtsu_vec.append(ThreshOtsu);

        // Get SNR and CNR using mean intensity and Otsu as threshold for background
        V3DLONG mysize = sz[0]*sz[1]*sz[2];
        getSNRCNR(mysize, intvec, meanint, maxint, ThreshOtsu, SNRmean, SNRotsu, CNRmean, CNRotsu);

        SNRmean_vec.append(SNRmean);
        CNRmean_vec.append(CNRmean);
        SNRotsu_vec.append(SNRotsu);
        CNRotsu_vec.append(CNRotsu);

        //cout << tmphist.at(0) << "\n";
        qDebug() << "minint\tmaxint\tmeanint\tmedint\tMADint\tStdint\tpcmin\tpcmax\tSNRmean\t\tCNRmean\t\tSNRotsu\t\tCNRotsu\t\tFocusScore\n";
        qDebug() << minint << "\t" << maxint << "\t" << meanint << "\t" << medianint << "\t" << madint << "\t" << stdevint << "\t" << pcmin << "\t" << pcmax << "\t" << SNRmean << "\t" << CNRmean << "\t" << SNRotsu << "\t" << CNRotsu << "\t" << focusscore << "\n";
    }
    ImgInfo.Channel=1;
    ImgInfo.MinIntensity=MinIntensity_vec.at(0);
    ImgInfo.MaxIntensity=MaxIntensity_vec.at(0);
    ImgInfo.MeanIntensity=MeanIntensity_vec.at(0);
    ImgInfo.MedianIntensity=MedianIntensity_vec.at(0);
    ImgInfo.MADIntensity=MADIntensity_vec.at(0);
    ImgInfo.StdIntensity=StdIntensity_vec.at(0);
    ImgInfo.PercentMinimal=PercentMinimal_vec.at(0);
    ImgInfo.PercentMaximal=PercentMaximal_vec.at(0);
    ImgInfo.ThresholdOtsu=ThreshOtsu_vec.at(0);
    ImgInfo.SNR_mean=SNRmean_vec.at(0);
    ImgInfo.CNR_mean=CNRmean_vec.at(0);
    ImgInfo.SNR_otsu=SNRotsu_vec.at(0);
    ImgInfo.CNR_otsu=CNRotsu_vec.at(0);
    ImgInfo.FocusScore=FocusScore_vec.at(0);
}
