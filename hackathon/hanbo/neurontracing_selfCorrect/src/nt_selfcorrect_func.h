#ifndef NT_SELFCORRECT_FUNC_H
#define NT_SELFCORRECT_FUNC_H

#include <QtGui>
#include "hang/my_surf_objs.h"
#include "v3d_basicdatatype.h"
#include "svm/svm.h"
#include "v3d_interface.h"

#define VAL_FG 3
#define VAL_BG 2

typedef struct Parameters{
    float sample_scoreThr;
    float sample_scoreType;
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
    int correct_falseAllow;

    double radius_bgthr;

    vector<double> app2_bgThrs;
    int app2_channel;
    bool app2_b256;
    bool app2_gsdt;
    bool app2_gap;
    bool app2_2dradius;
    double app2_lenThr;

    int grow_neighbor;
}ParamStruct;

class nt_selfcorrect_func
{
public:
    nt_selfcorrect_func();
    void correct_tracing(QString fname_img, QString fname_swc, QString fname_output);
    void tracing_correct(QString fname_img, QString fname_output, V3DPluginCallback2* cb);
    void smart_tracing(QString fname_img, QString fname_output, V3DPluginCallback2* cb);

private:
    bool loadData(QString fname_img, QString fname_swc);
    bool loadImageData(QString fname_img);
    bool saveData(QString fname_output);
    bool initTracing();
    bool simpleTracing();
    bool smartTracing_regionstart();
    bool smartTracing_seedstart();
    bool calculateScore_topology();
    bool getTrainingSample();
    bool performTraining();
    bool predictExisting();
    bool correctExisting();
    void loadParameter(QString fname_param);

private:
    void saveParameter(QString fname_param);
    void initParameter();
    vector<MyMarker *> sortTracing(QString fname_input, QString fname_output);
    vector<MyMarker *> app2Tracing(QString fname_img, QString fname_output, double bgThr);
    double getMarkersDistance(vector<MyMarker*> &m1, vector<MyMarker*> &m2);
    double predictWindow(V3DLONG x, V3DLONG y, V3DLONG z);
    bool getWindowWavelet(V3DLONG x, V3DLONG y, V3DLONG z, vector<float>& wave);
    //ppp_inImage is dim*dim*dim size 3D image, dim%2==0, ppp_inImage[z][y][x]
    //pppp_outWave is (dim/2)*(dim/2)*(dim/2)*8 4D image, pppp_outWave[z][y][x][w]
    //w from 0~7 are: LLL, HLL, LHL, LLH, LHH, HLH, HHL, HHH
    int getWavelet(float *** ppp_inImage, float **** pppp_outWave, int dim);
    void ind2sub(V3DLONG &x, V3DLONG &y, V3DLONG &z, V3DLONG ind);
    long sub2ind(V3DLONG x, V3DLONG y, V3DLONG z);

private:
    int taskID;

//input datas
    V3DPluginCallback2* callback;
    unsigned char * p_img1D;
    unsigned char *** ppp_img3D; //ppp_img3d[z][y][x]
    V3DLONG * sz_img;
    int type_img;
    vector<MyMarker*> ntmarkers;
    QString fname_tmpout;
    QString fname_inimg;
    QString fname_outswc;

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
