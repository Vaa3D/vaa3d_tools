#ifndef SOMEFUNCTION_H
#define SOMEFUNCTION_H

#include "v3d_interface.h"

using namespace std;

bool sortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result);
bool sortSWC(NeuronTree& nt);

void getSWCMeanStd(unsigned char* pdata, V3DLONG* sz, NeuronTree& nt, double& mean, double& std);

bool app2WithPreinfo(QString dir, QString brainPath, QString outDir, double ratio, V3DPluginCallback2& callback);

bool app2WithPreinfoForBatch(QString dir, QString brainPath, double ratio, V3DPluginCallback2& callback);

#endif // SOMEFUNCTION_H
