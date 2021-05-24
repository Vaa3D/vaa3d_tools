#include "neuronReconErrorTypes.h"

void neuronReconErrorTypes::ghostSegUnit::highlightErrorNodes()
{
	for (auto& node : this->theSeg.nodes) node.type = 0;
}

void neuronReconErrorTypes::ghostSegUnit::selfCorrect()
{
	cout << "not implemented yet" << endl;
}

void neuronReconErrorTypes::selfLoopingSegUnit::highlightErrorNodes()
{
	for (auto& node : this->theSeg.nodes) node.type = 5;
}

void neuronReconErrorTypes::selfLoopingSegUnit::selfCorrect()
{
	cout << "not implemented yet" << endl;
}