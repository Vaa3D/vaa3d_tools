#ifndef ANALYZER_H
#define ANALYZER_H

#include <queue>
#include <map>

#include <QtGui>

#include <v3d_interface.h>
#include "basic_surf_objs.h"
#include "VirtualVolume.h"
#include "NeuronStructNavigator.h"

using namespace std;
using namespace iim;

class Analyzer : public QObject
{

	Q_OBJECT
	friend class DatasetGeneratorUI;

public:
	V3DPluginCallback2* analyzerCallback;
	
	float patchMean;
	float patchVar;

	vector<float> patchMeans;
	vector<float> patchVars;

	float meanCompute(vector<float>);
	float stdCompute(vector<float>);
	void imgSetHistProfile(string inputPath, map<size_t, vector<float>> patchProflie);

private:
	float datasetMean;
	float datasetStd;
	float datasetSkew;
	float datasetKurtosis;
};







#endif