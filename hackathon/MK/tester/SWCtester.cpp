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
	vector<connectedComponent> outputConnCompList = myNeuronStructUtilPtr->swc2signal3DBlobs(inputTree);

	return outputConnCompList;
}
