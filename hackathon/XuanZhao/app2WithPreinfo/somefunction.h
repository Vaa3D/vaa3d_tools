#ifndef SOMEFUNCTION_H
#define SOMEFUNCTION_H

#include "v3d_interface.h"

using namespace std;

bool sortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result);
bool sortSWC(NeuronTree& nt);

double getSwcLength(NeuronTree &nt);

void getSWCMeanStd(unsigned char* pdata, V3DLONG* sz, NeuronTree& nt, double& mean, double& std);

void getSWCMeanStd2(unsigned char* pdata, V3DLONG* sz, NeuronTree& nt, double& mean, double& std);

//void getSWCMeanStd3(unsigned char* pdata, V3DLONG* sz, NeuronTree& nt, double& mean, double& std);

bool app2WithPreinfo(QString dir, QString brainPath, QString outDir, double ratio, int th, int resolutionTimes, int imageFlag, double lower, double upper, int isMulti, double app2Length, double contrastTh, double contrastRatio, ofstream &csvFile, V3DPluginCallback2& callback);

bool app2WithPreinfoForBatch(QString dir, QString brainPath, double ratio, int th, int resolutionTimes, int imageFlag, double lower, double upper, int isMulti, double app2Length, double contrastTh, double contrastRatio, ofstream &csvFile, V3DPluginCallback2& callback);

bool app2WithPreinfo2(QString dir, QString brainPath, QString outDir, ofstream& csvFile, int maxTh, float length, int resolutionTimes, int imageFlag, double lower, double upper, int isMulti, double app2Length, double contrastTh, double contrastRatio, V3DPluginCallback2& callback);

bool app2WithPreinfoForBatch2(QString dir, QString brainPath, ofstream &csvFile, int maxTh, float length, int resolutionTimes, int imageFlag, double lower, double upper, int isMulti, double app2Length, double contrastTh, double contrastRatio, V3DPluginCallback2& callback);

bool app2WithPreinfo3(QString dir, QString brainPath, QString outDir, ofstream& csvFile, int maxTh, int minTh, int resolutionTimes, int imageFlag, double lower, double upper, int isMulti, double app2Length, double contrastTh, double contrastRatio, V3DPluginCallback2& callback);

bool app2WithPreinfoForBatch3(QString dir, QString brainPath, ofstream &csvFile, int maxTh, int minTh, int resolutionTimes, int imageFlag, double lower, double upper, int isMulti, double app2Length, double contrastTh, double contrastRatio, V3DPluginCallback2& callback);

NeuronTree mergeNeuronTrees(vector<NeuronTree> neuronTrees);

void convertDataTo0_255(unsigned char* data1d, V3DLONG* sz);

void changeContrast(unsigned char* data1d, V3DLONG* sz, double contrastRatio);

void convertDataPiecewise(unsigned char* data1d, V3DLONG* sz, double th1, double th2, double lower, double upper, int mode);

#endif // SOMEFUNCTION_H
