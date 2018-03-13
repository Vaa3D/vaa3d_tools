#include <iostream>

#include "NeuronStructExplorer.h"

using namespace std;

NeuronStructExplorer::NeuronStructExplorer(QString inputFileName)
{
	QStringList fileNameParse = inputFileName.split(".");
	if (fileNameParse.back() == "swc")
	{	
		this->neuronFileName = inputFileName;
		NeuronTree inputSWC = readSWC_file(inputFileName);
		this->singleTree = inputSWC;
	}
}

NeuronStructExplorer::NeuronStructExplorer(string inputFileName)
{
	this->neuronFileName = QString::fromStdString(inputFileName);
	QStringList fileNameParse = this->neuronFileName.split(".");
	if (fileNameParse.back() == "swc")
	{
		NeuronTree inputSWC = readSWC_file(this->neuronFileName);
		this->singleTree = inputSWC;
	}
}

void NeuronStructExplorer::swcFlipY(NeuronTree const* inputTreePtr, NeuronTree*& outputTreePtr, long int yLength)
{
	float yMiddle = float(yLength + 1) / 2;
	for (QList<NeuronSWC>::const_iterator it = inputTreePtr->listNeuron.begin(); it != inputTreePtr->listNeuron.end(); ++it)
	{
		NeuronSWC flippedNode = *it;
		if (it->y > yMiddle) flippedNode.y = (yMiddle - (it->y - yMiddle));
		else if (it->y < yMiddle) flippedNode.y = (yMiddle + (yMiddle - it->y));
	}
}