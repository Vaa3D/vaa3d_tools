#include "SWCtester.h"

using namespace std;

SWCtester::SWCtester()
{
	this->myImgAnalyzerPtr = new ImgAnalyzer;
	this->myNeuronStructUtilPtr = new NeuronStructUtil;
}

vector<connectedComponent> SWCtester::connComponent2DmergeTest(QString inputSWCfileName)
{
	NeuronTree inputTree = readSWC_file(inputSWCfileName);
	vector<connectedComponent> signalBlobs2D = myNeuronStructUtilPtr->swc2signal2DBlobs(inputTree);
	vector<connectedComponent> outputConnCompList = this->myImgAnalyzerPtr->merge2DConnComponent(signalBlobs2D);

	return outputConnCompList;
}