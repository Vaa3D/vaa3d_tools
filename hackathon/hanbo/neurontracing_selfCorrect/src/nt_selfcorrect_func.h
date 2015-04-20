#ifndef NT_SELFCORRECT_FUNC_H
#define NT_SELFCORRECT_FUNC_H

#include <QtGui>
#include "hang/my_surf_objs.h"
#include "v3d_basicdatatype.h"
#include "svm/svm.h"

#define VAL_FG 3
#define VAL_BG 2

typedef struct Parameters{
    float sample_scoreThr;
    float sample_radiusFactor_positive;
    float sample_radiusFactor_inter;
    float sample_radiusFactor_negative;
    V3DLONG sample_maxNumber;

    int wavelet_level;

    int mrmr_feaNum;
    int mrmr_method;
    //int mrmr_discretizeThr;
    int mrmr_discretizeBinNumber;

    svm_parameter svm_param;
    //int svm_corss_validation;
    //int svm_nr_fold;

    int correct_sizeThr;
    int correct_neighborNumber;
}ParamStruct;

class nt_selfcorrect_func
{
public:
    nt_selfcorrect_func();
    bool loadData(QString fname_img, QString fname_swc);
    bool saveData(QString fname_output);
    bool calculateScore();
    bool getTrainingSample();
    bool performTraining();
    bool predictExisting();
    bool correctExisting();
    void loadParameter(QString fname_param);

private:
    void saveParameter(QString fname_param);
    void initParameter();
    double predictWindow(V3DLONG x, V3DLONG y, V3DLONG z);
    bool getWindowWavelet(V3DLONG x, V3DLONG y, V3DLONG z, vector<float>& wave);
    //ppp_inImage is dim*dim*dim size 3D image, dim%2==0, ppp_inImage[z][y][x]
    //pppp_outWave is (dim/2)*(dim/2)*(dim/2)*8 4D image, pppp_outWave[z][y][x][w]
    //w from 0~7 are: LLL, HLL, LHL, LLH, LHH, HLH, HHL, HHH
    int getWavelet(float *** ppp_inImage, float **** pppp_outWave, int dim);
    void ind2sub(V3DLONG &x, V3DLONG &y, V3DLONG &z, V3DLONG ind);
    long sub2ind(V3DLONG x, V3DLONG y, V3DLONG z);

private:
//input datas
    unsigned char * p_img1D;
    unsigned char *** ppp_img3D; //ppp_img3d[z][y][x]
    V3DLONG * sz_img;
    int type_img;
    vector<MyMarker*> ntmarkers;

//scored data
    map<MyMarker*, double> score_map;
    vector<V3DLONG> train_positive_idx, train_negative_idx;

//modeling data
    vector<int> selFeature;
    vector<vector<int> > featureInfo;
    int featureNum, sampleNum;
    vector<float ****> tmp_pppp_outWave;
    float *** tmp_ppp_window;
    int winSize;
    struct svm_model *svmModel;

//predict data
    svm_node * svmNode;

//parameters
    ParamStruct param;
};

float *** new_3D_memory(int dim);
void delete_3D_memory(float ***, int dim);

#endif // NT_SELFCORRECT_FUNC_H
