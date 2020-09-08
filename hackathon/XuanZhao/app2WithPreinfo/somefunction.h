#ifndef SOMEFUNCTION_H
#define SOMEFUNCTION_H

#include "v3d_interface.h"

using namespace std;

bool sortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result);
bool sortSWC(NeuronTree& nt);

double getSwcLength(NeuronTree &nt);

void getSWCMeanStd(unsigned char* pdata, V3DLONG* sz, NeuronTree& nt, double& mean, double& std);

void getSWCMeanStd2(unsigned char* pdata, V3DLONG* sz, NeuronTree& nt, double& mean, double& std);

bool app2WithPreinfo(QString dir, QString brainPath, QString outDir, double ratio, int th, ofstream &csvFile, V3DPluginCallback2& callback);

bool app2WithPreinfoForBatch(QString dir, QString brainPath, double ratio, int th, ofstream &csvFile, V3DPluginCallback2& callback);

bool app2WithPreinfo2(QString dir, QString brainPath, QString outDir, ofstream& csvFile, int maxTh, float length, V3DPluginCallback2& callback);

bool app2WithPreinfoForBatch2(QString dir, QString brainPath, ofstream &csvFile, int maxTh, float length, V3DPluginCallback2& callback);

bool app2WithPreinfo3(QString dir, QString brainPath, QString outDir, ofstream& csvFile, int maxTh, int minTh, V3DPluginCallback2& callback);

bool app2WithPreinfoForBatch3(QString dir, QString brainPath, ofstream &csvFile, int maxTh, int minTh, V3DPluginCallback2& callback);

NeuronTree mergeNeuronTrees(vector<NeuronTree> neuronTrees);

#endif // SOMEFUNCTION_H
