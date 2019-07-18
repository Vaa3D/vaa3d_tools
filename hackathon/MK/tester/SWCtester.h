#ifndef SWCTESTER_H
#define SWCTESTER_H

#include <iostream>
#include <vector>
#include <string>

#include <qstring.h>

#include "NeuronStructExplorer.h"
#include "NeuronStructUtilities.h"
#include "ImgAnalyzer.h"
#include "NeuronGeoGrapher.h"
#include "integratedDataTypes.h"

using namespace std;

class SWCtester
{
public:
	SWCtester();

	vector<connectedComponent> connComponent2DmergeTest(QString inputSWCfileName);

	QList<NeuronSWC> polarCoordShellPeeling(const QList<NeuronSWC>& inputNodeList, const vector<float>& origin, const float radius);
	QList<NeuronSWC> polarCoordAngle_horizontal(const QList<NeuronSWC>& inputNodeList, const vector<float>& origin, const float radius);
	QList<NeuronSWC> polarCoordAngle_vertical(const QList<NeuronSWC>& inputNodeList, const vector<float>& origin, const float radius);

private:
	ImgAnalyzer* myImgAnalyzerPtr;
	NeuronStructUtil* myNeuronStructUtilPtr;
};


#endif