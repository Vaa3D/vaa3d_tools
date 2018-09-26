#ifndef SWCTESTER_H
#define SWCTESTER_H

#include <iostream>
#include <vector>
#include <string>

#include <qstring.h>

#include "NeuronStructExplorer.h"
#include "NeuronStructUtilities.h"
#include "ImgAnalyzer.h"

using namespace std;

class SWCtester
{
public:
	SWCtester();

	vector<connectedComponent> connComponent2DmergeTest(QString inputSWCfileName);

private:
	ImgAnalyzer* myImgAnalyzerPtr;
	NeuronStructUtil* myNeuronStructUtilPtr;
};


#endif