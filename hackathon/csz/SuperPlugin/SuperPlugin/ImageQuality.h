#ifndef IMAGEQUALITY_H
#define IMAGEQUALITY_H

#include <v3d_interface.h>
#include <vector>

using std::vector;

struct ImageInfomation{
    int Channel;
    float MinIntensity;
    float MaxIntensity;
    float MeanIntensity;
    float MedianIntensity;
    float MADIntensity;
    float StdIntensity;
    float PercentMinimal;
    float PercentMaximal;
    float ThresholdOtsu;
    float SNR_mean;
    float CNR_mean;
    float SNR_otsu;
    float CNR_otsu;
    float FocusScore;
};
template <class T> bool scaleintensity(T *img, V3DLONG sz[4], V3DLONG channo, double lower_th, double higher_th, double target_min, double target_max);
template <class T> bool rc(T* data1d, V3DLONG *sz, V3DLONG c, double apercent);
template <class T> bool getVec(const T * pdata1d, V3DLONG datalen, vector<uint8_t> &Vec);
bool getStats(vector<uint8_t> pdata1d, V3DLONG datalen, int &minint, int &maxint, double &meanint, double &medianint, double &madint, double &stdevint);
bool getOtsu(QVector<int> tmphist, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, int &ThreshOtsu);
bool getHistogram(vector<uint8_t> pdata1d, V3DLONG datalen, double max_value, V3DLONG & histscale, QVector<int> &hist);
bool getSNRCNR(V3DLONG mysize, vector<uint8_t> intvec, double meanint, int maxint, int ThreshOtsu, double &SNRmean, double &SNRotsu, double &CNRmean, double &CNRotsu);

class ImageQuality{
public:
    ImageQuality(Image4DSimple *p4DImage);
    ~ImageQuality();
    int getTracingMethod(); //get the predict result, which is the tracing method number.
    QString getQuality();
private:
    void compute();
    Image4DSimple *Img_Tobe_Analyse;
    ImageInfomation ImgInfo;
};


#endif // IMAGEQUALITY_H
